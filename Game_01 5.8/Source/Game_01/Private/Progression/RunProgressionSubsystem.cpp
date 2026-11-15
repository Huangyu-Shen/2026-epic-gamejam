#include "Progression/RunProgressionSubsystem.h"
#include "Audio/GameAudioSubsystem.h"
#include "Combat/HealthComponent.h"
#include "Enemies/EnemyManagerSubsystem.h"
#include "Enemies/EnemyTypes.h"
#include "Extraction/CampStockSubsystem.h"
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
		case EWeaponType::Chain:
			Row.Damage = 12.0f; Row.DamagePerLevel = 6.0f; Row.Interval = 1.1f; Row.Range = 700.0f; Row.LineHalfWidth = 0.0f; Row.MaxLevel = 5;
			Row.ChainJumps = 3; Row.ChainRange = 350.0f; Row.bStartsLocked = true;
			break;
		case EWeaponType::Gun:
			// Long reach on purpose: it is the answer to enemies closing in before the hero is strong.
			Row.Damage = 14.0f; Row.DamagePerLevel = 7.0f; Row.Interval = 0.7f; Row.Range = 1500.0f; Row.LineHalfWidth = 0.0f; Row.MaxLevel = 5;
			Row.ProjectileSpeed = 2600.0f; Row.ProjectileCount = 1;
			break;
		case EWeaponType::Orbit:
			// Range is the orbit radius; Interval is how often one enemy can be hurt by the balls again.
			Row.Damage = 10.0f; Row.DamagePerLevel = 5.0f; Row.Interval = 0.5f; Row.Range = 240.0f; Row.LineHalfWidth = 0.0f; Row.MaxLevel = 5;
			Row.ProjectileSpeed = 160.0f; Row.ProjectileCount = 2;
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
		case EPassiveType::Haste:     Row.AttackSpeedAdd = 0.12f; Row.MaxLevel = 5; Row.bStartsLocked = true; break;
			case EPassiveType::Magnet:    Row.MagnetRadiusAdd = 150.0f; Row.MaxLevel = 5; Row.bStartsLocked = true; break;
			case EPassiveType::Reach:     Row.RangeAdd = 0.12f; Row.MaxLevel = 5; break;
			case EPassiveType::Armor:     Row.DamageReductionAdd = 0.08f; Row.MaxLevel = 5; break;
			case EPassiveType::Regen:     Row.RegenPerSecondAdd = 1.0f; Row.MaxLevel = 5; break;
			case EPassiveType::Crit:      Row.CritChanceAdd = 0.08f; Row.CritMultiplier = 2.0f; Row.MaxLevel = 5; break;
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
		case EWeaponType::Chain: return TEXT("Chain");
		case EWeaponType::Gun:   return TEXT("Gun");
		case EWeaponType::Orbit: return TEXT("Orbit");
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
		case EPassiveType::Haste:     return TEXT("Haste");
			case EPassiveType::Magnet:    return TEXT("Magnet");
			case EPassiveType::Reach:     return TEXT("Reach");
			case EPassiveType::Armor:     return TEXT("Armor");
			case EPassiveType::Regen:     return TEXT("Regen");
			case EPassiveType::Crit:      return TEXT("Crit");
			default: return TEXT("");
		}
	}
}

void URunProgressionSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	// The enemy manager must exist first: the kill binding below is made once, here.
	Collection.InitializeDependency<UEnemyManagerSubsystem>();
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
	PermanentCutsLeft = Stars.PermanentCutsForPool(GetPoolSize());
	UE_LOG(LogTemp, Log, TEXT("PROGRESSION pool=%d permanentCuts=%d"), GetPoolSize(), PermanentCutsLeft);

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

float URunProgressionSubsystem::GetAttackSpeedMultiplier() const
{
	return 1.0f + PassiveTotal(EPassiveType::Haste, Passives.FindChecked(EPassiveType::Haste).AttackSpeedAdd);
}

float URunProgressionSubsystem::GetMagnetRadius() const
{
	return XpCurve.BaseMagnetRadius + PassiveTotal(EPassiveType::Magnet, Passives.FindChecked(EPassiveType::Magnet).MagnetRadiusAdd);
}

float URunProgressionSubsystem::GetRangeMultiplier() const
{
	return 1.0f + PassiveTotal(EPassiveType::Reach, Passives.FindChecked(EPassiveType::Reach).RangeAdd);
}

float URunProgressionSubsystem::GetDamageReduction() const
{
	return FMath::Clamp(PassiveTotal(EPassiveType::Armor, Passives.FindChecked(EPassiveType::Armor).DamageReductionAdd), 0.0f, 0.75f);
}

float URunProgressionSubsystem::GetRegenPerSecond() const
{
	return PassiveTotal(EPassiveType::Regen, Passives.FindChecked(EPassiveType::Regen).RegenPerSecondAdd);
}

float URunProgressionSubsystem::GetCritChance() const
{
	return FMath::Clamp(PassiveTotal(EPassiveType::Crit, Passives.FindChecked(EPassiveType::Crit).CritChanceAdd), 0.0f, 0.9f);
}

float URunProgressionSubsystem::GetCritMultiplier() const
{
	return Passives.FindChecked(EPassiveType::Crit).CritMultiplier;
}

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

bool URunProgressionSubsystem::IsWeaponAvailable(EWeaponType Type, const TArray<FName>& Unlocked) const
{
	return FOptionUnlock::IsAvailable(Weapons.FindChecked(Type).bStartsLocked, WeaponRowName(Type), Unlocked);
}

bool URunProgressionSubsystem::IsPassiveAvailable(EPassiveType Type, const TArray<FName>& Unlocked) const
{
	return FOptionUnlock::IsAvailable(Passives.FindChecked(Type).bStartsLocked, PassiveRowName(Type), Unlocked);
}

int32 URunProgressionSubsystem::GetPoolSize() const
{
	const TArray<FName> Unlocked = GetUnlockedOptions();
	int32 Count = 0;
	for (uint8 W = 0; W < static_cast<uint8>(EWeaponType::Count); ++W)
	{
		Count += IsWeaponAvailable(static_cast<EWeaponType>(W), Unlocked) ? 1 : 0;
	}
	for (uint8 P = 0; P < static_cast<uint8>(EPassiveType::Count); ++P)
	{
		Count += IsPassiveAvailable(static_cast<EPassiveType>(P), Unlocked) ? 1 : 0;
	}
	return Count;
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

void URunProgressionSubsystem::DebugGrantWeapon(EWeaponType Weapon, int32 Copies)
{
	WeaponLevels.FindOrAdd(Weapon) = FMath::Clamp(Copies, 1, MaxCopies(Weapons.FindChecked(Weapon).MaxLevel));
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

	// The pool: everything unlocked, not cut for good and not yet at the copy cap.
	const TArray<FName> Unlocked = GetUnlockedOptions();
	TArray<FOfferOption> Pool;
	for (uint8 W = 0; W < static_cast<uint8>(EWeaponType::Count); ++W)
	{
		const EWeaponType Type = static_cast<EWeaponType>(W);
		const int32 Copies = WeaponLevels.FindRef(Type);
		if (IsWeaponAvailable(Type, Unlocked) && !CutWeapons.Contains(Type) && Copies < MaxCopies(Weapons.FindChecked(Type).MaxLevel))
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
		if (IsPassiveAvailable(Type, Unlocked) && !CutPassives.Contains(Type) && Copies < MaxCopies(Passives.FindChecked(Type).MaxLevel))
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

	// Options a lost film reel unlocked since the player last saw an offer go first and are marked NEW, so
	// "carry a reel out, see the new option in the next run" always happens in the first offer.
	UGameInstance* GameInstance = GetWorld()->GetGameInstance();
	if (UCampStockSubsystem* Stock = GameInstance ? GameInstance->GetSubsystem<UCampStockSubsystem>() : nullptr)
	{
		MoveNewUnlocksToFront(Pool, Stock->TakeNewOptions());
	}

	for (int32 i = 0; i < 3; ++i)
	{
		Offer.Add(Pool[i]);
	}
	bOfferPending = true;
	bCutUsedThisOffer = false;
	UGameAudioSubsystem::PlayIn(this, EGameSound::LevelUp);
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
		UGameAudioSubsystem::PlayIn(this, EGameSound::StarUp);
	}
}

bool URunProgressionSubsystem::CanCutFromOffer() const
{
	// One cut per level-up, only while permanent cuts remain (that rule keeps the pool from shrinking below an offer).
	return bOfferPending && !bCutUsedThisOffer && PermanentCutsLeft > 0 && Offer.Num() > 1;
}

void URunProgressionSubsystem::CutFromOffer(int32 Index)
{
	if (!CanCutFromOffer() || !Offer.IsValidIndex(Index))
	{
		return;
	}

	UGameAudioSubsystem::PlayIn(this, EGameSound::Cut);
	// The cut option leaves the pool for good and goes to the cutting-room floor, where the boss takes it.
	const FOfferOption Cut = Offer[Index];
	CuttingRoomFloor.Add(Cut);
	--PermanentCutsLeft;
	if (Cut.bIsWeapon)
	{
		CutWeapons.Add(Cut.Weapon);
	}
	else
	{
		CutPassives.Add(Cut.Passive);
	}
	Offer.RemoveAt(Index);
	bCutUsedThisOffer = true;
	UHudMessageSubsystem::Post(this, FString::Printf(TEXT("CUT: %s now belongs to the Boss"), *OptionName(Cut)), FLinearColor(1.0f, 0.4f, 0.4f), 3.0f);
}

void URunProgressionSubsystem::ChooseOption(int32 Index)
{
	if (!bOfferPending || !Offer.IsValidIndex(Index))
	{
		return;
	}

	// Take the chosen option. The others stay in the pool and may be offered again; only a cut removes one.
	const FOfferOption Chosen = Offer[Index];
	bOfferPending = false;
	Offer.Reset();
	ApplyOption(Chosen);

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

FName URunProgressionSubsystem::OptionId(const FOfferOption& Option)
{
	return Option.bIsWeapon ? FName(WeaponRowName(Option.Weapon)) : FName(PassiveRowName(Option.Passive));
}

void URunProgressionSubsystem::MoveNewUnlocksToFront(TArray<FOfferOption>& Pool, const TArray<FName>& NewIds)
{
	int32 Front = 0;
	for (int32 i = 0; i < Pool.Num(); ++i)
	{
		if (NewIds.Contains(OptionId(Pool[i])))
		{
			Pool[i].bIsNewUnlock = true;
			Pool.Swap(i, Front++);
		}
	}
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
	return FString::Printf(TEXT("%s%s  Lv%d -> Lv%d%s"), *NewMark, *Name, Option.CurrentLevel, After, *StarNote);
}

TArray<FOfferCardText> URunProgressionSubsystem::GetOfferCards() const
{
	TArray<FOfferCardText> Cards;
	if (!bOfferPending)
	{
		return Cards;
	}
	for (const FOfferOption& Option : Offer)
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
			const int32 StarsAfter = Stars.StarsForCopies(After);
			Card.Detail = FString::Printf(TEXT("Lv%d  >  Lv%d"), Option.CurrentLevel, After);
			if (StarsAfter > Stars.StarsForCopies(Option.CurrentLevel) && Option.CurrentLevel > 0)
			{
				Card.Note = FString::Printf(TEXT("%d-STAR UP!"), StarsAfter);
				Card.bHighlight = true;
			}
			else if (StarsAfter < 3)
			{
				Card.Note = FString::Printf(TEXT("%d more to %d-STAR"), Stars.CopiesToNextStar(After), StarsAfter + 1);
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
