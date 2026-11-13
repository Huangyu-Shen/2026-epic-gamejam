#include "Camp/CampPawn.h"
#include "Camp/CampBuilding.h"
#include "Camp/CampMetaSubsystem.h"
#include "UI/CampWidget.h"
#include "UI/HudMessageSubsystem.h"
#include "Blueprint/UserWidget.h"
#include "Camera/CameraComponent.h"
#include "Components/InputComponent.h"
#include "Components/SceneComponent.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/SpringArmComponent.h"
#include "InputCoreTypes.h"

ACampPawn::ACampPawn()
{
	Root = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	RootComponent = Root;

	// Fixed angled top-down camera; the camp is not walkable, only clicked.
	Boom = CreateDefaultSubobject<USpringArmComponent>(TEXT("Boom"));
	Boom->SetupAttachment(Root);
	Boom->SetUsingAbsoluteRotation(true);
	Boom->SetRelativeRotation(FRotator(-75.0f, 0.0f, 0.0f));
	Boom->TargetArmLength = 1400.0f;
	Boom->bDoCollisionTest = false;

	Camera = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera"));
	Camera->SetupAttachment(Boom);

	AutoPossessPlayer = EAutoReceiveInput::Player0;
}

void ACampPawn::BeginPlay()
{
	Super::BeginPlay();

	APlayerController* PC = GetWorld()->GetFirstPlayerController();
	if (!PC)
	{
		return;
	}

	PC->bShowMouseCursor = true;
	PC->bEnableClickEvents = true;
	PC->bEnableMouseOverEvents = true;
	FInputModeGameAndUI Mode;
	Mode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
	Mode.SetHideCursorDuringCapture(false);
	PC->SetInputMode(Mode);

	// The camp screen: Iron, building upgrades, warehouse, editing room, Start Run.
	CampScreen = CreateWidget<UCampWidget>(PC);
	if (CampScreen)
	{
		CampScreen->AddToViewport();
		// Only the panel's controls take clicks; the rest of the screen reaches the 3D scene.
		CampScreen->SetVisibility(ESlateVisibility::SelfHitTestInvisible);
	}
}

void ACampPawn::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	PlayerInputComponent->BindKey(EKeys::LeftMouseButton, IE_Pressed, this, &ACampPawn::OnClick);
}

void ACampPawn::OnClick()
{
	APlayerController* PC = Cast<APlayerController>(GetController());
	if (!PC)
	{
		return;
	}

	FHitResult Hit;
	if (!PC->GetHitResultUnderCursor(ECC_Visibility, false, Hit))
	{
		return;
	}

	ACampBuilding* Building = Cast<ACampBuilding>(Hit.GetActor());
	UCampMetaSubsystem* Camp = GetGameInstance()->GetSubsystem<UCampMetaSubsystem>();
	if (!Building || !Camp)
	{
		return;
	}

	FText Reason;
	if (Camp->TryUpgrade(Building->GetBuildingType(), Reason))
	{
		const FString Name = StaticEnum<EBuildingType>()->GetDisplayNameTextByValue(static_cast<int64>(Building->GetBuildingType())).ToString();
		UHudMessageSubsystem::Post(this,
			FString::Printf(TEXT("%s upgraded to level %d"), *Name, Camp->GetBuildingLevel(Building->GetBuildingType())),
			FLinearColor(0.35f, 0.9f, 0.4f));
	}
	else
	{
		UHudMessageSubsystem::Post(this, Reason.ToString(), FLinearColor(0.95f, 0.35f, 0.3f));
	}
}
