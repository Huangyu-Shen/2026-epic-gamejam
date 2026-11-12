// Director's Cut - Story 003 (design/game-brief.md, MVP feature 1). Story 005's cards call SpawnHorde / SpawnBoss.
#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "Enemies/EnemyTypes.h"
#include "EnemySpawnSubsystem.generated.h"

class AEnemy;
class AHeroCharacter;

/**
 * The wave scheduler. While a run is active it spawns Grunts on a ring around the hero at
 * (BaseRate + RampPerMinute * minutes) * heat spawn pressure per second, up to MaxAlive.
 * Access: GetWorld()->GetSubsystem<UEnemySpawnSubsystem>().
 */
UCLASS()
class GAME_01_API UEnemySpawnSubsystem : public UTickableWorldSubsystem
{
	GENERATED_BODY()

public:
	UEnemySpawnSubsystem();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawn")
	FSpawnTuning Tuning;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawn")
	FEnemyTuning GruntTuning;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawn")
	FEnemyTuning BossTuning;

	virtual void Initialize(FSubsystemCollectionBase& Collection) override;

	/** Spawns a burst of Grunts around the hero. Respects MaxAlive unless bIgnoreCap. Returns how many were spawned. */
	UFUNCTION(BlueprintCallable, Category = "Spawn")
	int32 SpawnHorde(int32 Count, bool bIgnoreCap = false);

	/** Spawns one Boss on the ring around the hero (ignores the cap). Null if there is no hero. */
	UFUNCTION(BlueprintCallable, Category = "Spawn")
	AEnemy* SpawnBoss();

	/** Grunts per second right now: ramped by run time and multiplied by heat spawn pressure. */
	UFUNCTION(BlueprintPure, Category = "Spawn")
	float GetCurrentSpawnRate() const;

	/** Seconds of active run time the ramp has seen. */
	UFUNCTION(BlueprintPure, Category = "Spawn")
	float GetRunTime() const { return RunTime; }

	/** Turns the scheduled spawning on or off (hordes and bosses still work). Used by the performance probe. */
	UFUNCTION(BlueprintCallable, Category = "Spawn")
	void SetSchedulerEnabled(bool bEnabled) { bSchedulerEnabled = bEnabled; }

	/** Clears run time and the spawn accumulator (new run). Does not remove enemies. */
	UFUNCTION(BlueprintCallable, Category = "Spawn")
	void ResetRun();

	virtual void Tick(float DeltaTime) override;
	virtual TStatId GetStatId() const override { RETURN_QUICK_DECLARE_CYCLE_STAT(UEnemySpawnSubsystem, STATGROUP_Tickables); }

private:
	/** Spawns one enemy of the given row at a random point on the ring around Hero. */
	AEnemy* SpawnOnRing(const AHeroCharacter& Hero, const FEnemyTuning& Row);

	UPROPERTY(VisibleInstanceOnly, Transient, Category = "Spawn", meta = (AllowPrivateAccess = "true"))
	float RunTime = 0.0f;

	float SpawnAccumulator = 0.0f;
	bool bSchedulerEnabled = true;
};
