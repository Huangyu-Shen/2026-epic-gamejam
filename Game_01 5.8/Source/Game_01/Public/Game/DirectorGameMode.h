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

/**
 * Game mode for the Director's Cut: the hero is the default pawn.
 * With -PerfProbe on the command line it also runs a 150-enemy frame-rate probe, logs one
 * PERFPROBE line to LogEnemies and exits. Without the flag nothing probe-related runs.
 */
UCLASS()
class GAME_01_API ADirectorGameMode : public AGameModeBase
{
	GENERATED_BODY()

public:
	ADirectorGameMode();

	UPROPERTY(EditAnywhere, Category = "PerfProbe")
	FPerfProbeTuning PerfProbe;

	virtual void BeginPlay() override;
	virtual void Tick(float DeltaSeconds) override;

private:
	/** Godmodes the hero, spawns the horde and starts sampling. */
	void StartPerfProbe();

	FTimerHandle ProbeStartHandle;
	float SampledSeconds = 0.0f;
	float WorstFrameSeconds = 0.0f;
	int32 SampleCount = 0;
};
