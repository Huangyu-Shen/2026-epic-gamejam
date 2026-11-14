#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "StandInCharacter.generated.h"

class UCameraComponent;
class ULootBagComponent;
class USpringArmComponent;
class UStaticMeshComponent;

/**
 * THROWAWAY stand-in pawn for story 001 (loot, bag, heat). Story 003 replaces it with the real player.
 *
 * Controls (raw keys, no input assets): WASD move, E search the nearest container.
 * Bag and heat state is printed on screen every frame.
 */
UCLASS()
class GAME_01_API AStandInCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	AStandInCharacter();

	/** How far (cm) the E key reaches when looking for a container to search. */
	UPROPERTY(EditAnywhere, Category = "StandIn")
	float InteractRange = 250.0f;

	/** Walk speed (cm/s). */
	UPROPERTY(EditAnywhere, Category = "StandIn")
	float BaseWalkSpeed = 600.0f;

	/** Camera height above the pawn (cm). */
	UPROPERTY(EditAnywhere, Category = "StandIn")
	float CameraDistance = 1400.0f;

	/** Bag that containers fill; exposed so later stories can read it. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "StandIn")
	TObjectPtr<ULootBagComponent> Bag;

	/** Searches the nearest unsearched container within InteractRange. Returns true if one was searched. */
	UFUNCTION(BlueprintCallable, Category = "StandIn")
	bool SearchNearestContainer();

	virtual void Tick(float DeltaSeconds) override;
	virtual void SetupPlayerInputComponent(UInputComponent* PlayerInputComponent) override;

private:
	void SearchNearestContainerKey();
	void DebugDieKey();
	void DebugNewRunKey();
	void DrawDebugState() const;

	UPROPERTY(VisibleAnywhere, Category = "StandIn")
	TObjectPtr<UStaticMeshComponent> BodyMesh;

	UPROPERTY(VisibleAnywhere, Category = "StandIn")
	TObjectPtr<USpringArmComponent> CameraBoom;

	UPROPERTY(VisibleAnywhere, Category = "StandIn")
	TObjectPtr<UCameraComponent> Camera;
};
