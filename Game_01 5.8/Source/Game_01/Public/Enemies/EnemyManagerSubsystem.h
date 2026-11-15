// Director's Cut - Story 003 (design/game-brief.md, MVP feature 1).
#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "Enemies/EnemyTypes.h"
#include "EnemyManagerSubsystem.generated.h"

class AEnemy;

/** Fired when an enemy is killed. Story 004 uses it for XP. */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnEnemyKilled, EEnemyKind, Kind, FVector, Location);

/**
 * Owns every live enemy and runs them all in one tick: chase the hero on the XY plane,
 * cheap grid push-apart, face the movement direction and contact damage with a per-enemy cooldown.
 * Enemies themselves never tick. Access: GetWorld()->GetSubsystem<UEnemyManagerSubsystem>().
 */
UCLASS()
class GAME_01_API UEnemyManagerSubsystem : public UTickableWorldSubsystem
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Enemies")
	FSwarmTuning Tuning;

	virtual void Initialize(FSubsystemCollectionBase& Collection) override;

	/** Hook for story 004: fires for every kill. */
	UPROPERTY(BlueprintAssignable, Category = "Enemies")
	FOnEnemyKilled OnEnemyKilled;

	/** Adds an enemy to the swarm. Called by the spawner right after spawning. */
	void RegisterEnemy(AEnemy* Enemy);

	/** Removes an enemy from the swarm (called when it is destroyed). */
	void UnregisterEnemy(AEnemy* Enemy);

	/** Counts the kill and broadcasts OnEnemyKilled. Called by AEnemy just before it is destroyed. */
	void NotifyEnemyKilled(AEnemy* Enemy);

	/** The living enemy nearest to Location within Range (cm, XY plane), or null. */
	UFUNCTION(BlueprintPure, Category = "Enemies")
	AEnemy* FindNearestEnemy(FVector Location, float Range) const;

	/** The living enemies (read-only). Story 004's weapons filter this list. */
	const TArray<TObjectPtr<AEnemy>>& GetEnemies() const { return Enemies; }

	UFUNCTION(BlueprintPure, Category = "Enemies")
	int32 GetAliveCount() const { return Enemies.Num(); }

	/** Destroys every living enemy without counting kills. */
	UFUNCTION(BlueprintCallable, Category = "Enemies")
	void ClearAll();

	/** Sets the kill counter to zero (new run). */
	UFUNCTION(BlueprintCallable, Category = "Enemies")
	void ResetKillCount() { KillCount = 0; }

	virtual void Tick(float DeltaTime) override;
	virtual TStatId GetStatId() const override { RETURN_QUICK_DECLARE_CYCLE_STAT(UEnemyManagerSubsystem, STATGROUP_Tickables); }

private:
	UPROPERTY(Transient)
	TArray<TObjectPtr<AEnemy>> Enemies;

	UPROPERTY(VisibleInstanceOnly, Transient, Category = "Enemies", meta = (AllowPrivateAccess = "true"))
	int32 KillCount = 0;

	// Scratch buffers reused every tick so the loop does not allocate.
	TArray<FVector> Positions;
	TArray<float> Yaws;
	TArray<FVector2D> Pushes;
	TMap<FIntPoint, TArray<int32>> Grid;
};
