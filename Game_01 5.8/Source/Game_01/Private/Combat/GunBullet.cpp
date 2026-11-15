#include "Combat/GunBullet.h"
#include "Components/StaticMeshComponent.h"
#include "Enemies/Enemy.h"
#include "Enemies/EnemyManagerSubsystem.h"
#include "Engine/StaticMesh.h"
#include "Engine/World.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Progression/RunProgressionSubsystem.h"
#include "UObject/ConstructorHelpers.h"

namespace
{
	/** The most a bullet moves between two enemy checks (cm). */
	constexpr float MaxStepCm = 40.0f;

	void StyleBody(UStaticMeshComponent* Body, const FLinearColor& Color, float Scale)
	{
		Body->SetWorldScale3D(FVector(Scale));
		if (UMaterialInstanceDynamic* Material = Body->CreateDynamicMaterialInstance(0))
		{
			Material->SetVectorParameterValue(TEXT("Color"), Color);
		}
	}
}

AGunBullet::AGunBullet()
{
	PrimaryActorTick.bCanEverTick = true;
	Body = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Body"));
	SetRootComponent(Body);
	Body->SetMobility(EComponentMobility::Movable);
	Body->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	Body->SetGenerateOverlapEvents(false);
	Body->SetCastShadow(false);
	static ConstructorHelpers::FObjectFinder<UStaticMesh> SphereFinder(TEXT("/Engine/BasicShapes/Sphere.Sphere"));
	static ConstructorHelpers::FObjectFinder<UMaterialInterface> FlatFinder(TEXT("/Game/Materials/M_FlatColor.M_FlatColor"));
	if (SphereFinder.Succeeded())
	{
		Body->SetStaticMesh(SphereFinder.Object);
	}
	if (FlatFinder.Succeeded())
	{
		Body->SetMaterial(0, FlatFinder.Object);
	}
}

void AGunBullet::Fire(UWorld* World, const FVector& Origin, const FVector& Direction, float Speed, float Range, float InDamage, float InHitRadius, const FLinearColor& Color)
{
	if (!World)
	{
		return;
	}
	FActorSpawnParameters Params;
	Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	AGunBullet* Bullet = World->SpawnActor<AGunBullet>(AGunBullet::StaticClass(), Origin, FRotator::ZeroRotator, Params);
	if (!Bullet)
	{
		return;
	}
	Bullet->Velocity = Direction.GetSafeNormal() * Speed;
	Bullet->DistanceLeft = Range;
	Bullet->Damage = InDamage;
	Bullet->HitRadius = InHitRadius;
	StyleBody(Bullet->Body, Color, 0.3f);
}

void AGunBullet::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	// The level-up offer soft-pauses the game: bullets wait too.
	if (const URunProgressionSubsystem* Progression = GetWorld()->GetSubsystem<URunProgressionSubsystem>())
	{
		if (Progression->IsOfferPending())
		{
			return;
		}
	}

	UEnemyManagerSubsystem* Manager = GetWorld()->GetSubsystem<UEnemyManagerSubsystem>();
	const float Speed = Velocity.Size();
	if (!Manager || Speed < 1.0f)
	{
		Destroy();
		return;
	}

	float Remaining = Speed * DeltaSeconds;
	while (Remaining > 0.0f)
	{
		const float Step = FMath::Min(Remaining, MaxStepCm);
		Remaining -= Step;
		SetActorLocation(GetActorLocation() + Velocity / Speed * Step);
		DistanceLeft -= Step;

		if (AEnemy* Target = Manager->FindNearestEnemy(GetActorLocation(), HitRadius))
		{
			Target->ApplyDamage(Damage);
			Destroy();
			return;
		}
		if (DistanceLeft <= 0.0f)
		{
			Destroy();
			return;
		}
	}
}

AOrbitOrb::AOrbitOrb()
{
	PrimaryActorTick.bCanEverTick = false;
	Body = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Body"));
	SetRootComponent(Body);
	Body->SetMobility(EComponentMobility::Movable);
	Body->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	Body->SetGenerateOverlapEvents(false);
	Body->SetCastShadow(false);
	static ConstructorHelpers::FObjectFinder<UStaticMesh> SphereFinder(TEXT("/Engine/BasicShapes/Sphere.Sphere"));
	static ConstructorHelpers::FObjectFinder<UMaterialInterface> FlatFinder(TEXT("/Game/Materials/M_FlatColor.M_FlatColor"));
	if (SphereFinder.Succeeded())
	{
		Body->SetStaticMesh(SphereFinder.Object);
	}
	if (FlatFinder.Succeeded())
	{
		Body->SetMaterial(0, FlatFinder.Object);
	}
}

AOrbitOrb* AOrbitOrb::Make(UWorld* World, const FVector& Location, const FLinearColor& Color, float Scale)
{
	if (!World)
	{
		return nullptr;
	}
	FActorSpawnParameters Params;
	Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	AOrbitOrb* Orb = World->SpawnActor<AOrbitOrb>(AOrbitOrb::StaticClass(), Location, FRotator::ZeroRotator, Params);
	if (Orb)
	{
		StyleBody(Orb->Body, Color, Scale);
	}
	return Orb;
}
