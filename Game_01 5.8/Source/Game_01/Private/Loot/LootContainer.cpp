#include "Loot/LootContainer.h"
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
}

void ALootContainer::ShowOpened()
{
	UGameAudioSubsystem::PlayIn(this, EGameSound::Pickup);
	// Greybox "opened" cue: the container sinks to a flat slab so a searched one is obvious at a glance.
	FVector Scale = Mesh->GetComponentScale();
	const float OldHeight = Scale.Z;
	Scale.Z = OldHeight * 0.25f;
	Mesh->SetWorldScale3D(Scale);
	Mesh->AddWorldOffset(FVector(0.0f, 0.0f, -50.0f * (OldHeight - Scale.Z)));
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
	if (!Bag)
	{
		UE_LOG(LogLoot, Warning, TEXT("%s: instigator has no ULootBagComponent."), *GetName());
		return false;
	}

	ULootConfig* UseConfig = ConfigOverride ? ConfigOverride.Get() : Bag->GetConfig();

	FLootContainerRow Row;
	if (!bRolled)
	{
		if (!UseConfig->FindContainerRow(ContainerType, Row))
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
			if (const UCampStockSubsystem* Stock = World->GetGameInstance()->GetSubsystem<UCampStockSubsystem>())
			{
				Unlocked = Stock->GetUnlockedOptions();
			}
		}
		bRolledRelic = FMath::FRand() < Row.RelicChance && UseConfig->PickRandomRelic(Unlocked, Bag->GetRelics(), RolledRelicType);
		bRolled = true;
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

	if (Bag->TryAddLoot(RolledIron, bRolledRelic, RolledRelicType) != EBagAddResult::Success)
	{
		return false; // refusal already reported by the bag; container remains searchable
	}

	bSearched = true;
	ShowOpened();
	const FString RelicText = bRolledRelic
		? FString::Printf(TEXT(" + lost reel %s"), *StaticEnum<ERelicType>()->GetDisplayNameTextByValue(static_cast<int64>(RolledRelicType)).ToString())
		: FString();
	UHudMessageSubsystem::Post(this, FString::Printf(TEXT("Searched: +%d Iron%s"), RolledIron, *RelicText),
		FLinearColor(1.0f, 0.85f, 0.3f), 3.0f);
	OnSearched.Broadcast(this, RolledIron, bRolledRelic, RolledRelicType);
	return true;
}
