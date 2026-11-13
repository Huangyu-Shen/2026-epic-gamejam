// Director's Cut - Story 007. The single auto-save slot: camp stock and building levels.
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/SaveGame.h"
#include "Loot/LootTypes.h"
#include "Camp/CampTypes.h"
#include "CampSaveGame.generated.h"

/** Persisted camp state. Bump SaveVersion if the layout changes incompatibly. */
UCLASS()
class GAME_01_API UCampSaveGame : public USaveGame
{
	GENERATED_BODY()

public:
	UPROPERTY()
	int32 SaveVersion = 1;

	UPROPERTY()
	int32 Iron = 0;

	UPROPERTY()
	TArray<ERelicType> WarehouseRelics;

	UPROPERTY()
	TMap<EBuildingType, int32> BuildingLevels;
};
