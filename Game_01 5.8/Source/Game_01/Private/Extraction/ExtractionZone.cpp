#include "Extraction/ExtractionZone.h"
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
}

void AExtractionZone::BeginPlay()
{
	Super::BeginPlay();
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
			Run->ReportExtracted(Bag);
		}
		return;
	}

	// Nobody inside and not resetting: drain the bar.
	Progress = FMath::Max(0.0f, Progress - Tuning.ProgressDecayPerSecond * DeltaSeconds);
	OnExtractionProgress.Broadcast(Progress);
	if (Progress <= 0.0f)
	{
		Interrupt();
		SetActorTickEnabled(false);
	}
}
