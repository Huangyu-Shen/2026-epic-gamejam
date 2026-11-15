#include "Player/HeroCharacter.h"
#include "Engine/GameInstance.h"
#include "UI/PauseMenuWidget.h"
#include "Camp/CampMetaSubsystem.h"
#include "Combat/AutoWeaponComponent.h"
#include "Combat/HealthComponent.h"
#include "Editing/FilmCardSubsystem.h"
#include "Enemies/EnemyManagerSubsystem.h"
#include "Enemies/EnemySpawnSubsystem.h"
#include "Extraction/RunOutcomeSubsystem.h"
#include "Game/PlayAreaSubsystem.h"
#include "Loot/LootBagComponent.h"
#include "Loot/LootContainer.h"
#include "Loot/LootHeatSubsystem.h"
#include "Progression/RunProgressionSubsystem.h"
#include "UI/HudMessageSubsystem.h"
#include "UI/RunHudWidget.h"
#include "Blueprint/UserWidget.h"
#include "Animation/AnimSequence.h"
#include "Camera/CameraComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Engine/SkeletalMesh.h"
#include "Components/CapsuleComponent.h"
#include "Game/DebugGate.h"
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
	static ConstructorHelpers::FObjectFinder<UMaterialInterface> FlatFinder(TEXT("/Game/Materials/M_FlatColor.M_FlatColor"));
	if (FlatFinder.Succeeded())
	{
		BodyMesh->SetMaterial(0, FlatFinder.Object);
	}

	// The model from the "RPG Hero Squad" pack (Polyart style, Dungeon Mason, Fab). Without the pack the cylinder stays.
	SwordMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("SwordMesh"));
	ShieldMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("ShieldMesh"));
	static ConstructorHelpers::FObjectFinder<USkeletalMesh> HeroFinder(TEXT("/Game/RPGHeroSquad/Mesh/Character/SK_RPGHeroPolyart.SK_RPGHeroPolyart"));
	static ConstructorHelpers::FObjectFinder<UStaticMesh> SwordFinder(TEXT("/Game/RPGHeroSquad/Mesh/Weapon/SM_RPGHero_Sword01_Polyart.SM_RPGHero_Sword01_Polyart"));
	static ConstructorHelpers::FObjectFinder<UStaticMesh> ShieldFinder(TEXT("/Game/RPGHeroSquad/Mesh/Weapon/SM_RPGHero_Shield01_Polyart.SM_RPGHero_Shield01_Polyart"));
	static ConstructorHelpers::FObjectFinder<UAnimSequence> RunFinder(TEXT("/Game/RPGHeroSquad/Animation/RPGHero/InPlace/Anim_Run_IP_RPGHero.Anim_Run_IP_RPGHero"));
	static ConstructorHelpers::FObjectFinder<UAnimSequence> WalkFinder(TEXT("/Game/RPGHeroSquad/Animation/RPGHero/InPlace/Anim_Walk_IP_RPGHero.Anim_Walk_IP_RPGHero"));
	bHasModel = HeroFinder.Succeeded() && RunFinder.Succeeded() && WalkFinder.Succeeded();
	if (bHasModel)
	{
		USkeletalMeshComponent* Model = GetMesh();
		Model->SetSkeletalMesh(HeroFinder.Object);
		// The capsule's origin is its centre; the model's origin is at its feet. Scaled up so it reads from the camera.
		Model->SetRelativeLocation(FVector(0.0f, 0.0f, -GetCapsuleComponent()->GetScaledCapsuleHalfHeight()));
		Model->SetRelativeRotation(FRotator(0.0f, HeroModelYaw, 0.0f));
		Model->SetRelativeScale3D(FVector(HeroModelScale));
		Model->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		Model->SetCastShadow(false);
		RunAnimation = RunFinder.Object;
		IdleAnimation = WalkFinder.Object;
		BodyMesh->SetStaticMesh(nullptr);
		if (SwordFinder.Succeeded())
		{
			SwordMesh->SetStaticMesh(SwordFinder.Object);
		}
		if (ShieldFinder.Succeeded())
		{
			ShieldMesh->SetStaticMesh(ShieldFinder.Object);
		}
	}
	SwordMesh->SetupAttachment(GetMesh(), TEXT("Sword"));
	ShieldMesh->SetupAttachment(GetMesh(), TEXT("Shield"));
	for (UStaticMeshComponent* Gear : { SwordMesh.Get(), ShieldMesh.Get() })
	{
		Gear->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		Gear->SetGenerateOverlapEvents(false);
		Gear->SetCastShadow(false);
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

	if (bHasModel)
	{
		GetMesh()->SetAnimationMode(EAnimationMode::AnimationSingleNode);
		GetMesh()->SetAnimation(IdleAnimation);
		GetMesh()->Play(true);
		GetMesh()->SetPlayRate(0.0f);
	}

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

	// The Esc menu sits above the HUD and stays hidden until Esc is pressed.
	PauseMenu = CreateWidget<UPauseMenuWidget>(PC);
	if (PauseMenu)
	{
		PauseMenu->SetContext(EPauseContext::Run);
		PauseMenu->AddToViewport(50);
		PauseMenu->SetVisibility(ESlateVisibility::SelfHitTestInvisible);
	}

	PC->bShowMouseCursor = true;
	FInputModeGameAndUI Mode;
	Mode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
	Mode.SetHideCursorDuringCapture(false);
	PC->SetInputMode(Mode);
}

void AHeroCharacter::QuitKey()
{
	// A level-up offer comes first: it has its own buttons and keys.
	const URunProgressionSubsystem* Progression = GetWorld()->GetSubsystem<URunProgressionSubsystem>();
	if (PauseMenu && !(Progression && Progression->IsOfferPending()))
	{
		PauseMenu->Toggle();
	}
}

void AHeroCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	PlayerInputComponent->BindKey(EKeys::SpaceBar, IE_Pressed, this, &AHeroCharacter::DodgeKey);
	PlayerInputComponent->BindKey(EKeys::E, IE_Pressed, this, &AHeroCharacter::SearchNearestContainerKey);
	if (GameDebug::CheatsEnabled())
	{
		// Debug only: kills the hero. Not bound in a Shipping build, nor in a package started without -DevCheats.
		PlayerInputComponent->BindKey(EKeys::K, IE_Pressed, this, &AHeroCharacter::DebugDieKey);
	}
	// Esc must also work while the menu has the game paused (that is how it closes again).
	PlayerInputComponent->BindKey(EKeys::Escape, IE_Pressed, this, &AHeroCharacter::QuitKey).bExecuteWhenPaused = true;

	// Level-up offer: 1/2/3 take that option, 4/5/6 cut it for good (it goes to the boss) and keep the offer open.
	const FKey TakeKeys[] = { EKeys::One, EKeys::Two, EKeys::Three };
	const FKey CutKeys[] = { EKeys::Four, EKeys::Five, EKeys::Six };
	for (int32 Index = 0; Index < UE_ARRAY_COUNT(TakeKeys); ++Index)
	{
		FInputKeyBinding Take(FInputChord(TakeKeys[Index]), IE_Pressed);
		Take.KeyDelegate.GetDelegateForManualSet().BindWeakLambda(this, [this, Index]()
		{
			GetWorld()->GetSubsystem<URunProgressionSubsystem>()->ChooseOption(Index);
		});
		PlayerInputComponent->KeyBindings.Add(Take);

		FInputKeyBinding Cut(FInputChord(CutKeys[Index]), IE_Pressed);
		Cut.KeyDelegate.GetDelegateForManualSet().BindWeakLambda(this, [this, Index]()
		{
			GetWorld()->GetSubsystem<URunProgressionSubsystem>()->CutFromOffer(Index);
		});
		PlayerInputComponent->KeyBindings.Add(Cut);
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
	const UCampMetaSubsystem* Camp = UGameInstance::GetSubsystem<UCampMetaSubsystem>(GetGameInstance());
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
	if (const URunProgressionSubsystem* Progression = GetWorld()->GetSubsystem<URunProgressionSubsystem>())
	{
		Amount *= 1.0f - Progression->GetDamageReduction();
	}
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

	KeepOnFloor();
	Regenerate(DeltaSeconds);
	ReadMoveInput();
	const bool bDashing = MoveOrDash(DeltaSeconds);
	UpdateModelAnimation(bDashing);

	const float ProgressionSpeed = GetWorld()->GetSubsystem<URunProgressionSubsystem>()->GetMoveSpeedMultiplier();
	GetCharacterMovement()->MaxWalkSpeed = BaseWalkSpeed * ProgressionSpeed;
}

void AHeroCharacter::KeepOnFloor()
{
	// The map has no walls and a void around it: the hero stops at the edge of the floor instead of falling.
	if (const UPlayAreaSubsystem* PlayArea = GetWorld()->GetSubsystem<UPlayAreaSubsystem>())
	{
		const FVector Here = GetActorLocation();
		const FVector Inside = PlayArea->ClampToArea(Here, GetCapsuleComponent()->GetScaledCapsuleRadius());
		if (!Inside.Equals(Here, 0.1f))
		{
			SetActorLocation(Inside, false, nullptr, ETeleportType::TeleportPhysics);
		}
	}
}

void AHeroCharacter::Regenerate(float DeltaSeconds)
{
	// Regen passive: health back while a run is on and no level-up offer has the game paused.
	if (const URunProgressionSubsystem* Progression = GetWorld()->GetSubsystem<URunProgressionSubsystem>())
	{
		const URunOutcomeSubsystem* Run = GetWorld()->GetSubsystem<URunOutcomeSubsystem>();
		if (!Progression->IsOfferPending() && Run && Run->IsRunActive())
		{
			Health->Heal(Progression->GetRegenPerSecond() * DeltaSeconds);
		}
	}
}

void AHeroCharacter::ReadMoveInput()
{
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
}

bool AHeroCharacter::MoveOrDash(float DeltaSeconds)
{
	// A dash that is in progress when the level-up offer opens waits there and goes on when the offer closes.
	const bool bDashing = DodgeTimeRemaining > 0.0f && CanAct();
	if (bDashing)
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
	return bDashing;
}

void AHeroCharacter::UpdateModelAnimation(bool bDashing)
{
	// Run while the hero actually moves (the dash too); a frozen walk pose when standing still.
	if (bHasModel)
	{
		const float Speed = GetVelocity().Size2D();
		const bool bMoving = Speed > 20.0f || bDashing;
		if (bMoving != bShowingRun)
		{
			bShowingRun = bMoving;
			GetMesh()->SetAnimation(bMoving ? RunAnimation : IdleAnimation);
			GetMesh()->Play(true);
		}
		GetMesh()->SetPlayRate(bMoving ? FMath::Clamp(Speed / BaseWalkSpeed, 0.6f, 1.6f) : 0.0f);
	}
}
