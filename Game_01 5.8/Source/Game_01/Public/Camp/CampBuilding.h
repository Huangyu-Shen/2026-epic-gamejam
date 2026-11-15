// Director's Cut - Story 006. A greybox, clickable camp building (Smithy or Clinic).
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Camp/CampTypes.h"
#include "CampBuilding.generated.h"

class UStaticMesh;
class UStaticMeshComponent;

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

public:

private:
	UFUNCTION()
	void HandleMetaChanged();

	/** The invisible cube that takes the clicks. Its size is the clickable area when a pack model replaces it. */
	UPROPERTY(VisibleAnywhere, Category = "Camp")
	TObjectPtr<UStaticMeshComponent> Mesh;

	/** The "RPG Tiny Fantasy Forest" model standing in for the cube. Null without the pack. */
	UPROPERTY(VisibleAnywhere, Category = "Camp")
	TObjectPtr<UStaticMeshComponent> Model;

	UPROPERTY()
	TObjectPtr<UStaticMesh> SmithyModel;

	UPROPERTY()
	TObjectPtr<UStaticMesh> ClinicModel;

	bool bUsesModel = false;
	float ModelScale = 1.0f;

	int32 CachedLevel = 0;
};
