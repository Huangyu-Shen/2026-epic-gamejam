// Test probes for the run map, moved out of ADirectorGameMode (tech debt TD-001). Each probe is switched on by a command-line flag.
#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "TimerManager.h"
#include "DirectorProbeSubsystem.generated.h"

class AHeroCharacter;

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
 * Runs the automated test probes of the run map. It is created only when a probe flag is on the command line
 * (see GameDebug::HasProbeFlag) and never in a Shipping build, so a normal game has none of this running.
 *
 * Flags: -PerfProbe (150-enemy frame rate, logs PERFPROBE), -OfferProbe (level-up panel, cuts, weapon visuals; logs
 * OFFERPROBE and saves screenshots to Saved/OfferProbe/), -ReelProbe or -ReelProbe=die (a lost film reel through
 * extraction or death; -ReelCount=N), -PaceProbe (a bot plays a whole run; -PaceSpeed=N and the -Pace... overrides),
 * -WeaponShowcase, -PauseProbe, -EdgeProbe, -BossProbe and -ZoneShot (screenshots). Every probe exits the game when done.
 */
UCLASS()
class GAME_01_API UDirectorProbeSubsystem : public UTickableWorldSubsystem
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, Category = "PerfProbe")
	FPerfProbeTuning PerfProbe;

	UPROPERTY(EditAnywhere, Category = "OfferProbe")
	FOfferProbeTuning OfferProbe;

	virtual bool ShouldCreateSubsystem(UObject* Outer) const override;
	virtual void OnWorldBeginPlay(UWorld& InWorld) override;
	virtual void Tick(float DeltaSeconds) override;
	virtual TStatId GetStatId() const override;
	virtual bool IsTickable() const override;

private:
	void StartWeaponShowcase();
	void StartPauseProbe();
	void StartEdgeProbe();
	void StartBossProbe();
	void StartZoneShot();

	/** Godmodes the hero, spawns the horde and starts sampling. */
	void StartPerfProbe();

	/** Runs one step of the level-up probe; called on a repeating timer. */
	void StepOfferProbe();

	/** Runs one step of the lost-reel probe; called on a repeating timer. */
	void StepReelProbe();

	/** One game-second of the pace bot; logs every 30 s and ends the run on death or at 10 minutes. */
	void StepPaceProbe();

	/** Logs the effective numbers the game is using (tables or C++ fallbacks), so two builds can be compared. */
	void LogTuning(const AHeroCharacter& Hero) const;

	FTimerHandle ProbeStartHandle;
	FTimerHandle OfferProbeHandle;
	FTimerHandle ReelProbeHandle;
	FTimerHandle PaceHandle;
	bool bTickEnabled = false;
	bool bPaceProbe = false;
	float PaceSeconds = 0.0f;
	float PaceAngle = 0.0f;
	int32 OfferProbeStep = 0;
	int32 ReelProbeStep = 0;
	bool bChainShotTaken = false;
	int32 ChainWaitSteps = 0;
	bool bReelProbeDies = false;
	float SampledSeconds = 0.0f;
	float WorstFrameSeconds = 0.0f;
	int32 SampleCount = 0;
};
