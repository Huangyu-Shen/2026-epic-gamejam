#include "Game/DirectorGameMode.h"
#include "Combat/AutoWeaponComponent.h"
#include "Combat/HealthComponent.h"
#include "Enemies/EnemyManagerSubsystem.h"
#include "Enemies/EnemySpawnSubsystem.h"
#include "Enemies/EnemyTypes.h"
#include "Player/HeroCharacter.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"
#include "Misc/App.h"
#include "Misc/CommandLine.h"
#include "Misc/Parse.h"
#include "TimerManager.h"

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
