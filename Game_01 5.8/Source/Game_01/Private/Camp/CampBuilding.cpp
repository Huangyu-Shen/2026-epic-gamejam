#include "Camp/CampBuilding.h"
#include "Camp/CampMetaSubsystem.h"
#include "Components/StaticMeshComponent.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Engine/StaticMesh.h"
#include "UObject/ConstructorHelpers.h"

ACampBuilding::ACampBuilding()
{
	PrimaryActorTick.bCanEverTick = false;

	Mesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh"));
	RootComponent = Mesh;
	Mesh->SetCollisionProfileName(UCollisionProfile::BlockAll_ProfileName); // clickable via visibility trace

	static ConstructorHelpers::FObjectFinder<UStaticMesh> CubeFinder(TEXT("/Engine/BasicShapes/Cube.Cube"));
	if (CubeFinder.Succeeded())
	{
		Mesh->SetStaticMesh(CubeFinder.Object);
	}
	// Cube defaults to WorldGridMaterial (no custom params). BasicShapeMaterial exposes the
	// "Color" vector parameter (the Cylinder hero uses it); DefaultMaterial does NOT.
	static ConstructorHelpers::FObjectFinder<UMaterialInterface> DefaultMat(TEXT("/Engine/BasicShapes/BasicShapeMaterial.BasicShapeMaterial"));
	if (DefaultMat.Succeeded())
	{
		Mesh->SetMaterial(0, DefaultMat.Object);
	}
	Mesh->SetRelativeScale3D(FVector(1.0f, 1.0f, 1.2f));
}

void ACampBuilding::BeginPlay()
{
	Super::BeginPlay();

	if (UCampMetaSubsystem* Camp = GetGameInstance()->GetSubsystem<UCampMetaSubsystem>())
	{
		Camp->OnMetaChanged.AddDynamic(this, &ACampBuilding::HandleMetaChanged);
		SetLevel(Camp->GetBuildingLevel(BuildingType));
	}
}

void ACampBuilding::HandleMetaChanged()
{
	if (const UCampMetaSubsystem* Camp = GetGameInstance()->GetSubsystem<UCampMetaSubsystem>())
	{
		SetLevel(Camp->GetBuildingLevel(BuildingType));
	}
}

void ACampBuilding::SetLevel(int32 NewLevel)
{
	CachedLevel = NewLevel;

	// Visible growth: each level makes the building taller.
	const float ScaleZ = 1.2f + 0.5f * static_cast<float>(FMath::Max(NewLevel, 0));
	Mesh->SetRelativeScale3D(FVector(1.0f, 1.0f, ScaleZ));

	// Tint by building type (Smithy orange, Clinic green).
	const FLinearColor Color = (BuildingType == EBuildingType::Clinic) ? FLinearColor(0.1f, 0.7f, 0.2f) : FLinearColor(0.9f, 0.5f, 0.1f);
	if (UMaterialInstanceDynamic* Material = Mesh->CreateDynamicMaterialInstance(0))
	{
		Material->SetVectorParameterValue(TEXT("Color"), Color);
	}
}
