#include "Progression/RunProgressionSubsystem.h"
#include "Audio/GameAudioSubsystem.h"
#include "Combat/HealthComponent.h"
#include "Enemies/EnemyManagerSubsystem.h"
#include "Enemies/EnemyTypes.h"
#include "Extraction/CampStockSubsystem.h"
#include "Game/DirectorTuningSettings.h"
#include "Player/HeroCharacter.h"
#include "Progression/RunLoadout.h"
#include "Progression/XpPickup.h"
#include "UI/HudMessageSubsystem.h"
#include "Engine/DataTable.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"

void URunProgressionSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	// The enemy manager must exist first: the kill binding below is made once, here.
	Collection.InitializeDependency<UEnemyManagerSubsystem>();
	Super::Initialize(Collection);

	// Project Settings > Game > Director Tuning. Missing tables/rows silently keep the inline defaults.
	const UDirectorTuningSettings* Settings = GetDefault<UDirectorTuningSettings>();
	const TCHAR* Context = TEXT("URunProgressionSubsystem::Initialize");
	Loadout.Stars = Settings->StarTuning;

	if (const UDataTable* XpTable = Settings->ProgressionTable.LoadSynchronous())
	{
		if (const FXpCurveRow* Row = XpTable->FindRow<FXpCurveRow>(TEXT("Progression"), Context, false))
		{
			XpCurve = *Row;
		}
	}
	Loadout.BaseMagnetRadius = XpCurve.BaseMagnetRadius;

	for (uint8 W = 0; W < static_cast<uint8>(EWeaponType::Count); ++W)
	{
		const EWeaponType Type = static_cast<EWeaponType>(W);
		FWeaponRow Row = FRunLoadout::DefaultWeapon(Type);
		if (const UDataTable* WeaponTable = Settings->WeaponTable.LoadSynchronous())
		{
			if (const FWeaponRow* Found = WeaponTable->FindRow<FWeaponRow>(FRunLoadout::WeaponRowName(Type), Context, false))
			{
				Row = *Found;
				Row.WeaponType = Type; // the row key is the source of truth, not the row's own type field
			}
		}
		Loadout.Weapons.Add(Type, Row);
	}

	for (uint8 P = 0; P < static_cast<uint8>(EPassiveType::Count); ++P)
	{
		const EPassiveType Type = static_cast<EPassiveType>(P);
		FPassiveRow Row = FRunLoadout::DefaultPassive(Type);
		if (const UDataTable* PassiveTable = Settings->PassiveTable.LoadSynchronous())
		{
			if (const FPassiveRow* Found = PassiveTable->FindRow<FPassiveRow>(FRunLoadout::PassiveRowName(Type), Context, false))
			{
				Row = *Found;
				Row.PassiveType = Type;
			}
		}
		Loadout.Passives.Add(Type, Row);
	}

	if (UEnemyManagerSubsystem* Manager = GetWorld()->GetSubsystem<UEnemyManagerSubsystem>())
	{
		Manager->OnEnemyKilled.AddDynamic(this, &URunProgressionSubsystem::HandleEnemyKilled);
	}

	Random.GenerateNewSeed();
	ResetProgression();
}

void URunProgressionSubsystem::ResetProgression()
{
	Level = 1;
	Xp = 0.0f;
	XpToNext = XpCurve.BaseXp;
	Loadout.ResetOwnership();
	Rules.Reset(Loadout.Stars.PermanentCutsForPool(GetPoolSize()));
	UE_LOG(LogTemp, Log, TEXT("PROGRESSION pool=%d permanentCuts=%d"), GetPoolSize(), Rules.PermanentCutsLeft);

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

float URunProgressionSubsystem::GetMoveSpeedMultiplier() const { return Loadout.MoveSpeedMultiplier(); }
float URunProgressionSubsystem::GetDamageMultiplier() const { return Loadout.DamageMultiplier(); }
float URunProgressionSubsystem::GetMaxHealthBonus() const { return Loadout.MaxHealthBonus(); }
float URunProgressionSubsystem::GetXpGainMultiplier() const { return Loadout.XpGainMultiplier(); }
float URunProgressionSubsystem::GetAttackSpeedMultiplier() const { return Loadout.AttackSpeedMultiplier(); }
float URunProgressionSubsystem::GetMagnetRadius() const { return Loadout.MagnetRadius(); }
float URunProgressionSubsystem::GetRangeMultiplier() const { return Loadout.RangeMultiplier(); }
float URunProgressionSubsystem::GetDamageReduction() const { return Loadout.DamageReduction(); }
float URunProgressionSubsystem::GetRegenPerSecond() const { return Loadout.RegenPerSecond(); }
float URunProgressionSubsystem::GetCritChance() const { return Loadout.CritChance(); }
float URunProgressionSubsystem::GetCritMultiplier() const { return Loadout.CritMultiplier(); }

TArray<FName> URunProgressionSubsystem::GetUnlockedOptions() const
{
	// Editor and commandlet worlds have no game instance, so every step may be null.
	const UWorld* World = GetWorld();
	const UGameInstance* GameInstance = World ? World->GetGameInstance() : nullptr;
	if (const UCampStockSubsystem* Stock = GameInstance ? GameInstance->GetSubsystem<UCampStockSubsystem>() : nullptr)
	{
		return Stock->GetUnlockedOptions();
	}
	return {};
}

int32 URunProgressionSubsystem::GetPoolSize() const
{
	return Loadout.PoolSize(GetUnlockedOptions());
}

void URunProgressionSubsystem::HandleEnemyKilled(EEnemyKind Kind, FVector Location)
{
	if (Rules.bPending)
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

void URunProgressionSubsystem::DebugGrantWeapon(EWeaponType Weapon, int32 Copies)
{
	Loadout.SetWeaponCopies(Weapon, Copies);
}

void URunProgressionSubsystem::GrantXp(float Amount)
{
	Xp += Amount;
	CheckLevelUp();
}

void URunProgressionSubsystem::CheckLevelUp()
{
	if (Rules.bPending || Xp < XpToNext)
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
	const TArray<FName> Unlocked = GetUnlockedOptions();
	const FOfferRules::FGenerated Generated = Rules.Generate(Loadout, Unlocked, Random, [this]() -> TArray<FName>
	{
		// The camp's NEW markers: options a lost film reel unlocked since the player last saw an offer.
		UGameInstance* GameInstance = GetWorld()->GetGameInstance();
		UCampStockSubsystem* Stock = GameInstance ? GameInstance->GetSubsystem<UCampStockSubsystem>() : nullptr;
		return Stock ? Stock->TakeNewOptions() : TArray<FName>();
	});

	for (const FOfferOption& Option : Generated.ApplyAtOnce)
	{
		ApplyOption(Option);
	}
	if (Generated.bOfferOpened)
	{
		UGameAudioSubsystem::PlayIn(this, EGameSound::LevelUp);
		ShowOffer();
	}
}

void URunProgressionSubsystem::ApplyOption(const FOfferOption& Option)
{
	int32 StarsBefore = 0;
	int32 StarsAfter = 0;

	if (Option.bIsWeapon)
	{
		StarsBefore = Loadout.WeaponStars(Option.Weapon);
		Loadout.AddWeaponCopy(Option.Weapon);
		StarsAfter = Loadout.WeaponStars(Option.Weapon);
	}
	else
	{
		// Vitality's effect is applied immediately: raise max health (and heal) by however much the bonus grew,
		// which includes the jump from a star-up.
		const float HealthBonusBefore = GetMaxHealthBonus();

		StarsBefore = Loadout.PassiveStarsFor(Option.Passive);
		Loadout.AddPassiveCopy(Option.Passive);
		StarsAfter = Loadout.PassiveStarsFor(Option.Passive);

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
		UGameAudioSubsystem::PlayIn(this, EGameSound::StarUp);
	}
}

bool URunProgressionSubsystem::CanCutFromOffer() const
{
	return Rules.CanCut();
}

void URunProgressionSubsystem::CutFromOffer(int32 Index)
{
	FOfferOption Cut;
	if (!Rules.Cut(Index, Cut))
	{
		return;
	}

	UGameAudioSubsystem::PlayIn(this, EGameSound::Cut);
	UHudMessageSubsystem::Post(this, FString::Printf(TEXT("CUT: %s now belongs to the Boss"), *OptionName(Cut)), FLinearColor(1.0f, 0.4f, 0.4f), 3.0f);
}

void URunProgressionSubsystem::ChooseOption(int32 Index)
{
	FOfferOption Chosen;
	if (!Rules.Choose(Index, Chosen))
	{
		return;
	}

	ApplyOption(Chosen);
	CheckLevelUp(); // leftover XP may already qualify for another level.
}

void URunProgressionSubsystem::ShowOffer() const
{
	// The level-up panel (story 007) reads GetOfferDescriptions(); nothing is drawn here.
	UE_LOG(LogTemp, Log, TEXT("Level up -> %d, %d option(s) offered"), Level, Rules.Offer.Num());
}

TArray<FString> URunProgressionSubsystem::GetOfferDescriptions() const
{
	TArray<FString> Lines;
	if (Rules.bPending)
	{
		for (const FOfferOption& Option : Rules.Offer)
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
	const FString NewMark = Option.bIsNewUnlock ? FString(TEXT("NEW! ")) : FString();
	if (Option.bIsWeapon && Option.bIsNewWeapon)
	{
		// An unlocked weapon is both new to the camp and new to the run: say it once.
		return Option.bIsNewUnlock ? FString::Printf(TEXT("NEW! %s  (unlocked weapon)"), *Name)
		                           : FString::Printf(TEXT("NEW weapon: %s"), *Name);
	}

	// What one more copy does to the star: a star-up, or how far the next one is.
	const int32 After = Option.CurrentLevel + 1;
	const int32 StarsAfter = Loadout.Stars.StarsForCopies(After);
	FString StarNote;
	if (StarsAfter > Loadout.Stars.StarsForCopies(Option.CurrentLevel) && Option.CurrentLevel > 0)
	{
		StarNote = FString::Printf(TEXT("   >> %d-STAR!"), StarsAfter);
	}
	else if (StarsAfter < 3)
	{
		StarNote = FString::Printf(TEXT("   (%d more to %d-STAR)"), Loadout.Stars.CopiesToNextStar(After), StarsAfter + 1);
	}
	return FString::Printf(TEXT("%s%s  Lv%d -> Lv%d%s"), *NewMark, *Name, Option.CurrentLevel, After, *StarNote);
}

TArray<FOfferCardText> URunProgressionSubsystem::GetOfferCards() const
{
	TArray<FOfferCardText> Cards;
	if (!Rules.bPending)
	{
		return Cards;
	}
	for (const FOfferOption& Option : Rules.Offer)
	{
		FOfferCardText Card;
		Card.Title = (Option.bIsNewUnlock ? FString(TEXT("NEW!  ")) : FString()) + OptionName(Option);
		Card.bHighlight = Option.bIsNewUnlock;
		if (Option.bIsWeapon && Option.bIsNewWeapon)
		{
			Card.Detail = Option.bIsNewUnlock ? FString(TEXT("Unlocked weapon")) : FString(TEXT("New weapon"));
		}
		else
		{
			const int32 After = Option.CurrentLevel + 1;
			const int32 StarsAfter = Loadout.Stars.StarsForCopies(After);
			Card.Detail = FString::Printf(TEXT("Lv%d  >  Lv%d"), Option.CurrentLevel, After);
			if (StarsAfter > Loadout.Stars.StarsForCopies(Option.CurrentLevel) && Option.CurrentLevel > 0)
			{
				Card.Note = FString::Printf(TEXT("%d-STAR UP!"), StarsAfter);
				Card.bHighlight = true;
			}
			else if (StarsAfter < 3)
			{
				Card.Note = FString::Printf(TEXT("%d more to %d-STAR"), Loadout.Stars.CopiesToNextStar(After), StarsAfter + 1);
			}
		}
		Cards.Add(Card);
	}
	return Cards;
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
				*FString::ChrN(Loadout.Stars.StarsForCopies(Copies), TEXT('*')));
		}
	};
	for (uint8 W = 0; W < static_cast<uint8>(EWeaponType::Count); ++W)
	{
		Append(StaticEnum<EWeaponType>()->GetDisplayNameTextByValue(W).ToString(), Loadout.WeaponCopies(static_cast<EWeaponType>(W)));
	}
	for (uint8 P = 0; P < static_cast<uint8>(EPassiveType::Count); ++P)
	{
		Append(StaticEnum<EPassiveType>()->GetDisplayNameTextByValue(P).ToString(), Loadout.PassiveCopies(static_cast<EPassiveType>(P)));
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

	if (Rules.bPending || Pickups.IsEmpty())
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
			UGameAudioSubsystem::PlayIn(this, EGameSound::Pickup);
			Pickups.RemoveAtSwap(i);
			Pickup->Destroy();
		}
		else if (Dist <= GetMagnetRadius())
		{
			Pickup->SetActorLocation(Pickup->GetActorLocation() + (ToHero / Dist) * MagnetSpeed * DeltaTime);
		}
	}
}
