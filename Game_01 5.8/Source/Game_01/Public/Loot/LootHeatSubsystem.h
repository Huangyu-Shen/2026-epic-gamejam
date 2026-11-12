#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "Loot/LootTypes.h"
#include "LootHeatSubsystem.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnHeatChanged, float, NewHeat, float, Delta);

/**
 * Global run heat. Rare pickups raise it; using a relic lowers it. Story 003's
 * spawner reads GetSpawnPressureMultiplier() (or binds OnHeatChanged).
 * Access: GetWorld()->GetSubsystem<ULootHeatSubsystem>().
 */
UCLASS()
class GAME_01_API ULootHeatSubsystem : public UWorldSubsystem
{
	GENERATED_BODY()

public:
	/** Fired whenever heat changes. Delta is signed. */
	UPROPERTY(BlueprintAssignable, Category = "Heat")
	FOnHeatChanged OnHeatChanged;

	/** Supplies tuning (the bag pushes its config here on BeginPlay). */
	UFUNCTION(BlueprintCallable, Category = "Heat")
	void SetTuning(const FLootTuning& InTuning);

	/** Current heat, 0..MaxHeat. */
	UFUNCTION(BlueprintPure, Category = "Heat")
	float GetHeat() const { return Heat; }

	/** Heat as 0..1. */
	UFUNCTION(BlueprintPure, Category = "Heat")
	float GetHeatNormalized() const;

	/**
	 * Hook for the spawner: multiplier on spawn pressure, lerped from
	 * SpawnPressureAtZeroHeat to SpawnPressureAtMaxHeat by normalized heat.
	 */
	UFUNCTION(BlueprintPure, Category = "Heat")
	float GetSpawnPressureMultiplier() const;

	/** Adds (or with negative Amount removes) heat, clamped to 0..MaxHeat. */
	UFUNCTION(BlueprintCallable, Category = "Heat")
	void AddHeat(float Amount);

	/** Adds HeatPerRarePickup. */
	UFUNCTION(BlueprintCallable, Category = "Heat")
	void AddRarePickupHeat();

	/** Removes HeatReductionOnRelicUse. */
	UFUNCTION(BlueprintCallable, Category = "Heat")
	void ReduceHeatForRelicUse();

	/** Sets heat to zero (start of a new run). */
	UFUNCTION(BlueprintCallable, Category = "Heat")
	void ResetHeat();

private:
	UPROPERTY(VisibleInstanceOnly, Category = "Heat", meta = (AllowPrivateAccess = "true"))
	float Heat = 0.0f;

	UPROPERTY()
	FLootTuning Tuning;
};
