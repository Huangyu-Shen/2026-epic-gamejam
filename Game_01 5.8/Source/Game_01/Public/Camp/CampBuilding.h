// Director's Cut - Story 006. A greybox, clickable camp building (Smithy or Clinic).
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Camp/CampTypes.h"
#include "CampBuilding.generated.h"

UCLASS()
class GAME_01_API ACampBuilding : public AActor
{
	GENERATED_BODY()

public:
	ACampBuilding();

	UFUNCTION(BlueprintPure, Category = "Camp")
	EBuildingType GetBuildingType() const { return BuildingType; }

	/** What this building upgrades. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camp")
	EBuildingType BuildingType = EBuildingType::Smithy;

	/** Current visible level (0..MaxLevel); grows the mesh so progress reads at a glance. */
	UFUNCTION(BlueprintCallable, Category = "Camp")
	void SetLevel(int32 NewLevel);

protected:
	virtual void BeginPlay() override;

private:
	UFUNCTION()
	void HandleMetaChanged();

	UPROPERTY(VisibleAnywhere, Category = "Camp")
	TObjectPtr<UStaticMeshComponent> Mesh;

	int32 CachedLevel = 0;
};
