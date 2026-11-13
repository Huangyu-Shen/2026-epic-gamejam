#include "Game/DirectorGameMode.h"
#include "Combat/AutoWeaponComponent.h"
#include "Combat/HealthComponent.h"
#include "Enemies/EnemyManagerSubsystem.h"
#include "Enemies/EnemySpawnSubsystem.h"
#include "Enemies/EnemyTypes.h"
#include "Player/HeroCharacter.h"
#include "Progression/RunProgressionSubsystem.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"
#include "Misc/App.h"
#include "Misc/CommandLine.h"
#include "Misc/Parse.h"
#include "TimerManager.h"
#include "UnrealClient.h"

ADirectorGameMode::ADirectorGameMode()
{
	DefaultPawnClass = AHeroCharacter::StaticClass();

	// Ticks only while the probe samples.
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.bStartWithTickEnabled = false;
}

void ADirectorGameMode::BeginPlay()
{
	Super::BeginPlay();

	if (FParse::Param(FCommandLine::Get(), TEXT("PerfProbe")))
	{
		GetWorldTimerManager().SetTimer(ProbeStartHandle, this, &ADirectorGameMode::StartPerfProbe, PerfProbe.StartDelaySeconds);
	}

	if (FParse::Param(FCommandLine::Get(), TEXT("OfferProbe")))
	{
		GetWorldTimerManager().SetTimer(OfferProbeHandle, this, &ADirectorGameMode::StepOfferProbe, OfferProbe.StepSeconds, true, OfferProbe.StartDelaySeconds);
	}
}

void ADirectorGameMode::StepOfferProbe()
{
	AHeroCharacter* Hero = Cast<AHeroCharacter>(UGameplayStatics::GetPlayerPawn(this, 0));
	URunProgressionSubsystem* Progression = GetWorld()->GetSubsystem<URunProgressionSubsystem>();
	if (!Hero || !Progression)
	{
		UE_LOG(LogEnemies, Error, TEXT("OFFERPROBE aborted: no hero pawn or progression subsystem."));
		FPlatformMisc::RequestExit(false);
		return;
	}

	auto Shot = [](const TCHAR* Name)
	{
		FScreenshotRequest::RequestScreenshot(FPaths::ProjectSavedDir() / TEXT("OfferProbe") / Name, true, false);
	};
	auto LogState = [Progression](const TCHAR* Label)
	{
		UE_LOG(LogEnemies, Display, TEXT("OFFERPROBE %s level=%d cutsLeft=%d cutSoFar=%d loadout=[%s]"), Label,
			Progression->GetLevel(), Progression->GetPermanentCutsLeft(), Progression->GetCuttingRoomFloor().Num(), *Progression->GetLoadoutSummary());
		for (const FString& Line : Progression->GetOfferDescriptions())
		{
			UE_LOG(LogEnemies, Display, TEXT("OFFERPROBE   option: %s"), *Line);
		}
	};

	switch (OfferProbeStep++)
	{
	case 0:
		// A calm screen: the hero cannot die and nothing new spawns. Then force the first level-up.
		Hero->Health->bGodMode = true;
		GetWorld()->GetSubsystem<UEnemySpawnSubsystem>()->SetSchedulerEnabled(false);
		Progression->GrantXp(Progression->GetXpToNext());
		LogState(TEXT("first-offer"));
		break;
	case 1:
		Shot(TEXT("1-first-offer.png"));
		break;
	case 2:
		// Play several level-ups, always cutting the first option and keeping the other two.
		for (int32 i = 0; i < OfferProbe.LevelUps; ++i)
		{
			if (Progression->IsOfferPending())
			{
				Progression->CutOffer(0);
			}
			Progression->GrantXp(Progression->GetXpToNext());
		}
		if (Progression->IsOfferPending())
		{
			Progression->CutOffer(0);
		}
		LogState(TEXT("after-level-ups"));
		break;
	case 3:
		Shot(TEXT("2-loadout-after-cuts.png"));
		break;
	case 4:
		Progression->GrantXp(Progression->GetXpToNext());
		LogState(TEXT("late-offer"));
		break;
	case 5:
		Shot(TEXT("3-late-offer.png"));
		break;
	default:
		GetWorldTimerManager().ClearTimer(OfferProbeHandle);
		FPlatformMisc::RequestExit(false);
		break;
	}
}

void ADirectorGameMode::StartPerfProbe()
{
	AHeroCharacter* Hero = Cast<AHeroCharacter>(UGameplayStatics::GetPlayerPawn(this, 0));
	if (!Hero)
	{
		UE_LOG(LogEnemies, Error, TEXT("PERFPROBE aborted: no hero pawn."));
		FPlatformMisc::RequestExit(false);
		return;
	}

	// God mode keeps the hero alive; weapon and scheduler off keep exactly EnemyCount enemies alive.
	Hero->Health->bGodMode = true;
	Hero->Weapon->bEnabled = false;
	UEnemySpawnSubsystem* Spawner = GetWorld()->GetSubsystem<UEnemySpawnSubsystem>();
	Spawner->SetSchedulerEnabled(false);
	Spawner->SpawnHorde(PerfProbe.EnemyCount, true);

	SetActorTickEnabled(true);
}

void ADirectorGameMode::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	const float FrameSeconds = FApp::GetDeltaTime();
	SampledSeconds += FrameSeconds;
	WorstFrameSeconds = FMath::Max(WorstFrameSeconds, FrameSeconds);
	++SampleCount;

	if (SampledSeconds >= PerfProbe.SampleSeconds)
	{
		const int32 Alive = GetWorld()->GetSubsystem<UEnemyManagerSubsystem>()->GetAliveCount();
		const float AvgFps = SampleCount / SampledSeconds;
		const float MinFps = 1.0f / FMath::Max(WorstFrameSeconds, UE_SMALL_NUMBER);
		UE_LOG(LogEnemies, Display, TEXT("PERFPROBE enemies=%d avgFPS=%.1f minFPS=%.1f samples=%d"), Alive, AvgFps, MinFps, SampleCount);
		SetActorTickEnabled(false);
		FPlatformMisc::RequestExit(false);
	}
}
