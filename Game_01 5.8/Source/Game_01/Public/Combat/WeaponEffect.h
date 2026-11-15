// Director's Cut - Story 011. A short-lived visual for one weapon attack; replaces the DrawDebug lines.
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "WeaponEffect.generated.h"

class UStaticMeshComponent;
class UStaticMesh;
class UMaterialInterface;

/**
 * A beam (Blade, Bolt, Chain) or a ring (Nova) built from engine cylinders and cubes. It lives about a tenth of a
 * second, shrinks as it fades and destroys itself. Unlike DrawDebug it is drawn in a Shipping build too.
 * Thickness is passed in already multiplied by the weapon's star reach, so a star-up makes the attack look bigger.
 */
UCLASS(NotPlaceable)
class GAME_01_API AWeaponEffect : public AActor
{
	GENERATED_BODY()

public:
	AWeaponEffect();

	/** A line between two points. Thickness and the colour are in cm and linear colour. */
	static void SpawnBeam(UWorld* World, const FVector& From, const FVector& To, const FLinearColor& Color, float Thickness, float Lifetime);

	/** A ring on the floor plane around a centre, made of a fixed number of straight pieces. */
	static void SpawnRing(UWorld* World, const FVector& Center, float Radius, const FLinearColor& Color, float Thickness, float Lifetime);

	virtual void Tick(float DeltaSeconds) override;

private:
	struct FPiece
	{
		UStaticMeshComponent* Component = nullptr;
		FVector FullScale = FVector::OneVector;
	};

	UStaticMeshComponent* AddPiece(UStaticMesh* Mesh, const FVector& Location, const FRotator& Rotation, const FVector& Scale, const FLinearColor& Color);

	UPROPERTY()
	TObjectPtr<UStaticMesh> CylinderMesh;

	UPROPERTY()
	TObjectPtr<UStaticMesh> CubeMesh;

	/** Unlit flat-colour material (Content/Materials/M_FlatColor) with a Color parameter; the engine cube default has none. */
	UPROPERTY()
	TObjectPtr<UMaterialInterface> FlatMaterial;

	TArray<FPiece> Pieces;
	float Age = 0.0f;
	float Life = 0.12f;
};
