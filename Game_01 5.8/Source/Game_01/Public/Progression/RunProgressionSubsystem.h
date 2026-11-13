// Director's Cut - Story 004 (design/game-brief.md, MVP feature 2: in-run growth).
#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "Enemies/EnemyTypes.h"
#include "Progression/ProgressionTypes.h"
#include "RunProgressionSubsystem.generated.h"

class AXpPickup;

/** One option in a level-up offer: a new weapon, another copy of a weapon, or another copy of a passive. */
USTRUCT(BlueprintType)
struct GAME_01_API FOfferOption
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "Progression")
	bool bIsWeapon = false;

	UPROPERTY(BlueprintReadOnly, Category = "Progression")
	EWeaponType Weapon = EWeaponType::Blade;

	UPROPERTY(BlueprintReadOnly, Category = "Progression")
	EPassiveType Passive = EPassiveType::Swiftness;

	/** True when a weapon is offered that the run does not own yet. */
	UPROPERTY(BlueprintReadOnly, Category = "Progression")
	bool bIsNewWeapon = false;

	/** Copies held before this pick (copies are also the level). */
	UPROPERTY(BlueprintReadOnly, Category = "Progression")
	int32 CurrentLevel = 0;
};

/**
 * Owns in-run growth: XP, level, the level-up offer and the weapon/passive loadout.
 * Enemies drop XP pickups on death; collecting enough XP soft-pauses the game and shows three options.
 * Brief v2: the player CUTS one option and keeps the other two. Each kept option adds one copy;
 * 3 copies make two stars and 9 make three (FStarTuning). While permanent cuts remain, the cut option
 * leaves the pool for the rest of the run. Resets with the run.
 */
UCLASS()
class GAME_01_API URunProgressionSubsystem : public UTickableWorldSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;

	UFUNCTION(BlueprintPure, Category = "Progression")
	int32 GetLevel() const { return Level; }

	UFUNCTION(BlueprintPure, Category = "Progression")
	float GetXp() const { return Xp; }

	UFUNCTION(BlueprintPure, Category = "Progression")
	float GetXpToNext() const { return XpToNext; }

	/** Passive multipliers (1.0 neutral; 0 bonus health when nothing is stacked). */
	UFUNCTION(BlueprintPure, Category = "Progression")
	float GetMoveSpeedMultiplier() const;

	UFUNCTION(BlueprintPure, Category = "Progression")
	float GetDamageMultiplier() const;

	UFUNCTION(BlueprintPure, Category = "Progression")
	float GetMaxHealthBonus() const;

	UFUNCTION(BlueprintPure, Category = "Progression")
	float GetXpGainMultiplier() const;

	/** Copies of the weapon held; 0 = not owned. Copies are also its level. */
	UFUNCTION(BlueprintPure, Category = "Progression")
	int32 GetWeaponLevel(EWeaponType Weapon) const { return WeaponLevels.FindRef(Weapon); }

	/** 0 = not owned, otherwise 1 to 3 stars. */
	UFUNCTION(BlueprintPure, Category = "Progression")
	int32 GetWeaponStars(EWeaponType Weapon) const { return Stars.StarsForCopies(WeaponLevels.FindRef(Weapon)); }

	UFUNCTION(BlueprintPure, Category = "Progression")
	int32 GetPassiveStars(EPassiveType Passive) const { return Stars.StarsForCopies(PassiveStacks.FindRef(Passive)); }

	/** Star multipliers for a weapon the run owns (1.0 at one star). */
	float GetWeaponStarDamageMultiplier(EWeaponType Weapon) const { return Stars.WeaponDamageMultiplier(GetWeaponStars(Weapon)); }
	float GetWeaponStarRangeMultiplier(EWeaponType Weapon) const { return Stars.WeaponRangeMultiplier(GetWeaponStars(Weapon)); }

	/** Cuts left this run that remove the cut option from the pool for good. */
	UFUNCTION(BlueprintPure, Category = "Progression")
	int32 GetPermanentCutsLeft() const { return PermanentCutsLeft; }

	/** Everything cut this run, oldest first (permanent or not). Kept for the boss that fights with the cut footage. */
	const TArray<FOfferOption>& GetCuttingRoomFloor() const { return CuttingRoomFloor; }

	/** One line for the HUD: every owned weapon and passive with its level and stars. */
	UFUNCTION(BlueprintPure, Category = "Progression")
	FString GetLoadoutSummary() const;

	/** The weapon's stats (never null; defaults are filled at init). */
	const FWeaponRow& GetWeaponRow(EWeaponType Weapon) const { return Weapons.FindChecked(Weapon); }

	/** True while the level-up offer is showing and the game is soft-paused. */
	UFUNCTION(BlueprintPure, Category = "Progression")
	bool IsOfferPending() const { return bOfferPending; }

	/** One line per option of the pending offer (empty when no offer is showing), for the level-up panel. */
	UFUNCTION(BlueprintPure, Category = "Progression")
	TArray<FString> GetOfferDescriptions() const;

	/** Cuts offer entry Index (0..2), keeps the other two and resumes play. No-op if no offer or a bad index. */
	UFUNCTION(BlueprintCallable, Category = "Progression")
	void CutOffer(int32 Index);

	/** Adds XP and opens the level-up offer once the threshold is reached. Used by pickups and by the -OfferProbe harness. */
	void GrantXp(float Amount);

	/** Clears XP, level, weapons and passives for a fresh run (starter weapon restored). */
	UFUNCTION(BlueprintCallable, Category = "Progression")
	void ResetProgression();

	/** Called by AXpPickup on BeginPlay / EndPlay. */
	void RegisterPickup(AXpPickup* Pickup);
	void UnregisterPickup(AXpPickup* Pickup);

	virtual void Tick(float DeltaTime) override;
	virtual TStatId GetStatId() const override { RETURN_QUICK_DECLARE_CYCLE_STAT(URunProgressionSubsystem, STATGROUP_Tickables); }

private:
	UFUNCTION()
	void HandleEnemyKilled(EEnemyKind Kind, FVector Location);

	void CheckLevelUp();
	void GenerateOffer();
	void ApplyOption(const FOfferOption& Option);
	void ShowOffer() const;
	FString DescribeOption(const FOfferOption& Option) const;
	FString OptionName(const FOfferOption& Option) const;
	int32 MaxCopies(int32 RowMaxLevel) const { return FMath::Max(RowMaxLevel, Stars.CopiesForThreeStar); }
	/** A passive's total effect: stacks x per-stack amount x its star multiplier. */
	float PassiveTotal(EPassiveType Passive, float PerStack) const;

	// Loaded tuning (defaults are always present; tables override).
	FXpCurveRow XpCurve;
	TMap<EWeaponType, FWeaponRow> Weapons;
	TMap<EPassiveType, FPassiveRow> Passives;
	FStarTuning Stars;

	// Run state.
	int32 Level = 1;
	float Xp = 0.0f;
	float XpToNext = 30.0f;
	TMap<EWeaponType, int32> WeaponLevels;
	TMap<EPassiveType, int32> PassiveStacks;
	bool bOfferPending = false;
	TArray<FOfferOption> Offer;
	TSet<EWeaponType> CutWeapons;
	TSet<EPassiveType> CutPassives;
	int32 PermanentCutsLeft = 0;
	TArray<FOfferOption> CuttingRoomFloor;

	UPROPERTY(EditAnywhere, Category = "Progression", meta = (ClampMin = "0.0"))
	float MagnetRadius = 600.0f;

	UPROPERTY(EditAnywhere, Category = "Progression", meta = (ClampMin = "0.0"))
	float CollectRadius = 80.0f;

	UPROPERTY(EditAnywhere, Category = "Progression", meta = (ClampMin = "0.0"))
	float MagnetSpeed = 900.0f;

	UPROPERTY(Transient)
	TArray<TObjectPtr<AXpPickup>> Pickups;
};
