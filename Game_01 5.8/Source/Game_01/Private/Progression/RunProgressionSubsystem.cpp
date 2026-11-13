#include "Progression/RunProgressionSubsystem.h"
#include "Combat/HealthComponent.h"
#include "Enemies/EnemyManagerSubsystem.h"
#include "Enemies/EnemyTypes.h"
#include "Game/DirectorTuningSettings.h"
#include "Player/HeroCharacter.h"
#include "Progression/XpPickup.h"
#include "UI/HudMessageSubsystem.h"
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
	Stars = Settings->StarTuning;

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
	CutWeapons.Reset();
	CutPassives.Reset();
	CuttingRoomFloor.Reset();
	PermanentCutsLeft = Stars.PermanentCutsForPool(Weapons.Num() + Passives.Num());

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

float URunProgressionSubsystem::PassiveTotal(EPassiveType Passive, float PerStack) const
{
	const int32 Stacks = PassiveStacks.FindRef(Passive);
	return static_cast<float>(Stacks) * PerStack * Stars.PassiveMultiplier(Stars.StarsForCopies(Stacks));
}

float URunProgressionSubsystem::GetMoveSpeedMultiplier() const
{
	return 1.0f + PassiveTotal(EPassiveType::Swiftness, Passives.FindChecked(EPassiveType::Swiftness).MoveSpeedAdd);
}

float URunProgressionSubsystem::GetDamageMultiplier() const
{
	return 1.0f + PassiveTotal(EPassiveType::Might, Passives.FindChecked(EPassiveType::Might).DamageAdd);
}

float URunProgressionSubsystem::GetMaxHealthBonus() const
{
	return PassiveTotal(EPassiveType::Vitality, Passives.FindChecked(EPassiveType::Vitality).MaxHealthAdd);
}

float URunProgressionSubsystem::GetXpGainMultiplier() const
{
	return 1.0f + PassiveTotal(EPassiveType::Greed, Passives.FindChecked(EPassiveType::Greed).XpGainAdd);
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

	// The pool: everything not cut for good and not yet at the copy cap.
	TArray<FOfferOption> Pool;
	for (uint8 W = 0; W < static_cast<uint8>(EWeaponType::Count); ++W)
	{
		const EWeaponType Type = static_cast<EWeaponType>(W);
		const int32 Copies = WeaponLevels.FindRef(Type);
		if (!CutWeapons.Contains(Type) && Copies < MaxCopies(Weapons.FindChecked(Type).MaxLevel))
		{
			FOfferOption Opt;
			Opt.bIsWeapon = true;
			Opt.Weapon = Type;
			Opt.bIsNewWeapon = (Copies == 0);
			Opt.CurrentLevel = Copies;
			Pool.Add(Opt);
		}
	}
	for (uint8 P = 0; P < static_cast<uint8>(EPassiveType::Count); ++P)
	{
		const EPassiveType Type = static_cast<EPassiveType>(P);
		const int32 Copies = PassiveStacks.FindRef(Type);
		if (!CutPassives.Contains(Type) && Copies < MaxCopies(Passives.FindChecked(Type).MaxLevel))
		{
			FOfferOption Opt;
			Opt.bIsWeapon = false;
			Opt.Passive = Type;
			Opt.CurrentLevel = Copies;
			Pool.Add(Opt);
		}
	}

	// Fisher-Yates shuffle, then take up to three.
	for (int32 i = Pool.Num() - 1; i > 0; --i)
	{
		Pool.Swap(i, FMath::RandRange(0, i));
	}

	// With fewer than three options there is nothing to cut: the player simply gets what is left.
	if (Pool.Num() < 3)
	{
		for (const FOfferOption& Option : Pool)
		{
			ApplyOption(Option);
		}
		return;
	}

	for (int32 i = 0; i < 3; ++i)
	{
		Offer.Add(Pool[i]);
	}
	bOfferPending = true;
	ShowOffer();
}

void URunProgressionSubsystem::ApplyOption(const FOfferOption& Option)
{
	int32 StarsBefore = 0;
	int32 StarsAfter = 0;

	if (Option.bIsWeapon)
	{
		int32& Copies = WeaponLevels.FindOrAdd(Option.Weapon);
		StarsBefore = Stars.StarsForCopies(Copies);
		Copies = FMath::Min(Copies + 1, MaxCopies(Weapons.FindChecked(Option.Weapon).MaxLevel));
		StarsAfter = Stars.StarsForCopies(Copies);
	}
	else
	{
		// Vitality's effect is applied immediately: raise max health (and heal) by however much the bonus grew,
		// which includes the jump from a star-up.
		const float HealthBonusBefore = GetMaxHealthBonus();

		int32& Copies = PassiveStacks.FindOrAdd(Option.Passive);
		StarsBefore = Stars.StarsForCopies(Copies);
		Copies = FMath::Min(Copies + 1, MaxCopies(Passives.FindChecked(Option.Passive).MaxLevel));
		StarsAfter = Stars.StarsForCopies(Copies);

		const float HealthGained = GetMaxHealthBonus() - HealthBonusBefore;
		if (HealthGained > 0.0f)
		{
			if (AHeroCharacter* Hero = Cast<AHeroCharacter>(UGameplayStatics::GetPlayerPawn(this, 0)))
			{
				Hero->Health->IncreaseMaxHealth(HealthGained);
			}
		}
	}

	// A star-up is the payoff moment: say so. (StarsBefore is 0 for a brand-new weapon, which is not a star-up.)
	if (StarsAfter > StarsBefore && StarsBefore > 0)
	{
		const FString Message = FString::Printf(TEXT("%s is now %d-STAR!"), *OptionName(Option).ToUpper(), StarsAfter);
		UHudMessageSubsystem::Post(this, Message, FLinearColor(1.0f, 0.85f, 0.2f), 4.0f);
		UE_LOG(LogTemp, Log, TEXT("%s"), *Message);
	}
}

void URunProgressionSubsystem::CutOffer(int32 Index)
{
	if (!bOfferPending || !Offer.IsValidIndex(Index))
	{
		return;
	}

	// The cut option goes to the cutting-room floor; while permanent cuts remain it also leaves the pool.
	const FOfferOption Cut = Offer[Index];
	CuttingRoomFloor.Add(Cut);
	if (PermanentCutsLeft > 0)
	{
		--PermanentCutsLeft;
		if (Cut.bIsWeapon)
		{
			CutWeapons.Add(Cut.Weapon);
		}
		else
		{
			CutPassives.Add(Cut.Passive);
		}
		UHudMessageSubsystem::Post(this, FString::Printf(TEXT("CUT for good: %s"), *OptionName(Cut)), FLinearColor(1.0f, 0.4f, 0.4f), 3.0f);
	}

	// Keep the other two.
	const TArray<FOfferOption> Kept = Offer;
	bOfferPending = false;
	Offer.Reset();
	for (int32 i = 0; i < Kept.Num(); ++i)
	{
		if (i != Index)
		{
			ApplyOption(Kept[i]);
		}
	}

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

FString URunProgressionSubsystem::OptionName(const FOfferOption& Option) const
{
	return Option.bIsWeapon
		? StaticEnum<EWeaponType>()->GetDisplayNameTextByValue(static_cast<int64>(Option.Weapon)).ToString()
		: StaticEnum<EPassiveType>()->GetDisplayNameTextByValue(static_cast<int64>(Option.Passive)).ToString();
}

FString URunProgressionSubsystem::DescribeOption(const FOfferOption& Option) const
{
	const FString Name = OptionName(Option);
	if (Option.bIsWeapon && Option.bIsNewWeapon)
	{
		return FString::Printf(TEXT("NEW weapon: %s"), *Name);
	}

	// What one more copy does to the star: a star-up, or how far the next one is.
	const int32 After = Option.CurrentLevel + 1;
	const int32 StarsAfter = Stars.StarsForCopies(After);
	FString StarNote;
	if (StarsAfter > Stars.StarsForCopies(Option.CurrentLevel) && Option.CurrentLevel > 0)
	{
		StarNote = FString::Printf(TEXT("   >> %d-STAR!"), StarsAfter);
	}
	else if (StarsAfter < 3)
	{
		StarNote = FString::Printf(TEXT("   (%d more to %d-STAR)"), Stars.CopiesToNextStar(After), StarsAfter + 1);
	}
	return FString::Printf(TEXT("%s  Lv%d -> Lv%d%s"), *Name, Option.CurrentLevel, After, *StarNote);
}

FString URunProgressionSubsystem::GetLoadoutSummary() const
{
	// "Blade Lv3**   Might Lv1*": one asterisk per star.
	FString Out;
	auto Append = [&Out, this](const FString& Name, int32 Copies)
	{
		if (Copies > 0)
		{
			Out += FString::Printf(TEXT("%s%s Lv%d%s"), Out.IsEmpty() ? TEXT("") : TEXT("   "), *Name, Copies,
				*FString::ChrN(Stars.StarsForCopies(Copies), TEXT('*')));
		}
	};
	for (uint8 W = 0; W < static_cast<uint8>(EWeaponType::Count); ++W)
	{
		Append(StaticEnum<EWeaponType>()->GetDisplayNameTextByValue(W).ToString(), WeaponLevels.FindRef(static_cast<EWeaponType>(W)));
	}
	for (uint8 P = 0; P < static_cast<uint8>(EPassiveType::Count); ++P)
	{
		Append(StaticEnum<EPassiveType>()->GetDisplayNameTextByValue(P).ToString(), PassiveStacks.FindRef(static_cast<EPassiveType>(P)));
	}
	return Out;
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
