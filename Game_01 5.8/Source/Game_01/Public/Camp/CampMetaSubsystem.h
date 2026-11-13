// Director's Cut - Story 006. Camp buildings, upgrade costs and their run-stat bonuses.
#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Camp/CampTypes.h"
#include "CampMetaSubsystem.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnCampMetaChanged);

/**
 * Building levels and their upgrade costs, kept for the whole game session.
 * Access: GetGameInstance()->GetSubsystem<UCampMetaSubsystem>().
 */
UCLASS()
class GAME_01_API UCampMetaSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;

	/** Fired whenever a building level changes. */
	UPROPERTY(BlueprintAssignable, Category = "Camp")
	FOnCampMetaChanged OnMetaChanged;

	UFUNCTION(BlueprintPure, Category = "Camp")
	int32 GetBuildingLevel(EBuildingType Type) const;

	/** Iron cost of the next upgrade for Type. */
	UFUNCTION(BlueprintPure, Category = "Camp")
	int32 GetUpgradeCost(EBuildingType Type) const;

	UFUNCTION(BlueprintPure, Category = "Camp")
	bool IsMaxLevel(EBuildingType Type) const;

	/** Spends the Iron and raises the level on success. On failure fills Reason and returns false. */
	UFUNCTION(BlueprintCallable, Category = "Camp")
	bool TryUpgrade(EBuildingType Type, FText& Reason);

	/** Flat weapon damage from Smithy levels (applied at run start). */
	UFUNCTION(BlueprintPure, Category = "Camp")
	float GetWeaponDamageBonus() const;

	/** Flat max health from Clinic levels (applied at run start). */
	UFUNCTION(BlueprintPure, Category = "Camp")
	float GetMaxHealthBonus() const;

	/** Dismantles the warehouse relic at Index into Iron. Returns the Iron gained (0 on a bad index). */
	UFUNCTION(BlueprintCallable, Category = "Camp")
	int32 DismantleRelic(int32 Index);

	/** Current building levels (copy), for saving. */
	TMap<EBuildingType, int32> GetLevels() const { return Levels; }

	/** Replaces all levels (save-game restore, story 007); each is clamped to [0, MaxLevel]. */
	void RestoreLevels(const TMap<EBuildingType, int32>& InLevels);

	/** Resets every building to level 0 (fresh game). */
	UFUNCTION(BlueprintCallable, Category = "Camp")
	void ResetCamp();

	/** The building's data row (fallback defaults if the table is missing). */
	const FBuildingRow& GetBuildingRow(EBuildingType Type) const;

private:
	UPROPERTY(VisibleInstanceOnly, Category = "Camp", meta = (AllowPrivateAccess = "true"))
	TMap<EBuildingType, FBuildingRow> Buildings;

	UPROPERTY(VisibleInstanceOnly, Category = "Camp", meta = (AllowPrivateAccess = "true"))
	TMap<EBuildingType, int32> Levels;

	/** Iron each dismantled relic yields (fixed default; not in the building table). */
	int32 RelicDismantleIron = 8;
};
