#include "Camp/CampWarehouse.h"
#include "Components/StaticMeshComponent.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Engine/StaticMesh.h"
#include "UObject/ConstructorHelpers.h"

ACampWarehouse::ACampWarehouse()
{
	PrimaryActorTick.bCanEverTick = false;

	Mesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh"));
	RootComponent = Mesh;
	Mesh->SetCollisionProfileName(UCollisionProfile::BlockAll_ProfileName);

	static ConstructorHelpers::FObjectFinder<UStaticMesh> CubeFinder(TEXT("/Engine/BasicShapes/Cube.Cube"));
	if (CubeFinder.Succeeded())
	{
		Mesh->SetStaticMesh(CubeFinder.Object);
	}
	// BasicShapeMaterial exposes the "Color" vector parameter; WorldGridMaterial/DefaultMaterial do not.
	static ConstructorHelpers::FObjectFinder<UMaterialInterface> DefaultMat(TEXT("/Engine/BasicShapes/BasicShapeMaterial.BasicShapeMaterial"));
	if (DefaultMat.Succeeded())
	{
		Mesh->SetMaterial(0, DefaultMat.Object);
	}
	Mesh->SetRelativeScale3D(FVector(1.6f, 1.6f, 0.8f));

	// The barracks from the "WW2 RTS & SLG Production Buildings" pack (Winyunq, Fab, CC BY 4.0) stands in for the warehouse cube.
	Model = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Model"));
	Model->SetupAttachment(Mesh);
	Model->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	Model->SetUsingAbsoluteScale(true);
	Model->SetUsingAbsoluteLocation(true);
	static ConstructorHelpers::FObjectFinder<UStaticMesh> RidgeFinder(TEXT("/Game/WW2Generic/ProductionBuildings/barracks/Rig/barracks_static/StaticMeshes/barracks_static.barracks_static"));
	if (RidgeFinder.Succeeded())
	{
		Model->SetStaticMesh(RidgeFinder.Object);
		Model->SetWorldScale3D(FVector(0.55f));
		Mesh->SetRelativeScale3D(FVector(6.0f, 6.0f, 1.0f)); // the invisible click cube, about the model's width
		Mesh->SetVisibility(false);
	}
}

void ACampWarehouse::BeginPlay()
{
	Super::BeginPlay();

	if (Model->GetStaticMesh())
	{
		Model->SetWorldLocation(GetActorLocation() - FVector(0.0f, 0.0f, 40.0f));
	}

	if (UMaterialInstanceDynamic* Material = Mesh->CreateDynamicMaterialInstance(0))
	{
		Material->SetVectorParameterValue(TEXT("Color"), FLinearColor(0.2f, 0.3f, 0.9f));
	}
}
