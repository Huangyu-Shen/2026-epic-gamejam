// Director's Cut - Story 002. In-memory camp stock; persistence is story 007, camp UI/buildings story 006.
#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Loot/LootTypes.h"
#include "CampStockSubsystem.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnCampStockChanged);

/**
 * Camp Iron and the warehouse relic list, kept for the whole game session.
 * Access: GetGameInstance()->GetSubsystem<UCampStockSubsystem>().
 */
UCLASS()
class GAME_01_API UCampStockSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	/** Fired whenever Iron or the warehouse changes. */
	UPROPERTY(BlueprintAssignable, Category = "Camp")
	FOnCampStockChanged OnStockChanged;

	/** Adds Iron to the camp. Non-positive amounts are ignored. */
	UFUNCTION(BlueprintCallable, Category = "Camp")
	void AddIron(int32 Amount);

	/** Adds relics to the warehouse. An empty list is ignored. */
	UFUNCTION(BlueprintCallable, Category = "Camp")
	void AddRelics(const TArray<ERelicType>& NewRelics);

	UFUNCTION(BlueprintPure, Category = "Camp")
	int32 GetIron() const { return Iron; }

	/** Warehouse relics (copy). */
	UFUNCTION(BlueprintPure, Category = "Camp")
	TArray<ERelicType> GetWarehouseRelics() const { return WarehouseRelics; }

private:
	UPROPERTY(VisibleInstanceOnly, Category = "Camp", meta = (AllowPrivateAccess = "true"))
	int32 Iron = 0;

	UPROPERTY(VisibleInstanceOnly, Category = "Camp", meta = (AllowPrivateAccess = "true"))
	TArray<ERelicType> WarehouseRelics;
};
