// Director's Cut - the look of the run map: a grass land mass instead of the grey checkerboard, trees and rocks around the rim.
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ForestDressing.generated.h"

class UInstancedStaticMeshComponent;
class UStaticMesh;
class UStaticMeshComponent;

/**
 * Dresses the run map with meshes from the "RPG Tiny Fantasy Forest" pack (Dungeon Mason, Fab): one big grass land mass
 * under the play area, a ring of trees, rocks and cliffs outside it, and grass tufts and flowers inside it. It is only
 * decoration: nothing here has collision, and the play area itself is still limited by UPlayAreaSubsystem.
 * Without the pack nothing is built and the map keeps its old floor.
 */
UCLASS(NotPlaceable)
class GAME_01_API AForestDressing : public AActor
{
	GENERATED_BODY()

public:
	AForestDressing();

	/**
	 * Builds the dressing for a play area (X/Y limits) whose floor top is at FloorTopZ, and hides OldFloor.
	 * Returns false, and spawns nothing, when the pack's meshes are not in the project.
	 */
	static bool Build(UWorld* World, const FBox2D& Area, float FloorTopZ, AActor* OldFloor);

private:
	void Fill(const FBox2D& Area, float FloorTopZ);

	UPROPERTY()
	TObjectPtr<UStaticMeshComponent> Land;

	UPROPERTY()
	TObjectPtr<UInstancedStaticMeshComponent> Trees;

	UPROPERTY()
	TObjectPtr<UInstancedStaticMeshComponent> Rocks;

	UPROPERTY()
	TObjectPtr<UInstancedStaticMeshComponent> Cliffs;

	UPROPERTY()
	TObjectPtr<UInstancedStaticMeshComponent> Grass;

	UPROPERTY()
	TObjectPtr<UInstancedStaticMeshComponent> Flowers;

	bool bHasLand = false;
};
