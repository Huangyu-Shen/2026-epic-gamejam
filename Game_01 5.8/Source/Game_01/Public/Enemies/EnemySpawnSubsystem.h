// Director's Cut - Story 003 (design/game-brief.md, MVP feature 1). Story 005's cards call SpawnHorde / SpawnBoss.
#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "Enemies/EnemyTypes.h"
#include "EnemySpawnSubsystem.generated.h"

class AEnemy;
class AHeroCharacter;
class USkeletalMesh;
class UAnimSequence;

/** One enemy model: a skeletal mesh and the animation it loops while chasing. Loaded with the game, so the cook includes them. */
USTRUCT()
struct FEnemyVisualAsset
{
	GENERATED_BODY()

	UPROPERTY()
	TObjectPtr<USkeletalMesh> Mesh;

	UPROPERTY()
	TObjectPtr<UAnimSequence> Move;

	/** Degrees added to the model's facing so it looks where it walks. */
	float YawOffset = 0.0f;

	/** Size of the model against the cube it replaces (the pack's monsters are small next to the hero). */
	float Scale = 1.0f;
};

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

	/** The models Grunts pick from at random, and the Boss's model. Empty when the asset packs are not installed (the cubes stay). */
	UPROPERTY()
	TArray<FEnemyVisualAsset> GruntVisuals;

	UPROPERTY()
	FEnemyVisualAsset BossVisual;

	float SpawnAccumulator = 0.0f;
	bool bSchedulerEnabled = true;
};
