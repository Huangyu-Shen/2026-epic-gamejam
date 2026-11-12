#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "Loot/LootTypes.h"
#include "LootConfig.generated.h"

class UDataTable;

/**
 * Data-driven loot configuration. Create a Data Asset of this class to tune.
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

	/** Optional table of FRelicRow. Overrides DefaultRelics when it has rows. */
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

	/** Inline relic definitions used when RelicTable is missing/empty. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Defaults")
	TArray<FRelicRow> DefaultRelics;

	/** Inline container definitions used when ContainerTable is missing/empty. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Defaults")
	TArray<FLootContainerRow> DefaultContainers;

	/** Effective tuning (table row if present, else DefaultTuning). */
	UFUNCTION(BlueprintCallable, Category = "Loot")
	FLootTuning GetTuning() const;

	/** All effective relic rows. */
	UFUNCTION(BlueprintCallable, Category = "Loot")
	TArray<FRelicRow> GetRelicRows() const;

	/** Finds the effective row for a relic type. Returns false if none. */
	UFUNCTION(BlueprintCallable, Category = "Loot")
	bool FindRelicRow(ERelicType Type, FRelicRow& OutRow) const;

	/** Finds the effective row for a container type. Returns false if none. */
	UFUNCTION(BlueprintCallable, Category = "Loot")
	bool FindContainerRow(ELootContainerType Type, FLootContainerRow& OutRow) const;

	/** Weighted random relic type from the effective rows. Returns false if there are no rows. */
	UFUNCTION(BlueprintCallable, Category = "Loot")
	bool PickRandomRelic(ERelicType& OutType) const;
};
