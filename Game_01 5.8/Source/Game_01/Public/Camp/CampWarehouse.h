// Director's Cut - Story 006. The greybox warehouse; clicking it toggles the dismantle panel.
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "CampWarehouse.generated.h"

UCLASS()
class GAME_01_API ACampWarehouse : public AActor
{
	GENERATED_BODY()

public:
	ACampWarehouse();

protected:
	virtual void BeginPlay() override;

private:
	UPROPERTY(VisibleAnywhere, Category = "Camp")
	TObjectPtr<UStaticMeshComponent> Mesh;
};
