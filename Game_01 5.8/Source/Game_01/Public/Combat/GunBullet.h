// Director's Cut - Story 011 (added at the playtest): the Gun weapon's bullet and the Orbit weapon's orb.
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "GunBullet.generated.h"

class UStaticMeshComponent;

/**
 * A bullet that flies straight, damages the first enemy it touches and disappears, or disappears at the end of
 * its range. It moves in short steps so a slow frame cannot make it skip over an enemy.
 */
UCLASS(NotPlaceable)
class GAME_01_API AGunBullet : public AActor
{
	GENERATED_BODY()

public:
	AGunBullet();

	/** Spawns a bullet at Origin flying along Direction (a unit vector). */
	static void Fire(UWorld* World, const FVector& Origin, const FVector& Direction, float Speed, float Range, float Damage, float HitRadius, const FLinearColor& Color);

	virtual void Tick(float DeltaSeconds) override;

private:
	UPROPERTY()
	TObjectPtr<UStaticMeshComponent> Body;

	FVector Velocity = FVector::ZeroVector;
	float DistanceLeft = 0.0f;
	float Damage = 0.0f;
	float HitRadius = 60.0f;
};

/** One glowing ball of the Orbit weapon. The weapon component moves it; it has no logic of its own. */
UCLASS(NotPlaceable)
class GAME_01_API AOrbitOrb : public AActor
{
	GENERATED_BODY()

public:
	AOrbitOrb();

	/** Creates an orb of the given colour and size scale at a location. */
	static AOrbitOrb* Make(UWorld* World, const FVector& Location, const FLinearColor& Color, float Scale);

private:
	UPROPERTY()
	TObjectPtr<UStaticMeshComponent> Body;
};
