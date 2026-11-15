// Director's Cut - Story 004 (design/game-brief.md, MVP feature 2: in-run growth).
#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "Enemies/EnemyTypes.h"
#include "Progression/ProgressionTypes.h"
#include "Progression/OfferRules.h"
#include "Progression/OfferText.h"
#include "Progression/RunLoadout.h"
#include "RunProgressionSubsystem.generated.h"

class AXpPickup;

/**
 * Owns in-run growth: XP, level, the level-up offer and the weapon/passive loadout.
 * Enemies drop XP pickups on death; collecting enough XP soft-pauses the game and shows three options.
 * Each level-up offers three options: the player takes one (it adds a copy) and may first cut one. 3 copies make
 * two stars and 9 make three (FStarTuning). While permanent cuts remain, a cut option leaves the pool for the rest of the
 * run and goes to the boss. Resets with the run. The loadout, the offer rules and the offer wording live in
 * FRunLoadout, FOfferRules and FOfferText; this class adds the world: sounds, messages, XP pickups and the hero.
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

	/** Haste: every weapon's interval is divided by this (1.0 neutral). */
	UFUNCTION(BlueprintPure, Category = "Progression")
	float GetAttackSpeedMultiplier() const;

	/** Magnet: distance (cm) within which XP pickups fly to the hero. */
	UFUNCTION(BlueprintPure, Category = "Progression")
	float GetMagnetRadius() const;

	/** Reach: multiplier on every weapon's range (1.0 with no stacks). */
	float GetRangeMultiplier() const;

	/** Armor: share of contact damage the hero ignores, 0 to 0.75. */
	float GetDamageReduction() const;

	/** Regen: health restored per second. */
	float GetRegenPerSecond() const;

	/** Crit: chance an attack hits for GetCritMultiplier() times damage, 0 to 0.9. */
	float GetCritChance() const;
	float GetCritMultiplier() const;

	/** How many weapons and passives can appear in an offer right now (locked options not counted). */
	UFUNCTION(BlueprintPure, Category = "Progression")
	int32 GetPoolSize() const;

	/** Copies of the weapon held; 0 = not owned. Copies are also its level. */
	UFUNCTION(BlueprintPure, Category = "Progression")
	int32 GetWeaponLevel(EWeaponType Weapon) const { return Loadout.WeaponCopies(Weapon); }

	/** 0 = not owned, otherwise 1 to 3 stars. */
	UFUNCTION(BlueprintPure, Category = "Progression")
	int32 GetWeaponStars(EWeaponType Weapon) const { return Loadout.WeaponStars(Weapon); }

	/** Star multipliers for a weapon the run owns (1.0 at one star). */
	float GetWeaponStarDamageMultiplier(EWeaponType Weapon) const { return Loadout.WeaponStarDamageMultiplier(Weapon); }
	float GetWeaponStarRangeMultiplier(EWeaponType Weapon) const { return Loadout.WeaponStarRangeMultiplier(Weapon); }

	/** Cuts left this run that remove the cut option from the pool for good. */
	UFUNCTION(BlueprintPure, Category = "Progression")
	int32 GetPermanentCutsLeft() const { return Rules.PermanentCutsLeft; }

	/** Everything cut this run, oldest first (permanent or not). Kept for the boss that fights with the cut footage. */
	const TArray<FOfferOption>& GetCuttingRoomFloor() const { return Rules.CuttingRoomFloor; }

	/** One line for the HUD: every owned weapon and passive with its level and stars. */
	UFUNCTION(BlueprintPure, Category = "Progression")
	FString GetLoadoutSummary() const;

	/** The weapon's stats (never null; defaults are filled at init). */
	const FWeaponRow& GetWeaponRow(EWeaponType Weapon) const { return Loadout.Weapons.FindChecked(Weapon); }

	/** True while the level-up offer is showing and the game is soft-paused. */
	UFUNCTION(BlueprintPure, Category = "Progression")
	bool IsOfferPending() const { return Rules.bPending; }

	/** The pending offer as three short pieces per option, for the level-up cards (empty when no offer is showing). */
	TArray<FOfferCardText> GetOfferCards() const;

	/** One line per option of the pending offer (empty when no offer is showing), for the level-up panel. */
	UFUNCTION(BlueprintPure, Category = "Progression")
	TArray<FString> GetOfferDescriptions() const;

	/** Takes offer entry Index and resumes play. The others stay in the pool. No-op if no offer or a bad index. */
	UFUNCTION(BlueprintCallable, Category = "Progression")
	void ChooseOption(int32 Index);

	/** Optional: removes offer entry Index from the pool for good and gives it to the boss. The offer stays open. */
	UFUNCTION(BlueprintCallable, Category = "Progression")
	void CutFromOffer(int32 Index);

	/** True while a cut is allowed in the pending offer (one per level-up, while permanent cuts remain). */
	UFUNCTION(BlueprintPure, Category = "Progression")
	bool CanCutFromOffer() const;

	/** Test harness only (-WeaponShowcase): gives the run Copies copies of a weapon without an offer. */
	void DebugGrantWeapon(EWeaponType Weapon, int32 Copies);

	/** Test harness only (-BossProbe): puts an option on the cutting-room floor as if the player had cut it. */
	void DebugCut(const FOfferOption& Option) { Rules.CuttingRoomFloor.Add(Option); }

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
	/** The upgrade options the camp has unlocked (empty when there is no camp stock). */
	TArray<FName> GetUnlockedOptions() const;
	void ApplyOption(const FOfferOption& Option);
	void ShowOffer() const;

	// Loaded tuning (defaults are always present; tables override).
	FXpCurveRow XpCurve;

	/** What the run owns and what it is worth (tuning rows, copies, stars, passive effects). */
	FRunLoadout Loadout;

	// Run state.
	int32 Level = 1;
	float Xp = 0.0f;
	float XpToNext = 30.0f;

	/** The level-up offer, the cuts and the choice (pure rules, see OfferRules.h). */
	FOfferRules Rules;

	/** The shuffle of the offer pool, seeded differently each session. */
	FRandomStream Random;

	UPROPERTY(EditAnywhere, Category = "Progression", meta = (ClampMin = "0.0"))
	float CollectRadius = 80.0f;

	UPROPERTY(EditAnywhere, Category = "Progression", meta = (ClampMin = "0.0"))
	float MagnetSpeed = 900.0f;

	UPROPERTY(Transient)
	TArray<TObjectPtr<AXpPickup>> Pickups;
};
