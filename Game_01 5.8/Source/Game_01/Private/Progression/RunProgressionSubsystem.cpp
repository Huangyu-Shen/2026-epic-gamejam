#include "Progression/RunProgressionSubsystem.h"
#include "Combat/HealthComponent.h"
#include "Enemies/EnemyManagerSubsystem.h"
#include "Enemies/EnemyTypes.h"
#include "Game/DirectorTuningSettings.h"
#include "Player/HeroCharacter.h"
#include "Progression/XpPickup.h"
#include "Engine/DataTable.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"

namespace
{
	FWeaponRow DefaultWeapon(EWeaponType Type)
	{
		FWeaponRow Row;
		Row.WeaponType = Type;
		switch (Type)
		{
		case EWeaponType::Blade:
			Row.Damage = 15.0f; Row.DamagePerLevel = 8.0f; Row.Interval = 0.4f; Row.Range = 600.0f; Row.LineHalfWidth = 0.0f; Row.MaxLevel = 5;
			break;
		case EWeaponType::Bolt:
			Row.Damage = 20.0f; Row.DamagePerLevel = 10.0f; Row.Interval = 0.9f; Row.Range = 1200.0f; Row.LineHalfWidth = 80.0f; Row.MaxLevel = 5;
			break;
		case EWeaponType::Nova:
			Row.Damage = 25.0f; Row.DamagePerLevel = 12.0f; Row.Interval = 1.6f; Row.Range = 350.0f; Row.LineHalfWidth = 0.0f; Row.MaxLevel = 5;
			break;
		default:
			break;
		}
		return Row;
	}

	FPassiveRow DefaultPassive(EPassiveType Type)
	{
		FPassiveRow Row;
		Row.PassiveType = Type;
		switch (Type)
		{
		case EPassiveType::Swiftness: Row.MoveSpeedAdd = 0.10f; Row.MaxLevel = 5; break;
		case EPassiveType::Might:     Row.DamageAdd = 0.15f; Row.MaxLevel = 5; break;
		case EPassiveType::Vitality:  Row.MaxHealthAdd = 20.0f; Row.MaxLevel = 5; break;
		case EPassiveType::Greed:     Row.XpGainAdd = 0.25f; Row.MaxLevel = 5; break;
		default: break;
		}
		return Row;
	}

	const TCHAR* WeaponRowName(EWeaponType Type)
	{
		switch (Type)
		{
		case EWeaponType::Blade: return TEXT("Blade");
		case EWeaponType::Bolt:  return TEXT("Bolt");
		case EWeaponType::Nova:  return TEXT("Nova");
		default: return TEXT("");
		}
	}

	const TCHAR* PassiveRowName(EPassiveType Type)
	{
		switch (Type)
		{
		case EPassiveType::Swiftness: return TEXT("Swiftness");
		case EPassiveType::Might:     return TEXT("Might");
		case EPassiveType::Vitality:  return TEXT("Vitality");
		case EPassiveType::Greed:     return TEXT("Greed");
		default: return TEXT("");
		}
	}
}

void URunProgressionSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	// Project Settings > Game > Director Tuning. Missing tables/rows silently keep the inline defaults.
	const UDirectorTuningSettings* Settings = GetDefault<UDirectorTuningSettings>();
	const TCHAR* Context = TEXT("URunProgressionSubsystem::Initialize");

	if (const UDataTable* XpTable = Settings->ProgressionTable.LoadSynchronous())
	{
		if (const FXpCurveRow* Row = XpTable->FindRow<FXpCurveRow>(TEXT("Progression"), Context, false))
		{
			XpCurve = *Row;
		}
	}

	for (uint8 W = 0; W < static_cast<uint8>(EWeaponType::Count); ++W)
	{
		const EWeaponType Type = static_cast<EWeaponType>(W);
		FWeaponRow Row = DefaultWeapon(Type);
		if (const UDataTable* WeaponTable = Settings->WeaponTable.LoadSynchronous())
		{
			if (const FWeaponRow* Found = WeaponTable->FindRow<FWeaponRow>(WeaponRowName(Type), Context, false))
			{
				Row = *Found;
				Row.WeaponType = Type; // the row key is the source of truth, not the row's own type field
			}
		}
		Weapons.Add(Type, Row);
	}

	for (uint8 P = 0; P < static_cast<uint8>(EPassiveType::Count); ++P)
	{
		const EPassiveType Type = static_cast<EPassiveType>(P);
		FPassiveRow Row = DefaultPassive(Type);
		if (const UDataTable* PassiveTable = Settings->PassiveTable.LoadSynchronous())
		{
			if (const FPassiveRow* Found = PassiveTable->FindRow<FPassiveRow>(PassiveRowName(Type), Context, false))
			{
				Row = *Found;
				Row.PassiveType = Type;
			}
		}
		Passives.Add(Type, Row);
	}

	if (UEnemyManagerSubsystem* Manager = GetWorld()->GetSubsystem<UEnemyManagerSubsystem>())
	{
		Manager->OnEnemyKilled.AddDynamic(this, &URunProgressionSubsystem::HandleEnemyKilled);
	}

	ResetProgression();
}

void URunProgressionSubsystem::ResetProgression()
{
	Level = 1;
	Xp = 0.0f;
	XpToNext = XpCurve.BaseXp;
	WeaponLevels.Reset();
	WeaponLevels.Add(EWeaponType::Blade, 1); // the starter weapon
	PassiveStacks.Reset();
	bOfferPending = false;
	Offer.Reset();

	// Clear any offer text still on screen.
	if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(2203, 0.0f, FColor::Green, TEXT(""));
		for (int32 i = 0; i < 3; ++i)
		{
			GEngine->AddOnScreenDebugMessage(2200 + i, 0.0f, FColor::Green, TEXT(""));
		}
	}

	// Destroy leftover pickups from the previous run (Destroy() unregisters them).
	TArray<TObjectPtr<AXpPickup>> Doomed = MoveTemp(Pickups);
	Pickups.Reset();
	for (AXpPickup* Pickup : Doomed)
	{
		if (IsValid(Pickup))
		{
			Pickup->Destroy();
		}
	}
}

float URunProgressionSubsystem::GetMoveSpeedMultiplier() const
{
	return 1.0f + static_cast<float>(PassiveStacks.FindRef(EPassiveType::Swiftness)) * Passives.FindChecked(EPassiveType::Swiftness).MoveSpeedAdd;
}

float URunProgressionSubsystem::GetDamageMultiplier() const
{
	return 1.0f + static_cast<float>(PassiveStacks.FindRef(EPassiveType::Might)) * Passives.FindChecked(EPassiveType::Might).DamageAdd;
}

float URunProgressionSubsystem::GetMaxHealthBonus() const
{
	return static_cast<float>(PassiveStacks.FindRef(EPassiveType::Vitality)) * Passives.FindChecked(EPassiveType::Vitality).MaxHealthAdd;
}

float URunProgressionSubsystem::GetXpGainMultiplier() const
{
	return 1.0f + static_cast<float>(PassiveStacks.FindRef(EPassiveType::Greed)) * Passives.FindChecked(EPassiveType::Greed).XpGainAdd;
}

void URunProgressionSubsystem::HandleEnemyKilled(EEnemyKind Kind, FVector Location)
{
	if (bOfferPending)
	{
		return;
	}

	FActorSpawnParameters Params;
	Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	if (AXpPickup* Pickup = GetWorld()->SpawnActor<AXpPickup>(AXpPickup::StaticClass(), Location, FRotator::ZeroRotator, Params))
	{
		Pickup->Value = XpCurve.XpPerKill * (Kind == EEnemyKind::Boss ? 10.0f : 1.0f);
	}
}

void URunProgressionSubsystem::GrantXp(float Amount)
{
	Xp += Amount;
	CheckLevelUp();
}

void URunProgressionSubsystem::CheckLevelUp()
{
	if (bOfferPending || Xp < XpToNext)
	{
		return;
	}
	Xp -= XpToNext;
	++Level;
	XpToNext = XpCurve.BaseXp * FMath::Pow(XpCurve.Growth, static_cast<float>(Level - 1));
	GenerateOffer();
}

void URunProgressionSubsystem::GenerateOffer()
{
	Offer.Reset();

	TArray<FOfferOption> Pool;
	for (uint8 W = 0; W < static_cast<uint8>(EWeaponType::Count); ++W)
	{
		const EWeaponType Type = static_cast<EWeaponType>(W);
		const int32 Lvl = WeaponLevels.FindRef(Type);
		if (Lvl < Weapons.FindChecked(Type).MaxLevel)
		{
			FOfferOption Opt;
			Opt.bIsWeapon = true;
			Opt.Weapon = Type;
			Opt.bIsNewWeapon = (Lvl == 0);
			Opt.CurrentLevel = Lvl;
			Pool.Add(Opt);
		}
	}
	for (uint8 P = 0; P < static_cast<uint8>(EPassiveType::Count); ++P)
	{
		const EPassiveType Type = static_cast<EPassiveType>(P);
		const int32 Stacks = PassiveStacks.FindRef(Type);
		if (Stacks < Passives.FindChecked(Type).MaxLevel)
		{
			FOfferOption Opt;
			Opt.bIsWeapon = false;
			Opt.Passive = Type;
			Opt.CurrentLevel = Stacks;
			Pool.Add(Opt);
		}
	}

	// Fisher-Yates shuffle, then take up to three.
	for (int32 i = Pool.Num() - 1; i > 0; --i)
	{
		Pool.Swap(i, FMath::RandRange(0, i));
	}
	const int32 Num = FMath::Min(3, Pool.Num());
	for (int32 i = 0; i < Num; ++i)
	{
		Offer.Add(Pool[i]);
	}

	bOfferPending = Offer.Num() > 0;
	ShowOffer();
}

void URunProgressionSubsystem::ApplyOption(const FOfferOption& Option)
{
	if (Option.bIsWeapon)
	{
		int32& Lvl = WeaponLevels.FindOrAdd(Option.Weapon);
		Lvl = FMath::Min(Lvl + 1, Weapons.FindChecked(Option.Weapon).MaxLevel);
		return;
	}

	const FPassiveRow& Row = Passives.FindChecked(Option.Passive);
	int32& Stacks = PassiveStacks.FindOrAdd(Option.Passive);
	Stacks = FMath::Min(Stacks + 1, Row.MaxLevel);

	// Vitality's effect is applied immediately: raise max health and heal by the same amount.
	if (Option.Passive == EPassiveType::Vitality)
	{
		if (AHeroCharacter* Hero = Cast<AHeroCharacter>(UGameplayStatics::GetPlayerPawn(this, 0)))
		{
			Hero->Health->IncreaseMaxHealth(Row.MaxHealthAdd);
		}
	}
}

void URunProgressionSubsystem::PickOffer(int32 Index)
{
	if (!bOfferPending || !Offer.IsValidIndex(Index))
	{
		return;
	}

	ApplyOption(Offer[Index]);
	bOfferPending = false;
	Offer.Reset();

	if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(2203, 0.0f, FColor::Green, TEXT(""));
		for (int32 i = 0; i < 3; ++i)
		{
			GEngine->AddOnScreenDebugMessage(2200 + i, 0.0f, FColor::Green, TEXT(""));
		}
	}

	CheckLevelUp(); // leftover XP may already qualify for another level.
}

void URunProgressionSubsystem::ShowOffer() const
{
	// The level-up panel (story 007) reads GetOfferDescriptions(); nothing is drawn here.
	UE_LOG(LogTemp, Log, TEXT("Level up -> %d, %d option(s) offered"), Level, Offer.Num());
}

TArray<FString> URunProgressionSubsystem::GetOfferDescriptions() const
{
	TArray<FString> Lines;
	if (bOfferPending)
	{
		for (const FOfferOption& Option : Offer)
		{
			Lines.Add(DescribeOption(Option));
		}
	}
	return Lines;
}

FString URunProgressionSubsystem::DescribeOption(const FOfferOption& Option) const
{
	if (Option.bIsWeapon)
	{
		const FString Name = StaticEnum<EWeaponType>()->GetDisplayNameTextByValue(static_cast<int64>(Option.Weapon)).ToString();
		if (Option.bIsNewWeapon)
		{
			return FString::Printf(TEXT("NEW weapon: %s"), *Name);
		}
		return FString::Printf(TEXT("%s  Lv%d -> Lv%d"), *Name, Option.CurrentLevel, Option.CurrentLevel + 1);
	}

	const FString Name = StaticEnum<EPassiveType>()->GetDisplayNameTextByValue(static_cast<int64>(Option.Passive)).ToString();
	return FString::Printf(TEXT("%s  Lv%d -> Lv%d"), *Name, Option.CurrentLevel, Option.CurrentLevel + 1);
}

void URunProgressionSubsystem::RegisterPickup(AXpPickup* Pickup)
{
	if (Pickup)
	{
		Pickups.Add(Pickup);
	}
}

void URunProgressionSubsystem::UnregisterPickup(AXpPickup* Pickup)
{
	Pickups.RemoveSingleSwap(Pickup, EAllowShrinking::No);
}

void URunProgressionSubsystem::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (bOfferPending || Pickups.IsEmpty())
	{
		return;
	}

	AHeroCharacter* Hero = Cast<AHeroCharacter>(UGameplayStatics::GetPlayerPawn(this, 0));
	if (!Hero)
	{
		return;
	}

	const FVector HeroLocation = Hero->GetActorLocation();
	for (int32 i = Pickups.Num() - 1; i >= 0; --i)
	{
		AXpPickup* Pickup = Pickups[i];
		if (!IsValid(Pickup))
		{
			Pickups.RemoveAtSwap(i);
			continue;
		}

		const FVector ToHero = HeroLocation - Pickup->GetActorLocation();
		const float Dist = ToHero.Size();
		if (Dist <= CollectRadius)
		{
			GrantXp(Pickup->Value * GetXpGainMultiplier());
			Pickups.RemoveAtSwap(i);
			Pickup->Destroy();
		}
		else if (Dist <= MagnetRadius)
		{
			Pickup->SetActorLocation(Pickup->GetActorLocation() + (ToHero / Dist) * MagnetSpeed * DeltaTime);
		}
	}
}
