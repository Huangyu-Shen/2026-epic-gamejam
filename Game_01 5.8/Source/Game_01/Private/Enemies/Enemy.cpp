#include "Enemies/Enemy.h"
#include "Audio/GameAudioSubsystem.h"
#include "Enemies/EnemyManagerSubsystem.h"
#include "Animation/AnimSequence.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/SkeletalMesh.h"
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
	static ConstructorHelpers::FObjectFinder<UMaterialInterface> FlatFinder(TEXT("/Game/Materials/M_FlatColor.M_FlatColor"));
	if (FlatFinder.Succeeded())
	{
		Body->SetMaterial(0, FlatFinder.Object);
	}

	Visual = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("Visual"));
	Visual->SetupAttachment(Body);
	Visual->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	Visual->SetGenerateOverlapEvents(false);
	Visual->SetCastShadow(false);
	// 150 animated bodies: skip the pose work for any that are off screen, and update far ones less often.
	Visual->VisibilityBasedAnimTickOption = EVisibilityBasedAnimTickOption::OnlyTickPoseWhenRendered;
	Visual->bEnableUpdateRateOptimizations = true;
}

void AEnemy::SetVisual(USkeletalMesh* Mesh, UAnimSequence* MoveAnimation, float YawOffset, float ModelScale)
{
	if (!Mesh)
	{
		return;
	}
	// The cube's origin is its centre; the model's origin is at its feet, so it hangs half a cube lower.
	Body->SetStaticMesh(nullptr);
	Visual->SetSkeletalMesh(Mesh);
	Visual->SetRelativeLocation(FVector(0.0f, 0.0f, -EngineCubeSize * 0.5f));
	Visual->SetRelativeRotation(FRotator(0.0f, YawOffset, 0.0f));
	Visual->SetRelativeScale3D(FVector(ModelScale));
	if (MoveAnimation)
	{
		Visual->SetAnimationMode(EAnimationMode::AnimationSingleNode);
		Visual->SetAnimation(MoveAnimation);
		Visual->Play(true);
		// A random start so a crowd does not step in unison.
		Visual->SetPosition(FMath::FRand() * MoveAnimation->GetPlayLength());
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
	BaseColor = Tuning.Kind == EEnemyKind::Boss ? BossColor : GruntColor;
	BodyMaterial = Body->CreateDynamicMaterialInstance(0);
	if (BodyMaterial)
	{
		BodyMaterial->SetVectorParameterValue(TEXT("Color"), BaseColor);
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
		UGameAudioSubsystem::PlayIn(this, EGameSound::EnemyDeath);
		Destroy();
		return;
	}

	// Cheap hit feedback: one material parameter and one scale change, undone by the manager loop.
	UGameAudioSubsystem::PlayIn(this, EGameSound::EnemyHit);
	HitFlashUntil = GetWorld()->GetTimeSeconds() + 0.1;
	if (!bFlashing)
	{
		bFlashing = true;
		Body->SetWorldScale3D(FVector(Tuning.Scale * 1.2f));
		if (BodyMaterial)
		{
			BodyMaterial->SetVectorParameterValue(TEXT("Color"), FLinearColor::White);
		}
	}
}

void AEnemy::UpdateHitFeedback(double Now)
{
	if (bFlashing && Now >= HitFlashUntil)
	{
		bFlashing = false;
		Body->SetWorldScale3D(FVector(Tuning.Scale));
		if (BodyMaterial)
		{
			BodyMaterial->SetVectorParameterValue(TEXT("Color"), BaseColor);
		}
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
