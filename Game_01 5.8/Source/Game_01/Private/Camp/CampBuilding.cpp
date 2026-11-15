#include "Camp/CampBuilding.h"
#include "Camp/CampMetaSubsystem.h"
#include "Components/SceneComponent.h"
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

	// The smithy is a factory with a chimney and a water tower from the "WW2 RTS & SLG Production Buildings" pack (Winyunq, Fab,
	// CC BY 4.0). The clinic is the "Cozy Low Poly House" (Demolition Studios, Fab), five parts that share one pivot.
	ModelRoot = CreateDefaultSubobject<USceneComponent>(TEXT("ModelRoot"));
	ModelRoot->SetupAttachment(Mesh);
	ModelRoot->SetUsingAbsoluteScale(true);
	ModelRoot->SetUsingAbsoluteLocation(true);
	auto Add = [](TArray<TObjectPtr<UStaticMesh>>& Parts, const TCHAR* Path)
	{
		ConstructorHelpers::FObjectFinder<UStaticMesh> Finder(Path);
		if (Finder.Succeeded())
		{
			Parts.Add(Finder.Object);
		}
	};
	Add(SmithyParts, TEXT("/Game/WW2Generic/ProductionBuildings/factory_6/Rig/factory_6_static/StaticMeshes/factory_6_static.factory_6_static"));
	for (const TCHAR* Part : { TEXT("SM_Foundation"), TEXT("SM_HouseMain"), TEXT("SM_Roof"), TEXT("SM_Porch"), TEXT("SM_PricksTube") })
	{
		Add(ClinicParts, *FString::Printf(TEXT("/Game/LowPolyHouse/StaticMeshes/%s.%s"), Part, Part));
	}
	// A house with only some of its parts would look broken: all five or none.
	if (ClinicParts.Num() != 5)
	{
		ClinicParts.Reset();
	}
}

void ACampBuilding::BeginPlay()
{
	Super::BeginPlay();

	const TArray<TObjectPtr<UStaticMesh>>& Parts = BuildingType == EBuildingType::Clinic ? ClinicParts : SmithyParts;
	for (UStaticMesh* PartMesh : Parts)
	{
		UStaticMeshComponent* Part = NewObject<UStaticMeshComponent>(this);
		Part->SetupAttachment(ModelRoot);
		Part->SetStaticMesh(PartMesh);
		Part->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		Part->SetCastShadow(false);
		Part->RegisterComponent();
		PartComponents.Add(Part);
	}
	if (!PartComponents.IsEmpty())
	{
		ModelScale = BuildingType == EBuildingType::Clinic ? 0.45f : 0.35f;
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
		const float Growth = 1.0f + 0.08f * static_cast<float>(FMath::Max(NewLevel, 0));
		Mesh->SetRelativeScale3D(FVector(4.0f, 4.0f, 3.0f));
		ModelRoot->SetWorldScale3D(FVector(ModelScale, ModelScale, ModelScale * Growth));
		ModelRoot->SetWorldLocation(GetActorLocation() - FVector(0.0f, 0.0f, 60.0f));
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
