// Director's Cut - Story 007. The single auto-save slot: camp stock and building levels.
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/SaveGame.h"
#include "Loot/LootTypes.h"
#include "Camp/CampTypes.h"
#include "CampSaveGame.generated.h"

/** What to do with a save file, from the version it was written with. Pure, so it is unit tested. */
struct GAME_01_API FSaveVersionCheck
{
	enum class EResult : uint8
	{
		/** Written by this version of the game: load it as it is. */
		Current,
		/** Written by an older version: migrate it while loading, and keep a backup of the file first. */
		NeedsMigration,
		/** Written by a newer version of the game: do not load it and never overwrite it (back it up). */
		TooNew,
	};

	/**
	 * True when the first bytes look like an Unreal save file (they start with the "GVAS" tag). A file that does not,
	 * for example an empty or half-written one, must not be handed to the engine's loader: it asserts on a bad name table.
	 */
	static bool LooksLikeSaveFile(const uint8* Bytes, int32 Count)
	{
		return Bytes && Count >= 4 && Bytes[0] == 'G' && Bytes[1] == 'V' && Bytes[2] == 'A' && Bytes[3] == 'S';
	}

	static EResult Classify(int32 FileVersion, int32 CurrentVersion)
	{
		if (FileVersion > CurrentVersion)
		{
			return EResult::TooNew;
		}
		return FileVersion < CurrentVersion ? EResult::NeedsMigration : EResult::Current;
	}
};

/** Persisted camp state. Bump CurrentSaveVersion (and add a migration) if the layout changes incompatibly. */
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
