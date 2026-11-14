#include "Game/DirectorGameMode.h"
#include "Combat/AutoWeaponComponent.h"
#include "Combat/HealthComponent.h"
#include "Extraction/CampStockSubsystem.h"
#include "Extraction/RunOutcomeSubsystem.h"
#include "Loot/LootBagComponent.h"
#include "Loot/LootConfig.h"
#include "Loot/LootHeatSubsystem.h"
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

	FString ReelProbeMode;
	if (FParse::Param(FCommandLine::Get(), TEXT("ReelProbe")) || FParse::Value(FCommandLine::Get(), TEXT("ReelProbe="), ReelProbeMode))
	{
		bReelProbeDies = ReelProbeMode.Equals(TEXT("die"), ESearchCase::IgnoreCase);
		GetWorldTimerManager().SetTimer(ReelProbeHandle, this, &ADirectorGameMode::StepReelProbe, OfferProbe.StepSeconds, true, OfferProbe.StartDelaySeconds);
	}
}

void ADirectorGameMode::StepReelProbe()
{
	AHeroCharacter* Hero = Cast<AHeroCharacter>(UGameplayStatics::GetPlayerPawn(this, 0));
	URunOutcomeSubsystem* Outcome = GetWorld()->GetSubsystem<URunOutcomeSubsystem>();
	UCampStockSubsystem* Stock = GetWorld()->GetGameInstance()->GetSubsystem<UCampStockSubsystem>();
	ULootHeatSubsystem* Heat = GetWorld()->GetSubsystem<ULootHeatSubsystem>();
	if (!Hero || !Outcome || !Stock || !Heat)
	{
		UE_LOG(LogEnemies, Error, TEXT("REELPROBE aborted: missing hero, outcome, stock or heat subsystem."));
		FPlatformMisc::RequestExit(false);
		return;
	}

	auto Shot = [](const TCHAR* Name)
	{
		FScreenshotRequest::RequestScreenshot(FPaths::ProjectSavedDir() / TEXT("ReelProbe") / Name, true, false);
	};
	auto LogState = [&](const TCHAR* Label)
	{
		FString Unlocked;
		for (const FName Option : Stock->GetUnlockedOptions())
		{
			Unlocked += Option.ToString() + TEXT(" ");
		}
		ERelicType Dummy;
		const bool bAnotherReelDrops = Hero->Bag->GetConfig()->PickRandomRelic(Stock->GetUnlockedOptions(), Hero->Bag->GetRelics(), Dummy);
		UE_LOG(LogEnemies, Display, TEXT("REELPROBE %s reelsInBag=%d heat=%.1f unlocked=[%s] anotherReelCanDrop=%d outcome=%d"), Label,
			Hero->Bag->GetRelicCount(), Heat->GetHeat(), *Unlocked, bAnotherReelDrops ? 1 : 0, static_cast<int32>(Outcome->GetOutcome()));
	};

	switch (ReelProbeStep++)
	{
	case 0:
		// A reel and some Iron in the bag; picking the reel up raises heat. Nothing else spawns, the hero cannot die by accident.
		Hero->Health->bGodMode = true;
		GetWorld()->GetSubsystem<UEnemySpawnSubsystem>()->SetSchedulerEnabled(false);
		Hero->Bag->TryAddIron(25);
		Hero->Bag->TryAddRelic(ERelicType::Swiftstep);
		LogState(TEXT("reel-in-bag"));
		break;
	case 1:
		Shot(TEXT("1-reel-in-bag.png"));
		break;
	case 2:
		if (bReelProbeDies)
		{
			Outcome->ReportPlayerDied(Hero->Bag);
		}
		else
		{
			Outcome->ReportExtracted(Hero->Bag);
		}
		LogState(bReelProbeDies ? TEXT("after-death") : TEXT("after-extraction"));
		break;
	case 3:
		Shot(bReelProbeDies ? TEXT("3-result-died.png") : TEXT("2-result-extracted.png"));
		break;
	default:
		GetWorldTimerManager().ClearTimer(ReelProbeHandle);
		FPlatformMisc::RequestExit(false);
		break;
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
