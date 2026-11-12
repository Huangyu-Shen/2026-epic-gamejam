#include "StandIn/StandInCharacter.h"
#include "Extraction/RunOutcomeSubsystem.h"
#include "Loot/LootBagComponent.h"
#include "Loot/LootContainer.h"
#include "Loot/LootHeatSubsystem.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/InputComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/Engine.h"
#include "Engine/StaticMesh.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/SpringArmComponent.h"
#include "InputCoreTypes.h"
#include "UObject/ConstructorHelpers.h"

AStandInCharacter::AStandInCharacter()
{
	PrimaryActorTick.bCanEverTick = true;

	Bag = CreateDefaultSubobject<ULootBagComponent>(TEXT("Bag"));

	// Greybox body: the engine cylinder, sized to the capsule.
	BodyMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("BodyMesh"));
	BodyMesh->SetupAttachment(GetCapsuleComponent());
	BodyMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	static ConstructorHelpers::FObjectFinder<UStaticMesh> CylinderFinder(TEXT("/Engine/BasicShapes/Cylinder.Cylinder"));
	if (CylinderFinder.Succeeded())
	{
		BodyMesh->SetStaticMesh(CylinderFinder.Object);
		BodyMesh->SetRelativeScale3D(FVector(0.7f, 0.7f, 1.76f));
	}

	// Fixed top-down camera that does not turn with the pawn.
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(GetCapsuleComponent());
	CameraBoom->SetUsingAbsoluteRotation(true);
	CameraBoom->SetRelativeRotation(FRotator(-60.0f, 0.0f, 0.0f));
	CameraBoom->TargetArmLength = CameraDistance;
	CameraBoom->bDoCollisionTest = false;

	Camera = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera"));
	Camera->SetupAttachment(CameraBoom);

	bUseControllerRotationYaw = false;
	GetCharacterMovement()->bOrientRotationToMovement = true;
	GetCharacterMovement()->MaxWalkSpeed = BaseWalkSpeed;
}

void AStandInCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	PlayerInputComponent->BindKey(EKeys::E, IE_Pressed, this, &AStandInCharacter::SearchNearestContainerKey);
	PlayerInputComponent->BindKey(EKeys::K, IE_Pressed, this, &AStandInCharacter::DebugDieKey);
	PlayerInputComponent->BindKey(EKeys::N, IE_Pressed, this, &AStandInCharacter::DebugNewRunKey);

	const FKey RelicKeys[] = { EKeys::One, EKeys::Two, EKeys::Three, EKeys::Four, EKeys::Five, EKeys::Six };
	for (int32 Index = 0; Index < UE_ARRAY_COUNT(RelicKeys); ++Index)
	{
		FInputKeyBinding Binding(FInputChord(RelicKeys[Index]), IE_Pressed);
		Binding.KeyDelegate.GetDelegateForManualSet().BindWeakLambda(this, [this, Index]() { UseRelicSlot(Index); });
		PlayerInputComponent->KeyBindings.Add(Binding);
	}
}

void AStandInCharacter::SearchNearestContainerKey()
{
	SearchNearestContainer();
}

void AStandInCharacter::DebugDieKey()
{
	GetWorld()->GetSubsystem<URunOutcomeSubsystem>()->ReportPlayerDied(Bag);
}

void AStandInCharacter::DebugNewRunKey()
{
	GetWorld()->GetSubsystem<URunOutcomeSubsystem>()->StartNewRun();
}

bool AStandInCharacter::SearchNearestContainer()
{
	ALootContainer* Nearest = nullptr;
	float NearestDistSq = FMath::Square(InteractRange);
	for (TActorIterator<ALootContainer> It(GetWorld()); It; ++It)
	{
		if (It->HasBeenSearched())
		{
			continue;
		}
		const float DistSq = FVector::DistSquared2D(It->GetActorLocation(), GetActorLocation());
		if (DistSq <= NearestDistSq)
		{
			NearestDistSq = DistSq;
			Nearest = *It;
		}
	}
	return Nearest && Nearest->Interact(this);
}

void AStandInCharacter::UseRelicSlot(int32 Index)
{
	if (!Bag->UseRelic(Index) && GEngine)
	{
		GEngine->AddOnScreenDebugMessage(-1, 1.5f, FColor::Orange,
			FString::Printf(TEXT("No relic in slot %d"), Index + 1));
	}
}

void AStandInCharacter::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	if (const APlayerController* PC = Cast<APlayerController>(GetController()))
	{
		const float Forward = (PC->IsInputKeyDown(EKeys::W) ? 1.0f : 0.0f) - (PC->IsInputKeyDown(EKeys::S) ? 1.0f : 0.0f);
		const float Right = (PC->IsInputKeyDown(EKeys::D) ? 1.0f : 0.0f) - (PC->IsInputKeyDown(EKeys::A) ? 1.0f : 0.0f);
		AddMovementInput(FVector::ForwardVector, Forward);
		AddMovementInput(FVector::RightVector, Right);
	}

	// The move-speed buff from a used relic is the one stat this stand-in can show.
	GetCharacterMovement()->MaxWalkSpeed = BaseWalkSpeed * Bag->GetActiveModifier().MoveSpeedMultiplier;

	DrawDebugState();
}

void AStandInCharacter::DrawDebugState() const
{
	if (!GEngine)
	{
		return;
	}

	float Heat = 0.0f;
	float Pressure = 1.0f;
	if (const ULootHeatSubsystem* HeatSystem = GetWorld()->GetSubsystem<ULootHeatSubsystem>())
	{
		Heat = HeatSystem->GetHeat();
		Pressure = HeatSystem->GetSpawnPressureMultiplier();
	}

	FString RelicList;
	const TArray<ERelicType> Relics = Bag->GetRelics();
	for (int32 Index = 0; Index < Relics.Num(); ++Index)
	{
		RelicList += FString::Printf(TEXT(" [%d] %s"), Index + 1,
			*StaticEnum<ERelicType>()->GetDisplayNameTextByValue(static_cast<int64>(Relics[Index])).ToString());
	}
	if (RelicList.IsEmpty())
	{
		RelicList = TEXT(" none");
	}

	const URunOutcomeSubsystem* RunSystem = GetWorld()->GetSubsystem<URunOutcomeSubsystem>();
	const TCHAR* RunState = RunSystem && !RunSystem->IsRunActive() ? TEXT("RUN ENDED (N = new run)") : TEXT("run active");

	const FLootStatModifier Modifier = Bag->GetActiveModifier();
	const FString Text = FString::Printf(
		TEXT("STATE: %s\nHEAT %.0f  (spawn pressure x%.2f)\nBAG %d/%d slots   Iron %d\nRELICS:%s\nBUFFS %d active   speed x%.2f  damage x%.2f  damage taken x%.2f\nWASD move   E search   1-6 use relic   K die   N new run"),
		RunState, Heat, Pressure, Bag->GetUsedSlots(), Bag->GetCapacitySlots(), Bag->GetIronCount(), *RelicList,
		Bag->GetActiveBuffs().Num(), Modifier.MoveSpeedMultiplier, Modifier.DamageMultiplier, Modifier.DamageTakenMultiplier);

	// A fixed key replaces the same message each frame instead of stacking new lines.
	GEngine->AddOnScreenDebugMessage(1001, 0.0f, FColor::White, Text);
}
