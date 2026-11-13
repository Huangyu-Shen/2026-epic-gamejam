// Director's Cut - Story 004 (design/game-brief.md, MVP feature 2).
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "XpPickup.generated.h"

class UStaticMeshComponent;

/**
 * A dropped XP gem (small green cube). It does not tick or collide; the progression
 * subsystem pulls nearby pickups toward the hero and collects them on contact.
 */
UCLASS()
class GAME_01_API AXpPickup : public AActor
{
	GENERATED_BODY()

public:
	AXpPickup();

	/** XP granted when collected. Set by the spawner before the pickup registers. */
	float Value = 5.0f;

	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

private:
	UPROPERTY(VisibleAnywhere, Category = "Pickup")
	TObjectPtr<UStaticMeshComponent> Body;
};
