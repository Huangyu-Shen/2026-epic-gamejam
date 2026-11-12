// Director's Cut - Story 003 (design/game-brief.md, MVP feature 1). Tuning structs for enemies, the swarm and the spawner.
#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "EnemyTypes.generated.h"

/** Log category for enemies, the swarm, the spawner and the performance probe. */
DECLARE_LOG_CATEGORY_EXTERN(LogEnemies, Log, All);

UENUM(BlueprintType)
enum class EEnemyKind : uint8
{
	Grunt,
	Boss,
};

/**
 * One enemy type. Defaults describe the Grunt; the spawn subsystem holds the Boss row.
 * Usable as a DataTable row: the member initializers are the fallback used when no row is found.
 */
USTRUCT(BlueprintType)
struct GAME_01_API FEnemyTuning : public FTableRowBase
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Enemy")
	EEnemyKind Kind = EEnemyKind::Grunt;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Enemy", meta = (ClampMin = "1.0"))
	float MaxHealth = 25.0f;

	/** Chase speed (cm/s). The hero walks at 600 by default. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Enemy", meta = (ClampMin = "0.0"))
	float MoveSpeed = 330.0f;

	/** Damage per contact hit, before the hero's damage-taken multiplier. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Enemy", meta = (ClampMin = "0.0"))
	float ContactDamage = 10.0f;

	/** Centre-to-centre distance (cm, XY plane) within which the enemy can hit the hero. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Enemy", meta = (ClampMin = "0.0"))
	float ContactRange = 90.0f;

	/** Seconds between one enemy's contact hits. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Enemy", meta = (ClampMin = "0.0"))
	float ContactCooldown = 0.8f;

	/** Uniform scale of the engine cube body (1.0 = 100 cm). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Enemy", meta = (ClampMin = "0.1"))
	float Scale = 0.6f;
};

/** How the swarm moves. Applies to all enemies. Usable as a DataTable row. */
USTRUCT(BlueprintType)
struct GAME_01_API FSwarmTuning : public FTableRowBase
{
	GENERATED_BODY()

	/** Enemies stop closing in at this fraction of their ContactRange, so they ring the hero instead of stacking. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Swarm", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float StopRangeFraction = 0.8f;

	/** Grid cell size (cm) for the push-apart pass. Must be at least the largest pair of body radii. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Swarm", meta = (ClampMin = "50.0"))
	float SeparationCellSize = 250.0f;

	/** Push speed (cm/s) per cm of overlap between two bodies. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Swarm", meta = (ClampMin = "0.0"))
	float SeparationStrength = 8.0f;

	/** Upper bound (cm/s) on the push-apart speed of one enemy. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Swarm", meta = (ClampMin = "0.0"))
	float MaxSeparationSpeed = 400.0f;
};

/** The wave scheduler's tuning. Usable as a DataTable row. */
USTRUCT(BlueprintType)
struct GAME_01_API FSpawnTuning : public FTableRowBase
{
	GENERATED_BODY()

	/** Grunts per second at run start, before heat. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawn", meta = (ClampMin = "0.0"))
	float BaseRate = 0.8f;

	/** Extra Grunts per second added for every minute of run time. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawn", meta = (ClampMin = "0.0"))
	float RampPerMinute = 1.5f;

	/** Hard cap on living enemies for scheduled spawns and (by default) hordes. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawn", meta = (ClampMin = "1"))
	int32 MaxAlive = 200;

	/** Inner radius (cm) of the spawn ring around the hero. Keep it outside the camera view. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawn", meta = (ClampMin = "0.0"))
	float RingMinRadius = 1500.0f;

	/** Outer radius (cm) of the spawn ring around the hero. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawn", meta = (ClampMin = "0.0"))
	float RingMaxRadius = 1900.0f;
};
