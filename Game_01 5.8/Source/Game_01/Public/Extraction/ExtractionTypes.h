// Director's Cut - Story 002 (design/game-brief.md, MVP feature 4: extraction and death).
#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "Loot/LootTypes.h"
#include "ExtractionTypes.generated.h"

DECLARE_LOG_CATEGORY_EXTERN(LogExtraction, Log, All);

/** How a run ended. None = still running. */
UENUM(BlueprintType)
enum class ERunOutcome : uint8
{
	None,
	Extracted,
	Died
};

/** Extraction bar tuning. These member initializers are the editable defaults. */
USTRUCT(BlueprintType)
struct GAME_01_API FExtractionTuning : public FTableRowBase
{
	GENERATED_BODY()

	/** Seconds a player must stand in the zone to fill the bar. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Extraction", meta = (ClampMin = "0.1"))
	float ExtractionSeconds = 5.0f;

	/** True: leaving the zone empties the bar at once. False: it drains by ProgressDecayPerSecond. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Extraction")
	bool bResetProgressOnLeave = true;

	/** Bar fraction (0..1 of a full bar) lost per second outside the zone; used only when not resetting. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Extraction", meta = (ClampMin = "0.0"))
	float ProgressDecayPerSecond = 0.5f;
};

/** What a finished run did to the camp, and what the bag held. */
USTRUCT(BlueprintType)
struct GAME_01_API FRunResult
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "Run")
	ERunOutcome Outcome = ERunOutcome::None;

	/** Iron in the bag when the run ended (before the multiplier). */
	UPROPERTY(BlueprintReadOnly, Category = "Run")
	int32 BagIron = 0;

	/** Extracted: Iron added to camp stock (bag Iron x multiplier, rounded down). */
	UPROPERTY(BlueprintReadOnly, Category = "Run")
	int32 IronGained = 0;

	/** Died: Iron that was in the bag and is gone. */
	UPROPERTY(BlueprintReadOnly, Category = "Run")
	int32 IronLost = 0;

	/** Extracted: lost film reels carried out. */
	UPROPERTY(BlueprintReadOnly, Category = "Run")
	TArray<ERelicType> RelicsBanked;

	/** Extracted: upgrade options newly unlocked by those reels (filled by URunOutcomeSubsystem, not by Compute). */
	UPROPERTY(BlueprintReadOnly, Category = "Run")
	TArray<FName> OptionsUnlocked;

	/** Died: lost film reels that were in the bag and are gone. */
	UPROPERTY(BlueprintReadOnly, Category = "Run")
	TArray<ERelicType> RelicsLost;

	/** Earnings multiplier the settlement used. */
	UPROPERTY(BlueprintReadOnly, Category = "Run")
	float MultiplierUsed = 1.0f;
};

/** Pure settlement math. Touches no UObjects, so it can be tested without the editor. */
struct GAME_01_API FRunSettlement
{
	/**
	 * Extracted: IronGained = floor(BagIron * EarningsMultiplier), all reels carried out.
	 * Died: nothing gained, the whole bag reported as lost. None: empty result.
	 */
	static FRunResult Compute(int32 BagIron, const TArray<ERelicType>& BagRelics, float EarningsMultiplier, ERunOutcome Outcome);
};
