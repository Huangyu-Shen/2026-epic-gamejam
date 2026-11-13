#include "Progression/XpPickup.h"
#include "Progression/RunProgressionSubsystem.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "Engine/World.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "UObject/ConstructorHelpers.h"

AXpPickup::AXpPickup()
{
	PrimaryActorTick.bCanEverTick = false;

	Body = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Body"));
	SetRootComponent(Body);
	Body->SetMobility(EComponentMobility::Movable);
	Body->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	Body->SetCastShadow(false);
	static ConstructorHelpers::FObjectFinder<UStaticMesh> CubeFinder(TEXT("/Engine/BasicShapes/Cube.Cube"));
	if (CubeFinder.Succeeded())
	{
		Body->SetStaticMesh(CubeFinder.Object);
	}
	Body->SetWorldScale3D(FVector(0.2f));
}

void AXpPickup::BeginPlay()
{
	Super::BeginPlay();

	// Green so XP gems read against the grey floor and the blue hero.
	if (UMaterialInstanceDynamic* Material = Body->CreateDynamicMaterialInstance(0))
	{
		Material->SetVectorParameterValue(TEXT("Color"), FLinearColor(0.1f, 0.9f, 0.2f));
	}

	if (URunProgressionSubsystem* Progression = GetWorld()->GetSubsystem<URunProgressionSubsystem>())
	{
		Progression->RegisterPickup(this);
	}
}

void AXpPickup::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (EndPlayReason == EEndPlayReason::Destroyed)
	{
		if (URunProgressionSubsystem* Progression = GetWorld()->GetSubsystem<URunProgressionSubsystem>())
		{
			Progression->UnregisterPickup(this);
		}
	}
	Super::EndPlay(EndPlayReason);
}
