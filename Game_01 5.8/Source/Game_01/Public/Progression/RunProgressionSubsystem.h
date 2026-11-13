// Director's Cut - Story 004 (design/game-brief.md, MVP feature 2: in-run growth).
#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "Enemies/EnemyTypes.h"
#include "Progression/ProgressionTypes.h"
#include "RunProgressionSubsystem.generated.h"

class AXpPickup;

/** One choice in a level-up offer: a new weapon, a weapon upgrade, or a passive upgrade. */
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

	/** Level (or stacks) before this pick. */
	UPROPERTY(BlueprintReadOnly, Category = "Progression")
	int32 CurrentLevel = 0;
};

/**
 * Owns in-run growth: XP, level, the level-up offer and the weapon/passive loadout.
 * Enemies drop XP pickups on death (AC-1); collecting enough XP soft-pauses the game and
 * shows a 3-choice offer (AC-2). Weapons reuse story 003's auto-attack hook (AC-3);
 * passives apply on pick (AC-4). Resets with the run (AC-7).
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

	/** 0 = not owned. */
	UFUNCTION(BlueprintPure, Category = "Progression")
	int32 GetWeaponLevel(EWeaponType Weapon) const { return WeaponLevels.FindRef(Weapon); }

	/** The weapon's stats (never null; defaults are filled at init). */
	const FWeaponRow& GetWeaponRow(EWeaponType Weapon) const { return Weapons.FindChecked(Weapon); }

	/** True while the level-up offer is showing and the game is soft-paused. */
	UFUNCTION(BlueprintPure, Category = "Progression")
	bool IsOfferPending() const { return bOfferPending; }

	/** Picks offer entry Index (0..2) and resumes play. No-op if no offer or a bad index. */
	UFUNCTION(BlueprintCallable, Category = "Progression")
	void PickOffer(int32 Index);

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

	void GrantXp(float Amount);
	void CheckLevelUp();
	void GenerateOffer();
	void ApplyOption(const FOfferOption& Option);
	void ShowOffer() const;
	FString DescribeOption(const FOfferOption& Option) const;

	// Loaded tuning (defaults are always present; tables override).
	FXpCurveRow XpCurve;
	TMap<EWeaponType, FWeaponRow> Weapons;
	TMap<EPassiveType, FPassiveRow> Passives;

	// Run state.
	int32 Level = 1;
	float Xp = 0.0f;
	float XpToNext = 30.0f;
	TMap<EWeaponType, int32> WeaponLevels;
	TMap<EPassiveType, int32> PassiveStacks;
	bool bOfferPending = false;
	TArray<FOfferOption> Offer;

	UPROPERTY(EditAnywhere, Category = "Progression", meta = (ClampMin = "0.0"))
	float MagnetRadius = 600.0f;

	UPROPERTY(EditAnywhere, Category = "Progression", meta = (ClampMin = "0.0"))
	float CollectRadius = 80.0f;

	UPROPERTY(EditAnywhere, Category = "Progression", meta = (ClampMin = "0.0"))
	float MagnetSpeed = 900.0f;

	UPROPERTY(Transient)
	TArray<TObjectPtr<AXpPickup>> Pickups;
};
