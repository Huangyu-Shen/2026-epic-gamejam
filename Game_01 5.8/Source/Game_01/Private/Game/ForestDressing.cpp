#include "Game/ForestDressing.h"
#include "Components/InstancedStaticMeshComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "Engine/World.h"
#include "Math/RandomStream.h"
#include "UObject/ConstructorHelpers.h"

namespace
{
	/** The land mass is 10000 cm square with its top face at local Z 0. */
	constexpr float LandSize = 10000.0f;

	/** The rim starts this far past the play area (the camera sits up to ~700 cm behind the hero, so nothing may stand closer) (cm). */
	constexpr float RimStart = 900.0f;
	constexpr float RimDepth = 900.0f;

	void Setup(UInstancedStaticMeshComponent* Component, UStaticMesh* Mesh)
	{
		Component->SetStaticMesh(Mesh);
		Component->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		Component->SetGenerateOverlapEvents(false);
		Component->SetCastShadow(false);
	}
}

AForestDressing::AForestDressing()
{
	PrimaryActorTick.bCanEverTick = false;

	const FString Root = TEXT("/Game/RPGTinyFantasyForest/Mesh/");
	auto Load = [&](const TCHAR* Path) -> UStaticMesh*
	{
		ConstructorHelpers::FObjectFinder<UStaticMesh> Finder(*(Root + Path));
		return Finder.Succeeded() ? Finder.Object : nullptr;
	};
	UStaticMesh* LandMesh = Load(TEXT("LandMass/SM_LM100RND.SM_LM100RND"));
	UStaticMesh* TreeMesh = Load(TEXT("TreePlant/SM_TreeB.SM_TreeB"));
	UStaticMesh* RockMesh = Load(TEXT("Rock/SM_RockA.SM_RockA"));
	UStaticMesh* CliffMesh = Load(TEXT("Rock/SM_RockCliffE.SM_RockCliffE"));
	UStaticMesh* GrassMesh = Load(TEXT("TreePlant/SM_GrassA.SM_GrassA"));
	UStaticMesh* FlowerMesh = Load(TEXT("TreePlant/SM_FlowerA.SM_FlowerA"));

	Land = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Land"));
	SetRootComponent(Land);
	Land->SetMobility(EComponentMobility::Movable);
	Land->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	Land->SetCastShadow(false);
	bHasLand = LandMesh != nullptr;
	Land->SetStaticMesh(LandMesh);

	Trees = CreateDefaultSubobject<UInstancedStaticMeshComponent>(TEXT("Trees"));
	Rocks = CreateDefaultSubobject<UInstancedStaticMeshComponent>(TEXT("Rocks"));
	Cliffs = CreateDefaultSubobject<UInstancedStaticMeshComponent>(TEXT("Cliffs"));
	Grass = CreateDefaultSubobject<UInstancedStaticMeshComponent>(TEXT("Grass"));
	Flowers = CreateDefaultSubobject<UInstancedStaticMeshComponent>(TEXT("Flowers"));
	for (UInstancedStaticMeshComponent* Component : { Trees.Get(), Rocks.Get(), Cliffs.Get(), Grass.Get(), Flowers.Get() })
	{
		Component->SetupAttachment(Land);
	}
	Setup(Trees, TreeMesh);
	Setup(Rocks, RockMesh);
	Setup(Cliffs, CliffMesh);
	Setup(Grass, GrassMesh);
	Setup(Flowers, FlowerMesh);
}

bool AForestDressing::Build(UWorld* World, const FBox2D& Area, float FloorTopZ, AActor* OldFloor)
{
	if (!World)
	{
		return false;
	}
	FActorSpawnParameters Params;
	Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	AForestDressing* Dressing = World->SpawnActor<AForestDressing>(AForestDressing::StaticClass(), FTransform::Identity, Params);
	if (!Dressing)
	{
		return false;
	}
	if (!Dressing->bHasLand)
	{
		Dressing->Destroy();
		return false;
	}
	Dressing->Fill(Area, FloorTopZ);
	if (OldFloor)
	{
		// Hidden, not removed: it still carries the collision the hero stands on.
		OldFloor->SetActorHiddenInGame(true);
	}
	return true;
}

void AForestDressing::Fill(const FBox2D& Area, float FloorTopZ)
{
	const FVector2D Centre = Area.GetCenter();
	const FVector2D Size = Area.GetSize();

	// The land mass reaches a little past the rim; scale it up if the play area ever grows beyond it.
	const float Scale = FMath::Max(1.0f, (FMath::Max(Size.X, Size.Y) + 2.0f * (RimStart + RimDepth)) / LandSize);
	Land->SetWorldLocation(FVector(Centre.X, Centre.Y, FloorTopZ + 1.0f));
	Land->SetWorldScale3D(FVector(Scale, Scale, 1.0f));

	FRandomStream Random(20261115);
	auto Add = [&](UInstancedStaticMeshComponent* Component, const FVector2D& At, float ZOffset, float MinScale, float MaxScale)
	{
		const float S = Random.FRandRange(MinScale, MaxScale);
		const FTransform Transform(FRotator(0.0f, Random.FRandRange(0.0f, 360.0f), 0.0f), FVector(At.X, At.Y, FloorTopZ + 1.0f + ZOffset), FVector(S));
		Component->AddInstance(Transform, /*bWorldSpace=*/true);
	};

	// A point on the rim: outside the play area, between Start and Start + Depth from its edge.
	auto RimPoint = [&](float Start, float Depth) -> FVector2D
	{
		const float Out = Random.FRandRange(Start, Start + Depth);
		const FVector2D Half = Size * 0.5f;
		const float AlongX = Random.FRandRange(-Half.X - Start, Half.X + Start);
		const float AlongY = Random.FRandRange(-Half.Y - Start, Half.Y + Start);
		switch (Random.RandRange(0, 3))
		{
		case 0:  return Centre + FVector2D(Half.X + Out, AlongY);
		case 1:  return Centre + FVector2D(-Half.X - Out, AlongY);
		case 2:  return Centre + FVector2D(AlongX, Half.Y + Out);
		default: return Centre + FVector2D(AlongX, -Half.Y - Out);
		}
	};

	for (int32 i = 0; i < 160; ++i)
	{
		Add(Trees, RimPoint(RimStart, RimDepth), 0.0f, 1.0f, 1.8f);
	}
	for (int32 i = 0; i < 40; ++i)
	{
		Add(Rocks, RimPoint(RimStart, RimDepth * 0.7f), -15.0f, 0.8f, 1.6f);
	}
	for (int32 i = 0; i < 10; ++i)
	{
		Add(Cliffs, RimPoint(RimStart + RimDepth * 0.5f, RimDepth * 0.4f), -30.0f, 1.4f, 2.2f);
	}

	// Ground detail inside the play area. Small and many; one draw call each.
	const FVector2D Half = Size * 0.5f;
	for (int32 i = 0; i < 500; ++i)
	{
		const FVector2D At = Centre + FVector2D(Random.FRandRange(-Half.X, Half.X), Random.FRandRange(-Half.Y, Half.Y));
		Add(i % 3 == 0 ? Flowers.Get() : Grass.Get(), At, 0.0f, 0.8f, 1.4f);
	}
}
