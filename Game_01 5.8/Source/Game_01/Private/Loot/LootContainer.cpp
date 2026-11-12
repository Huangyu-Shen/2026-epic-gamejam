#include "Loot/LootContainer.h"
#include "Loot/LootBagComponent.h"
#include "Loot/LootConfig.h"
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
		if (bSearched && GEngine)
		{
			GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Silver, TEXT("Already searched"));
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
		bRolledRelic = FMath::FRand() < Row.RelicChance && UseConfig->PickRandomRelic(RolledRelicType);
		bRolled = true;
		if (!bRolledRelic && RolledIron == 0)
		{
			bSearched = true; // empty container
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
	if (GEngine)
	{
		const FString RelicText = bRolledRelic
			? FString::Printf(TEXT(" + relic %s"), *StaticEnum<ERelicType>()->GetDisplayNameTextByValue(static_cast<int64>(RolledRelicType)).ToString())
			: FString();
		GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::Yellow,
			FString::Printf(TEXT("Searched: +%d Iron%s"), RolledIron, *RelicText));
	}
	OnSearched.Broadcast(this, RolledIron, bRolledRelic, RolledRelicType);
	return true;
}
