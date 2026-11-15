// Director's Cut - keeps the hero and every enemy on the floor. The map has no walls and a void around it.
#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "PlayAreaSubsystem.generated.h"

/**
 * Finds the walkable floor of the level when it starts (the union of the large, flat static meshes) and clamps
 * positions to it. With no floor found it does nothing, so a map without one behaves as before.
 */
UCLASS()
class GAME_01_API UPlayAreaSubsystem : public UWorldSubsystem
{
	GENERATED_BODY()

public:
	virtual void OnWorldBeginPlay(UWorld& InWorld) override;

	/** True when a floor was found. */
	bool HasArea() const { return bHasArea; }

	/** The location with X and Y pulled inside the floor, Margin cm from its edge. Z is untouched. */
	FVector ClampToArea(const FVector& Location, float Margin) const;

private:
	bool bHasArea = false;
	FBox2D Area = FBox2D(ForceInit);
};
