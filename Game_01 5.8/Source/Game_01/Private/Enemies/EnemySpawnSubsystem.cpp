#include "Enemies/EnemySpawnSubsystem.h"
#include "Enemies/BossRejectsSubsystem.h"
#include "Enemies/Enemy.h"
#include "Enemies/EnemyManagerSubsystem.h"
#include "Extraction/RunOutcomeSubsystem.h"
#include "Game/PlayAreaSubsystem.h"
#include "Game/DirectorTuningSettings.h"
#include "Loot/LootHeatSubsystem.h"
#include "Player/HeroCharacter.h"
#include "Progression/RunProgressionSubsystem.h"
#include "Animation/AnimSequence.h"
#include "Components/CapsuleComponent.h"
#include "Engine/SkeletalMesh.h"
#include "Engine/DataTable.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"
#include "UObject/ConstructorHelpers.h"

UEnemySpawnSubsystem::UEnemySpawnSubsystem()
{
	// GruntTuning keeps the FEnemyTuning defaults. The Boss is big, slow and hits hard.
	BossTuning.Kind = EEnemyKind::Boss;
	BossTuning.MaxHealth = 1500.0f;
	BossTuning.MoveSpeed = 180.0f;
	BossTuning.ContactDamage = 35.0f;
	BossTuning.ContactRange = 220.0f;
	BossTuning.ContactCooldown = 1.2f;
	BossTuning.Scale = 2.5f;

	// Models from the "Monster for Survival Game" pack (Polyart style, Dungeon Mason, Fab). If the pack is not
	// in Content, every lookup fails and the enemies stay grey cubes.
	const TCHAR* MeshRoot = TEXT("/Game/MonsterForSurvivalGame/Mesh/Polyart/");
	const TCHAR* AnimRoot = TEXT("/Game/MonsterForSurvivalGame/Animation/Polyart/");
	auto Find = [&](const FString& Monster, const FString& AnimFolder, const FString& AnimName, float Yaw, float ModelScale) -> FEnemyVisualAsset
	{
		FEnemyVisualAsset Visual;
		Visual.YawOffset = Yaw;
		Visual.Scale = ModelScale;
		ConstructorHelpers::FObjectFinder<USkeletalMesh> MeshFinder(*FString::Printf(TEXT("%s%s_SK.%s_SK"), MeshRoot, *Monster, *Monster));
		ConstructorHelpers::FObjectFinder<UAnimSequence> AnimFinder(*FString::Printf(TEXT("%s%s/%s.%s"), AnimRoot, *AnimFolder, *AnimName, *AnimName));
		if (MeshFinder.Succeeded())
		{
			Visual.Mesh = MeshFinder.Object;
			Visual.Move = AnimFinder.Succeeded() ? AnimFinder.Object : nullptr;
		}
		return Visual;
	};
	const TArray<FEnemyVisualAsset> Candidates = {
		Find(TEXT("Slime"), TEXT("Slime"), TEXT("Slime_Run_ANIM"), -90.0f, 1.2f),
		Find(TEXT("Mushroom"), TEXT("Mushroom/SmileVersion"), TEXT("Mushroom_runFWDSmile_ANIM"), -90.0f, 1.2f),
		Find(TEXT("Cactus"), TEXT("Cactus"), TEXT("Cactus_RunFWD_ANIM"), -90.0f, 1.2f),
		Find(TEXT("TurtleShell"), TEXT("TurtleShell"), TEXT("TurtleShell_Run_ANIM"), -90.0f, 1.2f),
		Find(TEXT("Beholder"), TEXT("Beholder"), TEXT("Beholder_Run_ANIM"), -90.0f, 1.2f),
	};
	for (const FEnemyVisualAsset& Candidate : Candidates)
	{
		if (Candidate.Mesh)
		{
			GruntVisuals.Add(Candidate);
		}
	}
	BossVisual = Find(TEXT("ChestMonster"), TEXT("ChestMonster"), TEXT("ChestMonster_Run_ANIM"), -90.0f, 0.85f);
}

void UEnemySpawnSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	// Project Settings > Game > Director Tuning. A missing table or row silently keeps the inline defaults above.
	const UDirectorTuningSettings* Settings = GetDefault<UDirectorTuningSettings>();
	const TCHAR* Context = TEXT("UEnemySpawnSubsystem::Initialize");

	if (const UDataTable* EnemyTable = Settings->EnemyTuningTable.LoadSynchronous())
	{
		if (const FEnemyTuning* Row = EnemyTable->FindRow<FEnemyTuning>(TEXT("Grunt"), Context, false))
		{
			GruntTuning = *Row;
			GruntTuning.Kind = EEnemyKind::Grunt;
		}
		if (const FEnemyTuning* Row = EnemyTable->FindRow<FEnemyTuning>(TEXT("Boss"), Context, false))
		{
			BossTuning = *Row;
			// Kind is identity rather than a number, so a forgotten cell cannot turn the boss into a Grunt.
			BossTuning.Kind = EEnemyKind::Boss;
		}
	}

	if (const UDataTable* SpawnTable = Settings->SpawnTuningTable.LoadSynchronous())
	{
		if (const FSpawnTuning* Row = SpawnTable->FindRow<FSpawnTuning>(TEXT("Spawn"), Context, false))
		{
			Tuning = *Row;
		}
	}
}

float UEnemySpawnSubsystem::GetCurrentSpawnRate() const
{
	const ULootHeatSubsystem* Heat = GetWorld()->GetSubsystem<ULootHeatSubsystem>();
	const float Pressure = Heat ? Heat->GetSpawnPressureMultiplier() : 1.0f;
	return (Tuning.BaseRate + Tuning.RampPerMinute * RunTime / 60.0f) * Pressure;
}

void UEnemySpawnSubsystem::ResetRun()
{
	RunTime = 0.0f;
	SpawnAccumulator = 0.0f;
}

void UEnemySpawnSubsystem::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	// Soft-pause while the level-up offer is showing (story 004).
	if (const URunProgressionSubsystem* Progression = GetWorld()->GetSubsystem<URunProgressionSubsystem>())
	{
		if (Progression->IsOfferPending())
		{
			return;
		}
	}

	const URunOutcomeSubsystem* Run = GetWorld()->GetSubsystem<URunOutcomeSubsystem>();
	const AHeroCharacter* Hero = Cast<AHeroCharacter>(UGameplayStatics::GetPlayerPawn(this, 0));
	if (!bSchedulerEnabled || !Hero || !Run || !Run->IsRunActive())
	{
		return;
	}

	RunTime += DeltaTime;
	SpawnAccumulator += GetCurrentSpawnRate() * DeltaTime;

	UEnemyManagerSubsystem* Manager = GetWorld()->GetSubsystem<UEnemyManagerSubsystem>();
	while (SpawnAccumulator >= 1.0f && Manager->GetAliveCount() < Tuning.MaxAlive)
	{
		SpawnOnRing(*Hero, GruntTuning);
		SpawnAccumulator -= 1.0f;
	}
	// At the cap, do not bank spawns that would arrive as a burst once room frees up.
	if (Manager->GetAliveCount() >= Tuning.MaxAlive)
	{
		SpawnAccumulator = FMath::Min(SpawnAccumulator, 1.0f);
	}
}

int32 UEnemySpawnSubsystem::SpawnHorde(int32 Count, bool bIgnoreCap)
{
	const AHeroCharacter* Hero = Cast<AHeroCharacter>(UGameplayStatics::GetPlayerPawn(this, 0));
	if (!Hero)
	{
		return 0;
	}

	const UEnemyManagerSubsystem* Manager = GetWorld()->GetSubsystem<UEnemyManagerSubsystem>();
	int32 Spawned = 0;
	while (Spawned < Count && (bIgnoreCap || Manager->GetAliveCount() < Tuning.MaxAlive))
	{
		SpawnOnRing(*Hero, GruntTuning);
		++Spawned;
	}
	UE_LOG(LogEnemies, Log, TEXT("Horde: spawned %d of %d requested, %d alive."), Spawned, Count, Manager->GetAliveCount());
	return Spawned;
}

AEnemy* UEnemySpawnSubsystem::SpawnBoss()
{
	const AHeroCharacter* Hero = Cast<AHeroCharacter>(UGameplayStatics::GetPlayerPawn(this, 0));
	AEnemy* Boss = Hero ? SpawnOnRing(*Hero, BossTuning) : nullptr;
	if (Boss)
	{
		// Story 012: from now on the player's cuts feed this boss.
		if (UBossRejectsSubsystem* Rejects = GetWorld()->GetSubsystem<UBossRejectsSubsystem>())
		{
			Rejects->SetBoss(Boss);
		}
	}
	return Boss;
}

AEnemy* UEnemySpawnSubsystem::SpawnOnRing(const AHeroCharacter& Hero, const FEnemyTuning& Row)
{
	const double Angle = FMath::FRandRange(0.0, UE_TWO_PI);
	const double Radius = FMath::FRandRange(static_cast<double>(Tuning.RingMinRadius), static_cast<double>(Tuning.RingMaxRadius));
	const FVector HeroLocation = Hero.GetActorLocation();

	// Rest the cube on the floor under the hero: the hero's origin is its capsule centre.
	const double FloorZ = HeroLocation.Z - Hero.GetCapsuleComponent()->GetScaledCapsuleHalfHeight();
	FVector Location(HeroLocation.X + FMath::Cos(Angle) * Radius, HeroLocation.Y + FMath::Sin(Angle) * Radius,
		FloorZ + Row.Scale * AEnemy::EngineCubeSize * 0.5f);
	if (const UPlayAreaSubsystem* PlayArea = GetWorld()->GetSubsystem<UPlayAreaSubsystem>())
	{
		Location = PlayArea->ClampToArea(Location, Row.Scale * AEnemy::EngineCubeSize * 0.5f);
	}

	FActorSpawnParameters Params;
	Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	AEnemy* Enemy = GetWorld()->SpawnActor<AEnemy>(AEnemy::StaticClass(), Location, FRotator::ZeroRotator, Params);
	if (Enemy)
	{
		Enemy->Init(Row);
		const FEnemyVisualAsset* Visual = nullptr;
		if (Row.Kind == EEnemyKind::Boss)
		{
			Visual = BossVisual.Mesh ? &BossVisual : nullptr;
		}
		else if (!GruntVisuals.IsEmpty())
		{
			Visual = &GruntVisuals[FMath::RandRange(0, GruntVisuals.Num() - 1)];
		}
		if (Visual)
		{
			Enemy->SetVisual(Visual->Mesh, Visual->Move, Visual->YawOffset, Visual->Scale);
		}
		GetWorld()->GetSubsystem<UEnemyManagerSubsystem>()->RegisterEnemy(Enemy);
	}
	return Enemy;
}
