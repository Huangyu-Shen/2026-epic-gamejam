// Director's Cut - Story 006. The greybox warehouse: a decorative cube in the camp (relics can no longer be dismantled, story 008).
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "CampWarehouse.generated.h"

class UStaticMeshComponent;

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

	UPROPERTY(VisibleAnywhere, Category = "Camp")
	TObjectPtr<UStaticMeshComponent> Model;
};
