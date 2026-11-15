#include "Combat/WeaponEffect.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "Engine/World.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "UObject/ConstructorHelpers.h"

namespace
{
	/** Straight pieces in a Nova ring. Sixteen reads as a circle at the sizes the weapon reaches. */
	constexpr int32 RingPieces = 16;
	constexpr float EngineShapeSize = 100.0f;
}

AWeaponEffect::AWeaponEffect()
{
	PrimaryActorTick.bCanEverTick = true;
	SetRootComponent(CreateDefaultSubobject<USceneComponent>(TEXT("Root")));

	static ConstructorHelpers::FObjectFinder<UStaticMesh> CylinderFinder(TEXT("/Engine/BasicShapes/Cylinder.Cylinder"));
	static ConstructorHelpers::FObjectFinder<UStaticMesh> CubeFinder(TEXT("/Engine/BasicShapes/Cube.Cube"));
	static ConstructorHelpers::FObjectFinder<UMaterialInterface> FlatFinder(TEXT("/Game/Materials/M_FlatColor.M_FlatColor"));
	CylinderMesh = CylinderFinder.Succeeded() ? CylinderFinder.Object : nullptr;
	CubeMesh = CubeFinder.Succeeded() ? CubeFinder.Object : nullptr;
	FlatMaterial = FlatFinder.Succeeded() ? FlatFinder.Object : nullptr;
}

UStaticMeshComponent* AWeaponEffect::AddPiece(UStaticMesh* Mesh, const FVector& Location, const FRotator& Rotation, const FVector& Scale, const FLinearColor& Color)
{
	UStaticMeshComponent* Piece = NewObject<UStaticMeshComponent>(this);
	Piece->SetStaticMesh(Mesh);
	Piece->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	Piece->SetGenerateOverlapEvents(false);
	Piece->SetCastShadow(false);
	Piece->SetMobility(EComponentMobility::Movable);
	Piece->SetupAttachment(GetRootComponent());
	Piece->RegisterComponent();
	Piece->SetWorldLocationAndRotation(Location, Rotation);
	Piece->SetWorldScale3D(Scale);
	if (FlatMaterial)
	{
		Piece->SetMaterial(0, FlatMaterial);
	}
	if (UMaterialInstanceDynamic* Material = Piece->CreateDynamicMaterialInstance(0))
	{
		Material->SetVectorParameterValue(TEXT("Color"), Color);
	}
	Pieces.Add({Piece, Scale});
	return Piece;
}

void AWeaponEffect::SpawnBeam(UWorld* World, const FVector& From, const FVector& To, const FLinearColor& Color, float Thickness, float Lifetime)
{
	const FVector Delta = To - From;
	const float Length = Delta.Size();
	if (!World || Length < 1.0f)
	{
		return;
	}
	AWeaponEffect* Effect = World->SpawnActor<AWeaponEffect>(AWeaponEffect::StaticClass(), FTransform(From));
	if (!Effect)
	{
		return;
	}
	Effect->Life = Lifetime;
	// The engine cylinder stands along Z, so point Z along the beam.
	const FRotator Rotation = FRotationMatrix::MakeFromZ(Delta).Rotator();
	Effect->AddPiece(Effect->CylinderMesh, From + Delta * 0.5f, Rotation, FVector(Thickness / EngineShapeSize, Thickness / EngineShapeSize, Length / EngineShapeSize), Color);
}

void AWeaponEffect::SpawnRing(UWorld* World, const FVector& Center, float Radius, const FLinearColor& Color, float Thickness, float Lifetime)
{
	if (!World || Radius < 1.0f)
	{
		return;
	}
	AWeaponEffect* Effect = World->SpawnActor<AWeaponEffect>(AWeaponEffect::StaticClass(), FTransform(Center));
	if (!Effect)
	{
		return;
	}
	Effect->Life = Lifetime;
	const float Step = UE_TWO_PI / RingPieces;
	const float PieceLength = 2.0f * Radius * FMath::Tan(Step * 0.5f);
	for (int32 i = 0; i < RingPieces; ++i)
	{
		const float Angle = Step * i;
		const FVector Location = Center + FVector(FMath::Cos(Angle), FMath::Sin(Angle), 0.0f) * Radius;
		const FRotator Rotation(0.0f, FMath::RadiansToDegrees(Angle), 0.0f); // tangent to the circle
		Effect->AddPiece(Effect->CubeMesh, Location, Rotation, FVector(Thickness / EngineShapeSize, PieceLength / EngineShapeSize * 1.05f, Thickness / EngineShapeSize), Color);
	}
}

void AWeaponEffect::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
	Age += DeltaSeconds;
	if (Age >= Life)
	{
		Destroy();
		return;
	}
	// Thin out over the lifetime instead of blinking off; length stays so the attack still reads.
	const float Fade = 1.0f - Age / Life;
	for (const FPiece& Piece : Pieces)
	{
		if (Piece.Component)
		{
			Piece.Component->SetWorldScale3D(FVector(Piece.FullScale.X * Fade, Piece.FullScale.Y * (Piece.FullScale.Y > Piece.FullScale.X ? 1.0f : Fade), Piece.FullScale.Z * (Piece.FullScale.Z > Piece.FullScale.X ? 1.0f : Fade)));
		}
	}
}
