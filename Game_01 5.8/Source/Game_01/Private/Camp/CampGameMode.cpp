#include "Camp/CampGameMode.h"
#include "Camp/CampBuilding.h"
#include "Camp/CampPawn.h"
#include "Camp/CampWarehouse.h"
#include "Engine/World.h"
#include "EngineUtils.h"

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
		auto SpawnBuilding = [&](EBuildingType Type, const FVector& Location)
		{
			// Deferred spawn: BuildingType must be set BEFORE BeginPlay, which reads it to sync level/tint.
			const FTransform Transform(FRotator::ZeroRotator, Location);
			if (ACampBuilding* Building = World->SpawnActorDeferred<ACampBuilding>(
					ACampBuilding::StaticClass(), Transform, nullptr, nullptr,
					ESpawnActorCollisionHandlingMethod::AlwaysSpawn))
			{
				Building->BuildingType = Type;
				Building->FinishSpawning(Transform);
			}
		};
		SpawnBuilding(EBuildingType::Smithy, FVector(-350.0f, 0.0f, 60.0f));
		SpawnBuilding(EBuildingType::Clinic, FVector(350.0f, 0.0f, 60.0f));
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
		World->SpawnActor<ACampWarehouse>(ACampWarehouse::StaticClass(), FVector(0.0f, -350.0f, 60.0f), FRotator::ZeroRotator, Params);
	}
}
