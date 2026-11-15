#include "Camp/CampGameMode.h"
#include "Audio/GameAudioSubsystem.h"
#include "Camp/CampBuilding.h"
#include "Camp/CampPawn.h"
#include "Camp/CampWarehouse.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "Extraction/CampStockSubsystem.h"
#include "Misc/CommandLine.h"
#include "Misc/Parse.h"
#include "Misc/Paths.h"
#include "TimerManager.h"
#include "UnrealClient.h"

namespace
{
	/** Distance (cm) from the camp's centre to each building; the three form an equilateral triangle. */
	constexpr float CampTriangleRadius = 700.0f;
}

ACampGameMode::ACampGameMode()
{
	DefaultPawnClass = ACampPawn::StaticClass();
}

void ACampGameMode::BeginPlay()
{
	Super::BeginPlay();

	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	if (UGameAudioSubsystem* Audio = World->GetSubsystem<UGameAudioSubsystem>())
	{
		Audio->StartMusic(EMusicTrack::Camp);
	}

	// Spawn the two buildings and the warehouse if the level does not already contain them
	// (lets a hand-placed camp override the defaults without duplicates).
	bool bHasBuildings = false;
	for (TActorIterator<ACampBuilding> It(World); It; ++It)
	{
		bHasBuildings = true;
		break;
	}
	if (!bHasBuildings)
	{
		auto SpawnBuilding = [&](EBuildingType Type, const FVector& Location, const FRotator& Rotation)
		{
			// Deferred spawn: BuildingType must be set BEFORE BeginPlay, which reads it to sync level/tint.
			const FTransform Transform(Rotation, Location);
			if (ACampBuilding* Building = World->SpawnActorDeferred<ACampBuilding>(
					ACampBuilding::StaticClass(), Transform, nullptr, nullptr,
					ESpawnActorCollisionHandlingMethod::AlwaysSpawn))
			{
				Building->BuildingType = Type;
				Building->FinishSpawning(Transform);
			}
		};
		// The three buildings stand on an equilateral triangle (radius 700 cm, centred on the origin): the clinic at the top of the
		// screen with its porch toward the camera, the smithy at the lower left and the warehouse at the lower right. The smithy and
		// the warehouse are turned a quarter turn clockwise.
		SpawnBuilding(EBuildingType::Clinic, FVector(CampTriangleRadius, 0.0f, 60.0f), FRotator(0.0f, 180.0f, 0.0f));
		SpawnBuilding(EBuildingType::Smithy, FVector(-0.5f * CampTriangleRadius, -0.8660254f * CampTriangleRadius, 60.0f), FRotator(0.0f, 90.0f, 0.0f));
	}

	bool bHasWarehouse = false;
	for (TActorIterator<ACampWarehouse> It(World); It; ++It)
	{
		bHasWarehouse = true;
		break;
	}
	if (!bHasWarehouse)
	{
		FActorSpawnParameters Params;
		Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
		World->SpawnActor<ACampWarehouse>(ACampWarehouse::StaticClass(), FVector(-0.5f * CampTriangleRadius, 0.8660254f * CampTriangleRadius, 60.0f), FRotator(0.0f, 90.0f, 0.0f), Params);
	}

	if (FParse::Param(FCommandLine::Get(), TEXT("CampShot")))
	{
		GetWorldTimerManager().SetTimer(CampShotHandle, this, &ACampGameMode::TakeCampShot, 4.0f);
	}
}

void ACampGameMode::TakeCampShot()
{
	FString Unlocked;
	int32 Iron = 0;
	if (const UCampStockSubsystem* Stock = GetGameInstance()->GetSubsystem<UCampStockSubsystem>())
	{
		Iron = Stock->GetIron();
		for (const FName Option : Stock->GetUnlockedOptions())
		{
			Unlocked += Option.ToString() + TEXT(" ");
		}
	}
	UE_LOG(LogTemp, Display, TEXT("CAMPSHOT iron=%d unlocked=[%s]"), Iron, *Unlocked);
	FScreenshotRequest::RequestScreenshot(FPaths::ProjectSavedDir() / TEXT("ReelProbe") / TEXT("camp-storage.png"), true, false);
	FTimerHandle Exit;
	GetWorldTimerManager().SetTimer(Exit, []() { FPlatformMisc::RequestExit(false); }, 1.5f, false);
}
