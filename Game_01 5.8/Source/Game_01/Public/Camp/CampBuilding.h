// Director's Cut - Story 006. A greybox, clickable camp building (Smithy or Clinic).
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Camp/CampTypes.h"
#include "CampBuilding.generated.h"

class UStaticMesh;
class USceneComponent;
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

	/** Holds the model's parts. The smithy is one factory mesh (WW2 pack); the clinic is a house made of five parts (Cozy Low Poly House pack). */
	UPROPERTY(VisibleAnywhere, Category = "Camp")
	TObjectPtr<USceneComponent> ModelRoot;

	UPROPERTY()
	TArray<TObjectPtr<UStaticMesh>> SmithyParts;

	UPROPERTY()
	TArray<TObjectPtr<UStaticMesh>> ClinicParts;

	UPROPERTY(Transient)
	TArray<TObjectPtr<UStaticMeshComponent>> PartComponents;

	bool bUsesModel = false;
	float ModelScale = 1.0f;

	int32 CachedLevel = 0;
};
