#include "Loot/LootContainer.h"
#include "Engine/GameInstance.h"
#include "Audio/GameAudioSubsystem.h"
#include "Loot/LootBagComponent.h"
#include "Loot/LootConfig.h"
#include "Extraction/CampStockSubsystem.h"
#include "UI/HudMessageSubsystem.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "Engine/Engine.h"
#include "GameFramework/Controller.h"
#include "UObject/ConstructorHelpers.h"

ALootContainer::ALootContainer()
{
	PrimaryActorTick.bCanEverTick = false;

	Mesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh"));
	SetRootComponent(Mesh);

	static ConstructorHelpers::FObjectFinder<UStaticMesh> CubeFinder(TEXT("/Engine/BasicShapes/Cube.Cube"));
	if (CubeFinder.Succeeded())
	{
		Mesh->SetStaticMesh(CubeFinder.Object);
		Mesh->SetWorldScale3D(FVector(0.6f));
	}

	static ConstructorHelpers::FObjectFinder<UStaticMesh> StumpFinder(TEXT("/Game/RPGTinyFantasyForest/Mesh/BuildingUtilityDeco/SM_TreeStump.SM_TreeStump"));
	static ConstructorHelpers::FObjectFinder<UStaticMesh> RockFinder(TEXT("/Game/RPGTinyFantasyForest/Mesh/Rock/SM_RockA.SM_RockA"));
	CrateModel = StumpFinder.Succeeded() ? StumpFinder.Object : nullptr;
	VaultModel = RockFinder.Succeeded() ? RockFinder.Object : nullptr;
}

void ALootContainer::BeginPlay()
{
	Super::BeginPlay();

	// The cube's origin is its centre and the pack models' origin is at their base, so drop by the cube's half height.
	UStaticMesh* Model = ContainerType == ELootContainerType::Vault ? VaultModel.Get() : CrateModel.Get();
	if (Model)
	{
		const float CubeHalf = Mesh->GetComponentScale().Z * 50.0f;
		Mesh->SetStaticMesh(Model);
		Mesh->SetMaterial(0, nullptr);
		Mesh->SetWorldScale3D(FVector(ContainerType == ELootContainerType::Vault ? 0.9f : 1.8f));
		Mesh->AddWorldOffset(FVector(0.0f, 0.0f, -CubeHalf));
		bUsesModel = true;
	}
}

void ALootContainer::ShowOpened()
{
	UGameAudioSubsystem::PlayIn(this, EGameSound::Pickup);
	// Greybox "opened" cue: the container sinks to a flat slab so a searched one is obvious at a glance.
	FVector Scale = Mesh->GetComponentScale();
	const float OldHeight = Scale.Z;
	Scale.Z = OldHeight * 0.25f;
	Mesh->SetWorldScale3D(Scale);
	if (!bUsesModel)
	{
		Mesh->AddWorldOffset(FVector(0.0f, 0.0f, -50.0f * (OldHeight - Scale.Z)));
	}
}

ULootBagComponent* ALootContainer::FindBag(AActor* InInstigator)
{
	ULootBagComponent* Bag = InInstigator->FindComponentByClass<ULootBagComponent>();
	if (!Bag)
	{
		if (const AController* Controller = Cast<AController>(InInstigator))
		{
			if (const APawn* Pawn = Controller->GetPawn())
			{
				Bag = Pawn->FindComponentByClass<ULootBagComponent>();
			}
		}
	}
	return Bag;
}

bool ALootContainer::RollContents(const ULootBagComponent& Bag, ULootConfig& Config)
{
	FLootContainerRow Row;
	if (!Config.FindContainerRow(ContainerType, Row))
	{
		UE_LOG(LogLoot, Warning, TEXT("%s: no container row for this type."), *GetName());
		return false;
	}
	const int32 IronLo = FMath::Max(0, Row.IronMin);
	RolledIron = FMath::RandRange(IronLo, FMath::Max(IronLo, Row.IronMax));
	// A lost film reel only drops if it would unlock something new and the bag does not already hold one.
	TArray<FName> Unlocked;
	if (const UWorld* World = GetWorld())
	{
		if (const UCampStockSubsystem* Stock = UGameInstance::GetSubsystem<UCampStockSubsystem>(World->GetGameInstance()))
		{
			Unlocked = Stock->GetUnlockedOptions();
		}
	}
	bRolledRelic = FMath::FRand() < Row.RelicChance && Config.PickRandomRelic(Unlocked, Bag.GetRelics(), RolledRelicType);
	bRolled = true;
	return true;
}

bool ALootContainer::Interact(AActor* InInstigator)
{
	if (bSearched || !InInstigator)
	{
		if (bSearched)
		{
			UHudMessageSubsystem::Post(this, TEXT("Already searched"), FLinearColor(0.6f, 0.6f, 0.6f), 2.0f);
		}
		return false;
	}

	ULootBagComponent* Bag = FindBag(InInstigator);
	if (!Bag)
	{
		UE_LOG(LogLoot, Warning, TEXT("%s: instigator has no ULootBagComponent."), *GetName());
		return false;
	}

	ULootConfig* UseConfig = ConfigOverride ? ConfigOverride.Get() : Bag->GetConfig();

	if (!bRolled)
	{
		if (!RollContents(*Bag, *UseConfig))
		{
			return false;
		}
		if (!bRolledRelic && RolledIron == 0)
		{
			bSearched = true; // empty container
			ShowOpened();
			return true;
		}
	}

	// The roll is kept while the bag is full, so another container may have filled the same reel slot meanwhile.
	if (bRolledRelic && Bag->GetRelics().Contains(RolledRelicType))
	{
		bRolledRelic = false;
		if (RolledIron == 0)
		{
			bSearched = true;
			ShowOpened();
			return true;
		}
	}

	const ELootResource Resource = ContainerType == ELootContainerType::Vault ? ELootResource::Stone : ELootResource::Wood;
	if (Bag->TryAddLoot(RolledIron, bRolledRelic, RolledRelicType, Resource) != EBagAddResult::Success)
	{
		return false; // refusal already reported by the bag; container remains searchable
	}

	bSearched = true;
	ShowOpened();
	const FString RelicText = bRolledRelic
		? FString::Printf(TEXT(" + lost reel %s"), *StaticEnum<ERelicType>()->GetDisplayNameTextByValue(static_cast<int64>(RolledRelicType)).ToString())
		: FString();
	UHudMessageSubsystem::Post(this, FString::Printf(TEXT("Searched: +%d %s%s"), RolledIron, Resource == ELootResource::Stone ? TEXT("stone") : TEXT("wood"), *RelicText),
		FLinearColor(1.0f, 0.85f, 0.3f), 3.0f);
	OnSearched.Broadcast(this, RolledIron, bRolledRelic, RolledRelicType);
	return true;
}
