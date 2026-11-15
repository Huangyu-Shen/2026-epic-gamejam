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
	/** Version written by the current game: 1 = relics in a warehouse, 2 = unlocked upgrade options (story 008). */
	static constexpr int32 CurrentSaveVersion = 2;

	/**
	 * The version stored in the file. The default must stay 1: a save stores only the values that differ from the
	 * class defaults, so a v1 file has no SaveVersion at all and reads back as this default. UCampSaveSubsystem::SaveNow
	 * sets CurrentSaveVersion explicitly, which differs from 1 and is therefore written.
	 */
	UPROPERTY()
	int32 SaveVersion = 1;

	/** Wood. The field keeps its old name, Iron, so older saves still load: their Iron becomes wood. */
	UPROPERTY()
	int32 Iron = 0;

	/** Stone. Missing in older saves, which read it back as 0. */
	UPROPERTY()
	int32 Stone = 0;

	/** Read from a v1 save only (each relic becomes its unlocked option); never written. */
	UPROPERTY()
	TArray<ERelicType> WarehouseRelics;

	/** Unlocked upgrade options, by stable name (not by enum number). */
	UPROPERTY()
	TArray<FName> UnlockedOptions;

	/** Unlocked options the player has not yet seen in a level-up offer (story 009). Added without a version bump: an older v2 file simply has none. */
	UPROPERTY()
	TArray<FName> NewOptions;

	UPROPERTY()
	TMap<EBuildingType, int32> BuildingLevels;
};
