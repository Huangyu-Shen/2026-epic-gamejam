#include "Player/HeroCharacter.h"
#include "Camp/CampMetaSubsystem.h"
#include "Combat/AutoWeaponComponent.h"
#include "Combat/HealthComponent.h"
#include "Editing/FilmCardSubsystem.h"
#include "Enemies/EnemyManagerSubsystem.h"
#include "Enemies/EnemySpawnSubsystem.h"
#include "Extraction/RunOutcomeSubsystem.h"
#include "Loot/LootBagComponent.h"
#include "Loot/LootContainer.h"
#include "Loot/LootHeatSubsystem.h"
#include "Progression/RunProgressionSubsystem.h"
#include "UI/HudMessageSubsystem.h"
#include "UI/RunHudWidget.h"
#include "Blueprint/UserWidget.h"
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
#include "Kismet/GameplayStatics.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "UObject/ConstructorHelpers.h"

AHeroCharacter::AHeroCharacter()
{
	PrimaryActorTick.bCanEverTick = true;

	Bag = CreateDefaultSubobject<ULootBagComponent>(TEXT("Bag"));
	Health = CreateDefaultSubobject<UHealthComponent>(TEXT("Health"));
	Weapon = CreateDefaultSubobject<UAutoWeaponComponent>(TEXT("Weapon"));

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

void AHeroCharacter::BeginPlay()
{
	Super::BeginPlay();
	CameraBoom->TargetArmLength = CameraDistance;
	Health->OnDied.AddDynamic(this, &AHeroCharacter::HandleDied);

	// Greybox readability: the hero is blue so it stands out from the grey floor and the red enemies.
	if (UMaterialInstanceDynamic* Material = BodyMesh->CreateDynamicMaterialInstance(0))
	{
		Material->SetVectorParameterValue(TEXT("Color"), FLinearColor(0.05f, 0.3f, 0.9f));
	}

	// The run map is entered from the camp: set the run up (camp bonuses, film cards) and show the HUD.
	StartRun();
	CreateHud();
}

void AHeroCharacter::CreateHud()
{
	APlayerController* PC = Cast<APlayerController>(GetController());
	if (!PC)
	{
		PC = UGameplayStatics::GetPlayerController(this, 0);
	}
	if (!PC || Hud)
	{
		return;
	}

	Hud = CreateWidget<URunHudWidget>(PC);
	if (Hud)
	{
		Hud->AddToViewport();
		// Only the buttons take mouse clicks; the rest of the screen stays click-through.
		Hud->SetVisibility(ESlateVisibility::SelfHitTestInvisible);
	}

	PC->bShowMouseCursor = true;
	FInputModeGameAndUI Mode;
	Mode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
	Mode.SetHideCursorDuringCapture(false);
	PC->SetInputMode(Mode);
}

void AHeroCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	PlayerInputComponent->BindKey(EKeys::SpaceBar, IE_Pressed, this, &AHeroCharacter::DodgeKey);
	PlayerInputComponent->BindKey(EKeys::E, IE_Pressed, this, &AHeroCharacter::SearchNearestContainerKey);
	PlayerInputComponent->BindKey(EKeys::K, IE_Pressed, this, &AHeroCharacter::DebugDieKey);

	// 1/2/3 cut a level-up option while the offer is showing (brief v2). A lost film reel cannot be used.
	const FKey OfferKeys[] = { EKeys::One, EKeys::Two, EKeys::Three };
	for (int32 Index = 0; Index < UE_ARRAY_COUNT(OfferKeys); ++Index)
	{
		FInputKeyBinding Binding(FInputChord(OfferKeys[Index]), IE_Pressed);
		Binding.KeyDelegate.GetDelegateForManualSet().BindWeakLambda(this, [this, Index]()
		{
			GetWorld()->GetSubsystem<URunProgressionSubsystem>()->CutOffer(Index);
		});
		PlayerInputComponent->KeyBindings.Add(Binding);
	}
}

bool AHeroCharacter::IsDead() const
{
	return Health->IsDead();
}

bool AHeroCharacter::CanAct() const
{
	const URunOutcomeSubsystem* Run = GetWorld()->GetSubsystem<URunOutcomeSubsystem>();
	if (!Run || !Run->IsRunActive() || Health->IsDead())
	{
		return false;
	}
	// Soft-pause while the level-up offer is showing (story 004).
	if (const URunProgressionSubsystem* Progression = GetWorld()->GetSubsystem<URunProgressionSubsystem>())
	{
		if (Progression->IsOfferPending())
		{
			return false;
		}
	}
	return true;
}

void AHeroCharacter::SearchNearestContainerKey()
{
	if (CanAct())
	{
		SearchNearestContainer();
	}
}

void AHeroCharacter::DodgeKey()
{
	TryDodge();
}

void AHeroCharacter::DebugDieKey()
{
	Health->Kill();
}

void AHeroCharacter::HandleDied()
{
	GetWorld()->GetSubsystem<URunOutcomeSubsystem>()->ReportPlayerDied(Bag);
}

void AHeroCharacter::StartRun()
{
	GetWorld()->GetSubsystem<UEnemyManagerSubsystem>()->ClearAll();
	GetWorld()->GetSubsystem<UEnemyManagerSubsystem>()->ResetKillCount();
	GetWorld()->GetSubsystem<UEnemySpawnSubsystem>()->ResetRun();
	GetWorld()->GetSubsystem<ULootHeatSubsystem>()->ResetHeat();
	Bag->ClearBag();
	// Camp building bonuses (story 006): Clinic raises max health; applied fresh each run (no compounding).
	const UCampMetaSubsystem* Camp = GetGameInstance()->GetSubsystem<UCampMetaSubsystem>();
	Health->SetMaxHealth(Health->GetBaseMaxHealth() + (Camp ? Camp->GetMaxHealthBonus() : 0.0f));
	GetWorld()->GetSubsystem<URunProgressionSubsystem>()->ResetProgression();
	DodgeTimeRemaining = 0.0f;
	NextDodgeTime = 0.0;
	GetWorld()->GetSubsystem<URunOutcomeSubsystem>()->StartNewRun();

	// Apply the slotted film cards (story 005): force horde/boss and set the earnings multiplier.
	GetWorld()->GetSubsystem<UFilmCardSubsystem>()->ApplyActiveCards();
}

float AHeroCharacter::ReceiveEnemyDamage(float Amount)
{
	return Health->ApplyDamage(Amount);
}

bool AHeroCharacter::TryDodge()
{
	const double Now = GetWorld()->GetTimeSeconds();
	if (!CanAct() || DodgeTimeRemaining > 0.0f || Now < NextDodgeTime)
	{
		return false;
	}

	DodgeDirection = MoveInput.IsNearlyZero() ? GetActorForwardVector().GetSafeNormal2D() : MoveInput;
	DodgeTimeRemaining = Dodge.Duration;
	NextDodgeTime = Now + Dodge.Cooldown;
	Health->GrantInvulnerability(Dodge.InvulnerabilitySeconds);
	SetActorRotation(DodgeDirection.Rotation());
	return true;
}

bool AHeroCharacter::SearchNearestContainer()
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

void AHeroCharacter::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	MoveInput = FVector::ZeroVector;
	if (CanAct())
	{
		if (const APlayerController* PC = Cast<APlayerController>(GetController()))
		{
			const float Forward = (PC->IsInputKeyDown(EKeys::W) ? 1.0f : 0.0f) - (PC->IsInputKeyDown(EKeys::S) ? 1.0f : 0.0f);
			const float Right = (PC->IsInputKeyDown(EKeys::D) ? 1.0f : 0.0f) - (PC->IsInputKeyDown(EKeys::A) ? 1.0f : 0.0f);
			MoveInput = FVector(Forward, Right, 0.0f).GetSafeNormal();
		}
	}

	if (DodgeTimeRemaining > 0.0f)
	{
		// The dash is a swept offset (walls stop it) and replaces normal movement while it lasts.
		const float Step = FMath::Min(DeltaSeconds, DodgeTimeRemaining);
		AddActorWorldOffset(DodgeDirection * Dodge.Speed * Step, true);
		DodgeTimeRemaining -= Step;
	}
	else
	{
		AddMovementInput(MoveInput);
	}

	const float ProgressionSpeed = GetWorld()->GetSubsystem<URunProgressionSubsystem>()->GetMoveSpeedMultiplier();
	GetCharacterMovement()->MaxWalkSpeed = BaseWalkSpeed * ProgressionSpeed;
}
