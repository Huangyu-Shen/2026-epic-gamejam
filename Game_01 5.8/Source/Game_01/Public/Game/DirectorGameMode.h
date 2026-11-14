// Director's Cut - Story 003 (design/game-brief.md, MVP feature 1). Replaces AStandInGameMode.
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "DirectorGameMode.generated.h"

/** Tuning for the automated performance probe (command line -PerfProbe). */
USTRUCT(BlueprintType)
struct GAME_01_API FPerfProbeTuning
{
	GENERATED_BODY()

	/** Seconds after BeginPlay before the horde is spawned. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PerfProbe", meta = (ClampMin = "0.0"))
	float StartDelaySeconds = 3.0f;

	/** Seconds of frame time sampled before the result is logged and the game exits. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PerfProbe", meta = (ClampMin = "0.1"))
	float SampleSeconds = 10.0f;

	/** Enemies alive during the sample (the cap is bypassed). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PerfProbe", meta = (ClampMin = "1"))
	int32 EnemyCount = 150;
};

/** Tuning for the level-up screenshot probe (command line -OfferProbe). */
USTRUCT(BlueprintType)
struct GAME_01_API FOfferProbeTuning
{
	GENERATED_BODY()

	/** Seconds after BeginPlay before the first level-up is forced. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "OfferProbe", meta = (ClampMin = "0.0"))
	float StartDelaySeconds = 3.0f;

	/** Seconds between probe steps, so each screenshot is taken after the UI has drawn. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "OfferProbe", meta = (ClampMin = "0.1"))
	float StepSeconds = 1.5f;

	/** Level-ups played automatically (always cutting the first option) between the two offer screenshots. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "OfferProbe", meta = (ClampMin = "1"))
	int32 LevelUps = 6;
};

/**
 * Game mode for the Director's Cut: the hero is the default pawn.
 * With -PerfProbe on the command line it also runs a 150-enemy frame-rate probe, logs one
 * PERFPROBE line to LogEnemies and exits. With -OfferProbe it forces level-ups, cuts the first option
 * each time, saves three screenshots of the level-up panel and the HUD to Saved/OfferProbe/, logs
 * OFFERPROBE lines and exits. With -ReelProbe (or -ReelProbe=die) it puts a lost film reel in the bag, ends the run
 * by extraction (or death), saves screenshots of the HUD and the result screen to Saved/ReelProbe/, logs REELPROBE
 * lines and exits. Without these flags nothing probe-related runs.
 */
UCLASS()
class GAME_01_API ADirectorGameMode : public AGameModeBase
{
	GENERATED_BODY()

public:
	ADirectorGameMode();

	UPROPERTY(EditAnywhere, Category = "PerfProbe")
	FPerfProbeTuning PerfProbe;

	UPROPERTY(EditAnywhere, Category = "OfferProbe")
	FOfferProbeTuning OfferProbe;

	virtual void BeginPlay() override;
	virtual void Tick(float DeltaSeconds) override;

private:
	/** Godmodes the hero, spawns the horde and starts sampling. */
	void StartPerfProbe();

	/** Runs one step of the level-up probe; called on a repeating timer. */
	void StepOfferProbe();

	/** Runs one step of the lost-reel probe; called on a repeating timer. */
	void StepReelProbe();

	FTimerHandle ProbeStartHandle;
	FTimerHandle OfferProbeHandle;
	FTimerHandle ReelProbeHandle;
	int32 OfferProbeStep = 0;
	int32 ReelProbeStep = 0;
	bool bReelProbeDies = false;
	float SampledSeconds = 0.0f;
	float WorstFrameSeconds = 0.0f;
	int32 SampleCount = 0;
};
