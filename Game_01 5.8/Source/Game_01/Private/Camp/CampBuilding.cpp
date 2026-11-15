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

	// Models from the "RPG Tiny Fantasy Forest" pack (Dungeon Mason, Fab). The pack has no houses, so these are stand-ins:
	// a rock outcrop for the smithy and a stone gate for the clinic.
	Model = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Model"));
	Model->SetupAttachment(Mesh);
	Model->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	Model->SetUsingAbsoluteScale(true);
	Model->SetUsingAbsoluteLocation(true);
	static ConstructorHelpers::FObjectFinder<UStaticMesh> SmithyFinder(TEXT("/Game/RPGTinyFantasyForest/Mesh/Rock/SM_RockCliffE.SM_RockCliffE"));
	static ConstructorHelpers::FObjectFinder<UStaticMesh> ClinicFinder(TEXT("/Game/RPGTinyFantasyForest/Mesh/BuildingUtilityDeco/SM_GateC.SM_GateC"));
	SmithyModel = SmithyFinder.Succeeded() ? SmithyFinder.Object : nullptr;
	ClinicModel = ClinicFinder.Succeeded() ? ClinicFinder.Object : nullptr;
}

void ACampBuilding::BeginPlay()
{
	Super::BeginPlay();

	UStaticMesh* Chosen = BuildingType == EBuildingType::Clinic ? ClinicModel.Get() : SmithyModel.Get();
	if (Chosen)
	{
		Model->SetStaticMesh(Chosen);
		ModelScale = BuildingType == EBuildingType::Clinic ? 0.9f : 0.35f;
		bUsesModel = true;
		Mesh->SetVisibility(false); // the cube stays for the clicks, unseen
	}

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

	if (bUsesModel)
	{
		// A wide invisible click target; the model grows taller with each level.
		const float Growth = 1.0f + 0.2f * static_cast<float>(FMath::Max(NewLevel, 0));
		Mesh->SetRelativeScale3D(FVector(4.0f, 4.0f, 3.0f));
		Model->SetWorldScale3D(FVector(ModelScale, ModelScale, ModelScale * Growth));
		Model->SetWorldLocation(GetActorLocation() - FVector(0.0f, 0.0f, 60.0f));
		return;
	}

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
