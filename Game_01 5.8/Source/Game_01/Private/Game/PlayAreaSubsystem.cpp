#include "Game/PlayAreaSubsystem.h"
#include "Enemies/EnemyTypes.h"
#include "Engine/StaticMeshActor.h"
#include "Engine/World.h"
#include "EngineUtils.h"

void UPlayAreaSubsystem::OnWorldBeginPlay(UWorld& InWorld)
{
	Super::OnWorldBeginPlay(InWorld);

	// A floor piece is wide in both directions and thin. Walls, props and the greybox cubes are not.
	constexpr float MinHalfSize = 1000.0f;
	constexpr float MaxThinness = 0.2f;

	Area = FBox2D(ForceInit);
	bHasArea = false;
	for (TActorIterator<AStaticMeshActor> It(&InWorld); It; ++It)
	{
		FVector Origin, Extent;
		It->GetActorBounds(false, Origin, Extent);
		const float Horizontal = FMath::Min(Extent.X, Extent.Y);
		const bool bFloor = Horizontal >= MinHalfSize && Extent.Z <= Horizontal * MaxThinness;
		UE_LOG(LogEnemies, Display, TEXT("PLAYAREA candidate %s origin=%s extent=%s floor=%d"), *It->GetName(), *Origin.ToString(), *Extent.ToString(), bFloor);
		if (bFloor)
		{
			Area += FBox2D(FVector2D(Origin.X - Extent.X, Origin.Y - Extent.Y), FVector2D(Origin.X + Extent.X, Origin.Y + Extent.Y));
			bHasArea = true;
		}
	}
	if (bHasArea)
	{
		UE_LOG(LogEnemies, Display, TEXT("PLAYAREA floor x[%.0f..%.0f] y[%.0f..%.0f]"), Area.Min.X, Area.Max.X, Area.Min.Y, Area.Max.Y);
	}
	else
	{
		UE_LOG(LogEnemies, Warning, TEXT("PLAYAREA no floor found: nothing is kept on the map."));
	}
}

FVector UPlayAreaSubsystem::ClampToArea(const FVector& Location, float Margin) const
{
	if (!bHasArea)
	{
		return Location;
	}
	return FVector(
		FMath::Clamp(Location.X, Area.Min.X + Margin, Area.Max.X - Margin),
		FMath::Clamp(Location.Y, Area.Min.Y + Margin, Area.Max.Y - Margin),
		Location.Z);
}
