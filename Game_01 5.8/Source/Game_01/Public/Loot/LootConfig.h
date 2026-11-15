#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "Loot/LootTypes.h"
#include "LootConfig.generated.h"

class UDataTable;

/**
 * Data-driven loot configuration. Tuned through the data tables named in the Director Tuning settings.
 * Each table is optional: if a table is unset or empty, the inline defaults
 * below (filled in the constructor, editable on the asset) are used, so the
 * game runs with no authored content.
 */
UCLASS(BlueprintType)
class GAME_01_API ULootConfig : public UDataAsset
{
	GENERATED_BODY()

public:
	ULootConfig();

	/** Builds a config whose tables come from Project Settings > Director Tuning (any unset table keeps the C++ defaults). */
	static ULootConfig* CreateFromSettings(UObject* Outer);

	/** Optional table of FRelicRow (the lost film reels). Overrides DefaultRelics when it has rows. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Tables", meta = (RequiredAssetDataTags = "RowStructure=/Script/Game_01.RelicRow"))
	TObjectPtr<UDataTable> RelicTable;

	/** Optional table of FLootContainerRow. Overrides DefaultContainers when it has rows. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Tables", meta = (RequiredAssetDataTags = "RowStructure=/Script/Game_01.LootContainerRow"))
	TObjectPtr<UDataTable> ContainerTable;

	/** Optional table holding FLootTuning; TuningRowName selects the row. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Tables", meta = (RequiredAssetDataTags = "RowStructure=/Script/Game_01.LootTuning"))
	TObjectPtr<UDataTable> TuningTable;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Tables")
	FName TuningRowName = TEXT("Default");

	/** Inline tuning used when TuningTable/row is missing. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Defaults")
	FLootTuning DefaultTuning;

	/** Inline reel definitions used when RelicTable is missing/empty. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Defaults")
	TArray<FRelicRow> DefaultRelics;

	/** Inline container definitions used when ContainerTable is missing/empty. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Defaults")
	TArray<FLootContainerRow> DefaultContainers;

	/** Effective tuning (table row if present, else DefaultTuning). */
	UFUNCTION(BlueprintCallable, Category = "Loot")
	FLootTuning GetTuning() const;

	/** All effective reel rows. */
	UFUNCTION(BlueprintCallable, Category = "Loot")
	TArray<FRelicRow> GetRelicRows() const;

	/** Finds the effective row for a container type. Returns false if none. */
	UFUNCTION(BlueprintCallable, Category = "Loot")
	bool FindContainerRow(ELootContainerType Type, FLootContainerRow& OutRow) const;

	/** True when the tuning came from TuningTable's row, false when the inline defaults were used. */
	bool IsTuningFromTable() const;

	/**
	 * Weighted random reel type among the reels that can still drop (see FRelicRow::IsDroppable): not already
	 * unlocked, mapped to an option, and not already in the bag. Returns false if none can drop.
	 */
	bool PickRandomRelic(const TArray<FName>& UnlockedOptions, const TArray<ERelicType>& ReelsInBag, ERelicType& OutType) const;
};
