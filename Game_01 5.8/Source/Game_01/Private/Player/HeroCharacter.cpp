#include "Player/HeroCharacter.h"
#include "Combat/AutoWeaponComponent.h"
#include "Combat/HealthComponent.h"
#include "Enemies/EnemyManagerSubsystem.h"
#include "Enemies/EnemySpawnSubsystem.h"
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
#include "Materials/MaterialInstanceDynamic.h"
#include "UObject/ConstructorHelpers.h"

/** Fixed on-screen key so the debug printout replaces itself every frame. */
static constexpr uint64 HeroDebugMessageKey = 1101;

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
}

void AHeroCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	PlayerInputComponent->BindKey(EKeys::SpaceBar, IE_Pressed, this, &AHeroCharacter::DodgeKey);
	PlayerInputComponent->BindKey(EKeys::E, IE_Pressed, this, &AHeroCharacter::SearchNearestContainerKey);
	PlayerInputComponent->BindKey(EKeys::K, IE_Pressed, this, &AHeroCharacter::DebugDieKey);
	PlayerInputComponent->BindKey(EKeys::N, IE_Pressed, this, &AHeroCharacter::StartNewRunKey);

	// Debug keys until story 005's film cards call these: H = horde, B = boss.
	FInputKeyBinding HordeBinding(FInputChord(EKeys::H), IE_Pressed);
	HordeBinding.KeyDelegate.GetDelegateForManualSet().BindWeakLambda(this, [this]()
	{
		GetWorld()->GetSubsystem<UEnemySpawnSubsystem>()->SpawnHorde(DebugHordeSize);
	});
	PlayerInputComponent->KeyBindings.Add(HordeBinding);

	FInputKeyBinding BossBinding(FInputChord(EKeys::B), IE_Pressed);
	BossBinding.KeyDelegate.GetDelegateForManualSet().BindWeakLambda(this, [this]()
	{
		GetWorld()->GetSubsystem<UEnemySpawnSubsystem>()->SpawnBoss();
	});
	PlayerInputComponent->KeyBindings.Add(BossBinding);

	const FKey RelicKeys[] = { EKeys::One, EKeys::Two, EKeys::Three, EKeys::Four, EKeys::Five, EKeys::Six };
	for (int32 Index = 0; Index < UE_ARRAY_COUNT(RelicKeys); ++Index)
	{
		FInputKeyBinding Binding(FInputChord(RelicKeys[Index]), IE_Pressed);
		Binding.KeyDelegate.GetDelegateForManualSet().BindWeakLambda(this, [this, Index]() { UseRelicSlot(Index); });
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
	return Run && Run->IsRunActive() && !Health->IsDead();
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

void AHeroCharacter::StartNewRunKey()
{
	GetWorld()->GetSubsystem<UEnemyManagerSubsystem>()->ClearAll();
	GetWorld()->GetSubsystem<UEnemyManagerSubsystem>()->ResetKillCount();
	GetWorld()->GetSubsystem<UEnemySpawnSubsystem>()->ResetRun();
	GetWorld()->GetSubsystem<ULootHeatSubsystem>()->ResetHeat();
	Bag->ClearBag();
	Health->ResetHealth();
	DodgeTimeRemaining = 0.0f;
	NextDodgeTime = 0.0;
	GetWorld()->GetSubsystem<URunOutcomeSubsystem>()->StartNewRun();
}

float AHeroCharacter::ReceiveEnemyDamage(float Amount)
{
	return Health->ApplyDamage(Amount * Bag->GetActiveModifier().DamageTakenMultiplier);
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

void AHeroCharacter::UseRelicSlot(int32 Index)
{
	if (!CanAct())
	{
		return;
	}
	if (!Bag->UseRelic(Index) && GEngine)
	{
		GEngine->AddOnScreenDebugMessage(-1, 1.5f, FColor::Orange,
			FString::Printf(TEXT("No relic in slot %d"), Index + 1));
	}
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

	GetCharacterMovement()->MaxWalkSpeed = BaseWalkSpeed * Bag->GetActiveModifier().MoveSpeedMultiplier;

	DrawDebugState();
}

void AHeroCharacter::DrawDebugState() const
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

	const UEnemyManagerSubsystem* Enemies = GetWorld()->GetSubsystem<UEnemyManagerSubsystem>();
	const UEnemySpawnSubsystem* Spawner = GetWorld()->GetSubsystem<UEnemySpawnSubsystem>();

	const FLootStatModifier Modifier = Bag->GetActiveModifier();
	const FString Text = FString::Printf(
		TEXT("STATE: %s\nHP %.0f/%.0f%s\nHEAT %.0f  (spawn pressure x%.2f)   SPAWN %.2f/s   run time %.0fs\nENEMIES alive %d   kills %d\nBAG %d/%d slots   Iron %d\nRELICS:%s\nBUFFS %d active   speed x%.2f  damage x%.2f  damage taken x%.2f\nWASD move   Space dodge   E search   1-6 use relic   K die   N new run   H horde   B boss"),
		RunState, Health->GetHealth(), Health->GetMaxHealth(), Health->bGodMode ? TEXT("  GOD MODE") : TEXT(""),
		Heat, Pressure, Spawner->GetCurrentSpawnRate(), Spawner->GetRunTime(),
		Enemies->GetAliveCount(), Enemies->GetKillCount(),
		Bag->GetUsedSlots(), Bag->GetCapacitySlots(), Bag->GetIronCount(), *RelicList,
		Bag->GetActiveBuffs().Num(), Modifier.MoveSpeedMultiplier, Modifier.DamageMultiplier, Modifier.DamageTakenMultiplier);

	// A fixed key replaces the same message each frame instead of stacking new lines.
	GEngine->AddOnScreenDebugMessage(HeroDebugMessageKey, 0.0f, FColor::White, Text);
}
