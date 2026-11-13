// Director's Cut - Story 006. Point-and-click camera pawn for the camp scene (not walkable).
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "CampPawn.generated.h"

class UCameraComponent;
class USceneComponent;
class USpringArmComponent;

UCLASS()
class GAME_01_API ACampPawn : public APawn
{
	GENERATED_BODY()

public:
	ACampPawn();

	virtual void SetupPlayerInputComponent(UInputComponent* PlayerInputComponent) override;
	virtual void Tick(float DeltaSeconds) override;

protected:
	virtual void BeginPlay() override;

private:
	void OnClick();
	void OnDismantleKey(int32 Index);
	void OnDepartKey();
	void DrawCampState() const;

	UPROPERTY(VisibleAnywhere, Category = "Camp")
	TObjectPtr<USceneComponent> Root;

	UPROPERTY(VisibleAnywhere, Category = "Camp")
	TObjectPtr<USpringArmComponent> Boom;

	UPROPERTY(VisibleAnywhere, Category = "Camp")
	TObjectPtr<UCameraComponent> Camera;

	bool bWarehouseOpen = false;
};
