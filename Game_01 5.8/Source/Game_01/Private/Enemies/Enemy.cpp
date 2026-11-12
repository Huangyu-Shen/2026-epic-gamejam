#include "Enemies/Enemy.h"
#include "Enemies/EnemyManagerSubsystem.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "Engine/World.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "UObject/ConstructorHelpers.h"

AEnemy::AEnemy()
{
	PrimaryActorTick.bCanEverTick = false;

	Body = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Body"));
	SetRootComponent(Body);
	Body->SetMobility(EComponentMobility::Movable);
	Body->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	Body->SetGenerateOverlapEvents(false);
	Body->SetCastShadow(false);

	static ConstructorHelpers::FObjectFinder<UStaticMesh> CubeFinder(TEXT("/Engine/BasicShapes/Cube.Cube"));
	if (CubeFinder.Succeeded())
	{
		Body->SetStaticMesh(CubeFinder.Object);
	}
}

void AEnemy::Init(const FEnemyTuning& InTuning)
{
	Tuning = InTuning;
	Health = Tuning.MaxHealth;
	Body->SetWorldScale3D(FVector(Tuning.Scale));

	// Greybox readability: enemies must not share the floor's grey. Grunts are red, the Boss is purple.
	static const FLinearColor GruntColor(0.75f, 0.08f, 0.06f);
	static const FLinearColor BossColor(0.45f, 0.05f, 0.7f);
	if (UMaterialInstanceDynamic* Material = Body->CreateDynamicMaterialInstance(0))
	{
		Material->SetVectorParameterValue(TEXT("Color"), Tuning.Kind == EEnemyKind::Boss ? BossColor : GruntColor);
	}
}

void AEnemy::ApplyDamage(float Amount)
{
	if (Health <= 0.0f)
	{
		return;
	}
	Health -= Amount;
	if (Health <= 0.0f)
	{
		if (UEnemyManagerSubsystem* Manager = GetWorld()->GetSubsystem<UEnemyManagerSubsystem>())
		{
			Manager->NotifyEnemyKilled(this);
		}
		Destroy();
	}
}

void AEnemy::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	// Only a real destroy unregisters; world teardown frees the manager along with everything else.
	if (EndPlayReason == EEndPlayReason::Destroyed && GetWorld())
	{
		if (UEnemyManagerSubsystem* Manager = GetWorld()->GetSubsystem<UEnemyManagerSubsystem>())
		{
			Manager->UnregisterEnemy(this);
		}
	}
	Super::EndPlay(EndPlayReason);
}
