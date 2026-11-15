// Director's Cut - Story 006. Camp buildings, upgrade costs and their run-stat bonuses.
#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Camp/CampTypes.h"
#include "Loot/LootTypes.h"
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

	/** What upgrading Type costs: the smithy takes stone, the clinic takes wood. */
	static ELootResource GetCostResource(EBuildingType Type) { return Type == EBuildingType::Smithy ? ELootResource::Stone : ELootResource::Wood; }

	/** Cost (in GetCostResource) of the next upgrade for Type. */
	UFUNCTION(BlueprintPure, Category = "Camp")
	int32 GetUpgradeCost(EBuildingType Type) const;

	UFUNCTION(BlueprintPure, Category = "Camp")
	bool IsMaxLevel(EBuildingType Type) const;

	/** Spends the cost (wood or stone) and raises the level on success. On failure fills Reason and returns false. */
	UFUNCTION(BlueprintCallable, Category = "Camp")
	bool TryUpgrade(EBuildingType Type, FText& Reason);

	/** Flat weapon damage from Smithy levels (applied at run start). */
	UFUNCTION(BlueprintPure, Category = "Camp")
	float GetWeaponDamageBonus() const;

	/** Flat max health from Clinic levels (applied at run start). */
	UFUNCTION(BlueprintPure, Category = "Camp")
	float GetMaxHealthBonus() const;

	/** Current building levels (copy), for saving. */
	TMap<EBuildingType, int32> GetLevels() const { return Levels; }

	/** Replaces all levels (save-game restore, story 007); each is clamped to [0, MaxLevel]. */
	void RestoreLevels(const TMap<EBuildingType, int32>& InLevels);

	/**
	 * Film cards slotted in the editing room, kept here so they survive the camp -> run level change
	 * (the film-card subsystem lives in the world and is rebuilt per level).
	 */
	TArray<FName> PendingFilmCards;

	/** The building's data row (fallback defaults if the table is missing). */
	const FBuildingRow& GetBuildingRow(EBuildingType Type) const;

private:
	UPROPERTY(VisibleInstanceOnly, Category = "Camp", meta = (AllowPrivateAccess = "true"))
	TMap<EBuildingType, FBuildingRow> Buildings;

	UPROPERTY(VisibleInstanceOnly, Category = "Camp", meta = (AllowPrivateAccess = "true"))
	TMap<EBuildingType, int32> Levels;
};
