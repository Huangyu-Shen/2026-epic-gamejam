#include "Extraction/ExtractionZone.h"
#include "Audio/GameAudioSubsystem.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "UI/HudMessageSubsystem.h"
#include "Extraction/RunOutcomeSubsystem.h"
#include "Loot/LootBagComponent.h"
#include "Components/BoxComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/Engine.h"
#include "Engine/StaticMesh.h"
#include "Engine/World.h"
#include "GameFramework/Pawn.h"
#include "TimerManager.h"
#include "UObject/ConstructorHelpers.h"

AExtractionZone::AExtractionZone()
{
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.bStartWithTickEnabled = false;

	// Overlap volume is the root so it stays unscaled; size it with the box extent.
	ZoneVolume = CreateDefaultSubobject<UBoxComponent>(TEXT("ZoneVolume"));
	SetRootComponent(ZoneVolume);
	ZoneVolume->SetCollisionProfileName(TEXT("OverlapOnlyPawn"));
	ZoneVolume->SetBoxExtent(FVector(200.0f, 200.0f, 100.0f));

	// Greybox disc: the engine cylinder (100 cm wide) flattened to match the volume's footprint.
	DiscMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("DiscMesh"));
	DiscMesh->SetupAttachment(ZoneVolume);
	DiscMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	static ConstructorHelpers::FObjectFinder<UStaticMesh> CylinderFinder(TEXT("/Engine/BasicShapes/Cylinder.Cylinder"));
	if (CylinderFinder.Succeeded())
	{
		DiscMesh->SetStaticMesh(CylinderFinder.Object);
		DiscMesh->SetRelativeScale3D(FVector(4.0f, 4.0f, 0.02f));
		DiscMesh->SetRelativeLocation(FVector(0.0f, 0.0f, -100.0f));
	}

	BeaconMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("BeaconMesh"));
	BeaconMesh->SetupAttachment(ZoneVolume);
	BeaconMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	BeaconMesh->SetCastShadow(false);
	FillMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("FillMesh"));
	FillMesh->SetupAttachment(ZoneVolume);
	FillMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	FillMesh->SetCastShadow(false);
	static ConstructorHelpers::FObjectFinder<UMaterialInterface> FlatFinder(TEXT("/Game/Materials/M_FlatColor.M_FlatColor"));
	if (CylinderFinder.Succeeded())
	{
		BeaconMesh->SetStaticMesh(CylinderFinder.Object);
		BeaconMesh->SetRelativeScale3D(FVector(0.6f, 0.6f, 14.0f));
		BeaconMesh->SetRelativeLocation(FVector(0.0f, 0.0f, 600.0f));
		FillMesh->SetStaticMesh(CylinderFinder.Object);
		FillMesh->SetRelativeScale3D(FVector(0.01f, 0.01f, 0.03f));
		FillMesh->SetRelativeLocation(FVector(0.0f, 0.0f, -95.0f));
	}
	// The portal from the "RPG Tiny Fantasy Forest" pack (Dungeon Mason, Fab) stands in the zone when it is there, next to the green beam, which stays so the exit is easy to find.
	PortalMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("PortalMesh"));
	PortalMesh->SetupAttachment(ZoneVolume);
	PortalMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	PortalMesh->SetCastShadow(false);
	static ConstructorHelpers::FObjectFinder<UStaticMesh> PortalFinder(TEXT("/Game/RPGTinyFantasyForest/Mesh/BuildingUtilityDeco/SM_PortalA.SM_PortalA"));
	if (PortalFinder.Succeeded())
	{
		PortalMesh->SetStaticMesh(PortalFinder.Object);
		PortalMesh->SetRelativeLocation(FVector(0.0f, 0.0f, -100.0f));
		PortalMesh->SetRelativeScale3D(FVector(0.5f));
	}
	else
	{
		PortalMesh->SetVisibility(false);
	}

	if (FlatFinder.Succeeded())
	{
		DiscMesh->SetMaterial(0, FlatFinder.Object);
		BeaconMesh->SetMaterial(0, FlatFinder.Object);
		FillMesh->SetMaterial(0, FlatFinder.Object);
	}
}

void AExtractionZone::UpdateFillVisual() const
{
	const float Size = FMath::Max(Progress, 0.01f) * 4.0f;
	FillMesh->SetRelativeScale3D(FVector(Size, Size, 0.03f));
}

void AExtractionZone::BeginPlay()
{
	Super::BeginPlay();

	// Bright green so it is the one thing on the map that looks like a way out.
	static const FLinearColor ZoneColor(0.1f, 1.0f, 0.3f);
	static const FLinearColor FillColor(0.9f, 1.0f, 0.2f);
	if (UMaterialInstanceDynamic* Material = DiscMesh->CreateDynamicMaterialInstance(0))
	{
		Material->SetVectorParameterValue(TEXT("Color"), ZoneColor);
	}
	if (UMaterialInstanceDynamic* Material = BeaconMesh->CreateDynamicMaterialInstance(0))
	{
		Material->SetVectorParameterValue(TEXT("Color"), ZoneColor);
	}
	if (UMaterialInstanceDynamic* Material = FillMesh->CreateDynamicMaterialInstance(0))
	{
		Material->SetVectorParameterValue(TEXT("Color"), FillColor);
	}
	ZoneVolume->OnComponentBeginOverlap.AddDynamic(this, &AExtractionZone::HandleBeginOverlap);
	ZoneVolume->OnComponentEndOverlap.AddDynamic(this, &AExtractionZone::HandleEndOverlap);

	// Overlaps that exist at level start are recorded without events (and only after every actor's BeginPlay),
	// so a pawn that starts inside the zone is picked up on the next tick instead.
	GetWorldTimerManager().SetTimerForNextTick(this, &AExtractionZone::EnterPawnsAlreadyInside);
}

void AExtractionZone::EnterPawnsAlreadyInside()
{
	TArray<AActor*> AlreadyInside;
	ZoneVolume->GetOverlappingActors(AlreadyInside, APawn::StaticClass());
	for (AActor* Actor : AlreadyInside)
	{
		TryEnter(Actor);
	}
}

void AExtractionZone::HandleBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp,
	int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	TryEnter(OtherActor);
}

void AExtractionZone::TryEnter(AActor* OtherActor)
{
	if (Occupant.IsValid() || !Cast<APawn>(OtherActor))
	{
		return;
	}
	if (ULootBagComponent* Bag = OtherActor->FindComponentByClass<ULootBagComponent>())
	{
		Occupant = Bag;
		SetActorTickEnabled(true);
		if (const URunOutcomeSubsystem* Run = GetWorld()->GetSubsystem<URunOutcomeSubsystem>(); Run && Run->IsRunActive())
		{
			UGameAudioSubsystem::PlayIn(this, EGameSound::ExtractStart);
			UHudMessageSubsystem::Post(this, TEXT("EXTRACTING - stay inside the green circle"), FLinearColor(0.3f, 1.0f, 0.4f), 3.0f);
		}
	}
}

void AExtractionZone::HandleEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp,
	int32 OtherBodyIndex)
{
	if (!OtherActor || OtherActor != (Occupant.IsValid() ? Occupant->GetOwner() : nullptr))
	{
		return;
	}
	Occupant.Reset();
	if (Tuning.bResetProgressOnLeave || Progress <= 0.0f)
	{
		Interrupt();
		SetActorTickEnabled(false);
	}
	// Otherwise keep ticking so the bar drains.
}

void AExtractionZone::Interrupt()
{
	if (Progress > 0.0f)
	{
		Progress = 0.0f;
		UpdateFillVisual();
		OnExtractionInterrupted.Broadcast();
	}
}

void AExtractionZone::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	URunOutcomeSubsystem* Run = GetWorld()->GetSubsystem<URunOutcomeSubsystem>();
	ULootBagComponent* Bag = Occupant.Get();

	// The run ended (death or another extraction): drop the bar. Keep ticking only if someone is still inside,
	// so a new run starts filling without re-entering.
	if (!Run->IsRunActive())
	{
		Interrupt();
		SetActorTickEnabled(Bag != nullptr);
		return;
	}

	if (Bag)
	{
		Progress = FMath::Min(1.0f, Progress + DeltaSeconds / Tuning.ExtractionSeconds);
		OnExtractionProgress.Broadcast(Progress);
		if (Progress >= 1.0f)
		{
			Progress = 0.0f;
			UpdateFillVisual();
			Run->ReportExtracted(Bag);
		}
		else
		{
			UpdateFillVisual();
		}
		return;
	}

	// Nobody inside and not resetting: drain the bar.
	Progress = FMath::Max(0.0f, Progress - Tuning.ProgressDecayPerSecond * DeltaSeconds);
	OnExtractionProgress.Broadcast(Progress);
	UpdateFillVisual();
	if (Progress <= 0.0f)
	{
		Interrupt();
		SetActorTickEnabled(false);
	}
}
