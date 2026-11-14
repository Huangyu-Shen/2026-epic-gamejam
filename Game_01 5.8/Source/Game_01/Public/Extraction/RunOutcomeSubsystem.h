// Director's Cut - Story 002 (design/game-brief.md, MVP feature 4). Story 005 sets the multiplier; story 007 calls StartNewRun.
#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "Extraction/ExtractionTypes.h"
#include "RunOutcomeSubsystem.generated.h"

class ULootBagComponent;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnRunEnded, const FRunResult&, Result);

/**
 * Owns the run state and ends the run exactly once, by extraction or death.
 * Ending settles the bag into the camp stock, clears the bag and resets heat.
 * Access: GetWorld()->GetSubsystem<URunOutcomeSubsystem>().
 */
UCLASS()
class GAME_01_API URunOutcomeSubsystem : public UWorldSubsystem
{
	GENERATED_BODY()

public:
	/** Fired once per run when it ends, with what was gained or lost. */
	UPROPERTY(BlueprintAssignable, Category = "Run")
	FOnRunEnded OnRunEnded;

	/** True from world start (and StartNewRun) until the run ends. */
	UFUNCTION(BlueprintPure, Category = "Run")
	bool IsRunActive() const { return bRunActive; }

	UFUNCTION(BlueprintPure, Category = "Run")
	ERunOutcome GetOutcome() const { return Outcome; }

	/** Result of the run that just ended (empty while a run is active). */
	UFUNCTION(BlueprintPure, Category = "Run")
	FRunResult GetLastResult() const { return LastResult; }

	/** Multiplier on extracted Iron. 1.0 unless run cards change it (story 005). */
	UFUNCTION(BlueprintPure, Category = "Run")
	float GetEarningsMultiplier() const { return EarningsMultiplier; }

	UFUNCTION(BlueprintCallable, Category = "Run")
	void SetEarningsMultiplier(float NewMultiplier);

	/** Success: bag Iron x multiplier to camp, lost film reels unlock their options. Ignored if the run already ended. */
	UFUNCTION(BlueprintCallable, Category = "Run")
	void ReportExtracted(ULootBagComponent* Bag);

	/** Failure: the whole bag is lost. Story 003's health calls this. Ignored if the run already ended. */
	UFUNCTION(BlueprintCallable, Category = "Run")
	void ReportPlayerDied(ULootBagComponent* Bag);

	/** Resets run state for the next run. Keeps the earnings multiplier. */
	UFUNCTION(BlueprintCallable, Category = "Run")
	void StartNewRun();

private:
	void EndRun(ULootBagComponent* Bag, ERunOutcome NewOutcome);

	UPROPERTY(VisibleInstanceOnly, Category = "Run", meta = (AllowPrivateAccess = "true"))
	bool bRunActive = true;

	UPROPERTY(VisibleInstanceOnly, Category = "Run", meta = (AllowPrivateAccess = "true"))
	ERunOutcome Outcome = ERunOutcome::None;

	UPROPERTY(EditAnywhere, Category = "Run", meta = (AllowPrivateAccess = "true", ClampMin = "0.0"))
	float EarningsMultiplier = 1.0f;

	UPROPERTY(VisibleInstanceOnly, Category = "Run", meta = (AllowPrivateAccess = "true"))
	FRunResult LastResult;
};
