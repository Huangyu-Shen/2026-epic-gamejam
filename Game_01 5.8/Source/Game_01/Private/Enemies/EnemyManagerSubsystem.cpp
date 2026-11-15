#include "Enemies/EnemyManagerSubsystem.h"
#include "Enemies/Enemy.h"
#include "Extraction/RunOutcomeSubsystem.h"
#include "Game/DirectorTuningSettings.h"
#include "Player/HeroCharacter.h"
#include "Progression/RunProgressionSubsystem.h"
#include "Engine/DataTable.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"

void UEnemyManagerSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	// Project Settings > Game > Director Tuning. A missing table or row silently keeps the inline defaults.
	const UDirectorTuningSettings* Settings = GetDefault<UDirectorTuningSettings>();
	if (const UDataTable* SwarmTable = Settings->SwarmTuningTable.LoadSynchronous())
	{
		if (const FSwarmTuning* Row = SwarmTable->FindRow<FSwarmTuning>(TEXT("Swarm"), TEXT("UEnemyManagerSubsystem::Initialize"), false))
		{
			Tuning = *Row;
		}
	}
}

void UEnemyManagerSubsystem::RegisterEnemy(AEnemy* Enemy)
{
	Enemies.Add(Enemy);
}

void UEnemyManagerSubsystem::UnregisterEnemy(AEnemy* Enemy)
{
	Enemies.RemoveSingleSwap(Enemy, EAllowShrinking::No);
}

void UEnemyManagerSubsystem::NotifyEnemyKilled(AEnemy* Enemy)
{
	++KillCount;
	OnEnemyKilled.Broadcast(Enemy->Tuning.Kind, Enemy->GetActorLocation());
}

AEnemy* UEnemyManagerSubsystem::FindNearestEnemy(FVector Location, float Range) const
{
	AEnemy* Nearest = nullptr;
	double NearestDistSq = FMath::Square(static_cast<double>(Range));
	for (AEnemy* Enemy : Enemies)
	{
		const double DistSq = FVector::DistSquared2D(Enemy->GetActorLocation(), Location);
		if (DistSq <= NearestDistSq)
		{
			NearestDistSq = DistSq;
			Nearest = Enemy;
		}
	}
	return Nearest;
}

void UEnemyManagerSubsystem::ClearAll()
{
	// Destroy() unregisters each enemy, so work on a detached copy.
	TArray<TObjectPtr<AEnemy>> Doomed = MoveTemp(Enemies);
	Enemies.Reset();
	for (AEnemy* Enemy : Doomed)
	{
		if (IsValid(Enemy))
		{
			Enemy->Destroy();
		}
	}
}

void UEnemyManagerSubsystem::Tick(float DeltaTime)
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

	AHeroCharacter* Hero = Cast<AHeroCharacter>(UGameplayStatics::GetPlayerPawn(this, 0));
	if (!Hero || Enemies.IsEmpty())
	{
		return;
	}

	const URunOutcomeSubsystem* Run = GetWorld()->GetSubsystem<URunOutcomeSubsystem>();
	const bool bCanHurt = Run && Run->IsRunActive();
	const FVector HeroLocation = Hero->GetActorLocation();
	const double Now = GetWorld()->GetTimeSeconds();
	const int32 Num = Enemies.Num();

	Positions.SetNumUninitialized(Num);
	Yaws.SetNumUninitialized(Num);
	Pushes.SetNumZeroed(Num);

	// Pass 1: chase the hero, face the movement, apply contact damage.
	for (int32 Index = 0; Index < Num; ++Index)
	{
		AEnemy* Enemy = Enemies[Index];
		const FEnemyTuning& Row = Enemy->Tuning;
		FVector Position = Enemy->GetActorLocation();
		Yaws[Index] = Enemy->GetActorRotation().Yaw;

		const FVector2D ToHero = FVector2D(HeroLocation) - FVector2D(Position);
		const double Dist = ToHero.Size();
		if (Dist > UE_KINDA_SMALL_NUMBER)
		{
			const FVector2D Dir = ToHero / Dist;
			const double Step = FMath::Clamp(Dist - Row.ContactRange * Tuning.StopRangeFraction, 0.0, static_cast<double>(Row.MoveSpeed) * DeltaTime);
			Position.X += Dir.X * Step;
			Position.Y += Dir.Y * Step;
			Yaws[Index] = FMath::RadiansToDegrees(FMath::Atan2(Dir.Y, Dir.X));
		}
		Positions[Index] = Position;

		if (bCanHurt && Dist <= Row.ContactRange && Now >= Enemy->NextContactTime)
		{
			Hero->ReceiveEnemyDamage(Row.ContactDamage);
			Enemy->NextContactTime = Now + Row.ContactCooldown;
		}
	}

	// Pass 2: bucket enemies into a uniform grid.
	Grid.Reset();
	for (int32 Index = 0; Index < Num; ++Index)
	{
		const FIntPoint Cell(FMath::FloorToInt(Positions[Index].X / Tuning.SeparationCellSize), FMath::FloorToInt(Positions[Index].Y / Tuning.SeparationCellSize));
		Grid.FindOrAdd(Cell).Add(Index);
	}

	// Pass 3: push overlapping bodies apart (each enemy only looks at the 3x3 cells around it).
	for (int32 Index = 0; Index < Num; ++Index)
	{
		const FIntPoint Cell(FMath::FloorToInt(Positions[Index].X / Tuning.SeparationCellSize), FMath::FloorToInt(Positions[Index].Y / Tuning.SeparationCellSize));
		const float RadiusA = Enemies[Index]->GetBodyRadius();
		for (int32 DX = -1; DX <= 1; ++DX)
		{
			for (int32 DY = -1; DY <= 1; ++DY)
			{
				const TArray<int32>* Neighbours = Grid.Find(FIntPoint(Cell.X + DX, Cell.Y + DY));
				if (!Neighbours)
				{
					continue;
				}
				for (const int32 Other : *Neighbours)
				{
					if (Other == Index)
					{
						continue;
					}
					const FVector2D Offset = FVector2D(Positions[Index]) - FVector2D(Positions[Other]);
					const double Overlap = (RadiusA + Enemies[Other]->GetBodyRadius()) - Offset.Size();
					if (Overlap <= 0.0)
					{
						continue;
					}
					// Exactly coincident bodies get a stable pseudo-random direction from their indices.
					const FVector2D Dir = Offset.SizeSquared() > UE_KINDA_SMALL_NUMBER
						? Offset.GetSafeNormal()
						: FVector2D(FMath::Cos(static_cast<double>(Index)), FMath::Sin(static_cast<double>(Index)));
					Pushes[Index] += Dir * (Overlap * Tuning.SeparationStrength);
				}
			}
		}
	}

	// Apply.
	for (int32 Index = 0; Index < Num; ++Index)
	{
		FVector2D Push = Pushes[Index];
		if (Push.SizeSquared() > FMath::Square(static_cast<double>(Tuning.MaxSeparationSpeed)))
		{
			Push = Push.GetSafeNormal() * Tuning.MaxSeparationSpeed;
		}
		FVector NewLocation = Positions[Index];
		NewLocation.X += Push.X * DeltaTime;
		NewLocation.Y += Push.Y * DeltaTime;
		Enemies[Index]->SetActorLocationAndRotation(NewLocation, FRotator(0.0, Yaws[Index], 0.0));
		Enemies[Index]->UpdateHitFeedback(Now);
	}
}
