// Director's Cut - Story 006. Point-and-click camera pawn for the camp scene (not walkable).
// Story 007: it also owns the camp screen (UCampWidget) and starts the run from there.
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "CampPawn.generated.h"

class UCameraComponent;
class UCampWidget;
class UPauseMenuWidget;
class USceneComponent;
class USpringArmComponent;

UCLASS()
class GAME_01_API ACampPawn : public APawn
{
	GENERATED_BODY()

public:
	ACampPawn();

	virtual void SetupPlayerInputComponent(UInputComponent* PlayerInputComponent) override;

	/** Opens or closes the Esc menu (also called by the camp screen's MENU button). */
	void TogglePauseMenu();

protected:
	virtual void BeginPlay() override;

private:
	/** Left click on a building in the scene upgrades it (the panel's buttons do the same). */
	void OnClick();

	/** Esc opens the pause menu. */
	void QuitKey();

	UPROPERTY(VisibleAnywhere, Category = "Camp")
	TObjectPtr<USceneComponent> Root;

	UPROPERTY(VisibleAnywhere, Category = "Camp")
	TObjectPtr<USpringArmComponent> Boom;

	UPROPERTY(VisibleAnywhere, Category = "Camp")
	TObjectPtr<UCameraComponent> Camera;

	UPROPERTY(Transient)
	TObjectPtr<UCampWidget> CampScreen;

	UPROPERTY(Transient)
	TObjectPtr<UPauseMenuWidget> PauseMenu;
};
