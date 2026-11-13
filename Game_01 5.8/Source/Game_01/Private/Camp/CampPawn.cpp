#include "Camp/CampPawn.h"
#include "Camp/CampBuilding.h"
#include "Camp/CampMetaSubsystem.h"
#include "Camp/CampWarehouse.h"
#include "Extraction/CampStockSubsystem.h"
#include "Camera/CameraComponent.h"
#include "Components/InputComponent.h"
#include "Components/SceneComponent.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/SpringArmComponent.h"
#include "InputCoreTypes.h"
#include "Kismet/GameplayStatics.h"
#include "Loot/LootTypes.h"

/** Fixed on-screen key so the camp debug printout replaces itself every frame. */
static constexpr uint64 CampDebugMessageKey = 1201;

ACampPawn::ACampPawn()
{
	PrimaryActorTick.bCanEverTick = true;

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

	if (APlayerController* PC = GetWorld()->GetFirstPlayerController())
	{
		PC->bShowMouseCursor = true;
		PC->bEnableClickEvents = true;
		PC->bEnableMouseOverEvents = true;
	}
}

void ACampPawn::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	PlayerInputComponent->BindKey(EKeys::LeftMouseButton, IE_Pressed, this, &ACampPawn::OnClick);
	PlayerInputComponent->BindKey(EKeys::N, IE_Pressed, this, &ACampPawn::OnDepartKey);

	const FKey RelicKeys[] = { EKeys::One, EKeys::Two, EKeys::Three, EKeys::Four, EKeys::Five, EKeys::Six };
	for (int32 Index = 0; Index < UE_ARRAY_COUNT(RelicKeys); ++Index)
	{
		FInputKeyBinding Binding(FInputChord(RelicKeys[Index]), IE_Pressed);
		Binding.KeyDelegate.GetDelegateForManualSet().BindWeakLambda(this, [this, Index]() { OnDismantleKey(Index); });
		PlayerInputComponent->KeyBindings.Add(Binding);
	}
}

void ACampPawn::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
	DrawCampState();
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

	UCampMetaSubsystem* Camp = GetGameInstance()->GetSubsystem<UCampMetaSubsystem>();

	if (ACampBuilding* Building = Cast<ACampBuilding>(Hit.GetActor()))
	{
		FText Reason;
		if (Camp && Camp->TryUpgrade(Building->GetBuildingType(), Reason))
		{
			const FString Name = StaticEnum<EBuildingType>()->GetDisplayNameTextByValue(static_cast<int64>(Building->GetBuildingType())).ToString();
			GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Green,
				FString::Printf(TEXT("%s upgraded to level %d"), *Name, Camp->GetBuildingLevel(Building->GetBuildingType())));
		}
		else
		{
			GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Red, Reason.ToString());
		}
		return;
	}

	if (Cast<ACampWarehouse>(Hit.GetActor()))
	{
		bWarehouseOpen = !bWarehouseOpen;
	}
}

void ACampPawn::OnDismantleKey(int32 Index)
{
	if (!bWarehouseOpen)
	{
		return;
	}

	UCampMetaSubsystem* Camp = GetGameInstance()->GetSubsystem<UCampMetaSubsystem>();
	if (!Camp)
	{
		return;
	}

	const int32 Gained = Camp->DismantleRelic(Index);
	if (GEngine)
	{
		if (Gained > 0)
		{
			GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Green, FString::Printf(TEXT("Dismantled relic %d -> +%d Iron"), Index + 1, Gained));
		}
		else
		{
			GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Orange, FString::Printf(TEXT("No relic in slot %d"), Index + 1));
		}
	}
}

void ACampPawn::OnDepartKey()
{
	UGameplayStatics::OpenLevel(this, TEXT("L_Greybox"));
}

void ACampPawn::DrawCampState() const
{
	if (!GEngine)
	{
		return;
	}

	const UCampMetaSubsystem* Camp = GetGameInstance()->GetSubsystem<UCampMetaSubsystem>();
	const UCampStockSubsystem* Stock = GetGameInstance()->GetSubsystem<UCampStockSubsystem>();
	if (!Camp || !Stock)
	{
		return;
	}

	FString BuildingLine;
	for (uint8 B = 0; B < static_cast<uint8>(EBuildingType::Count); ++B)
	{
		const EBuildingType Type = static_cast<EBuildingType>(B);
		const int32 Level = Camp->GetBuildingLevel(Type);
		const FString Name = StaticEnum<EBuildingType>()->GetDisplayNameTextByValue(static_cast<int64>(Type)).ToString();
		BuildingLine += Camp->IsMaxLevel(Type)
			? FString::Printf(TEXT("%s Lv%d (MAX)\n"), *Name, Level)
			: FString::Printf(TEXT("%s Lv%d -> %d Iron\n"), *Name, Level, Camp->GetUpgradeCost(Type));
	}

	FString RelicList;
	const TArray<ERelicType> Relics = Stock->GetWarehouseRelics();
	for (int32 i = 0; i < Relics.Num(); ++i)
	{
		RelicList += FString::Printf(TEXT(" [%d] %s"), i + 1,
			*StaticEnum<ERelicType>()->GetDisplayNameTextByValue(static_cast<int64>(Relics[i])).ToString());
	}
	if (RelicList.IsEmpty())
	{
		RelicList = TEXT(" none");
	}

	const FString Text = FString::Printf(
		TEXT("CAMP\nIron %d\n%sSmithy bonus: damage +%.0f   Clinic bonus: max health +%.0f\nWAREHOUSE %s\nRELICS:%s\nclick building = upgrade   click warehouse = open/close   1-6 dismantle   N depart"),
		Stock->GetIron(),
		*BuildingLine,
		Camp->GetWeaponDamageBonus(), Camp->GetMaxHealthBonus(),
		bWarehouseOpen ? TEXT("OPEN") : TEXT("closed"),
		*RelicList);

	GEngine->AddOnScreenDebugMessage(CampDebugMessageKey, 0.0f, FColor::White, Text);
}
