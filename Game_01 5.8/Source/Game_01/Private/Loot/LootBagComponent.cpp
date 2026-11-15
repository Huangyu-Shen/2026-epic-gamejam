#include "Loot/LootBagComponent.h"
#include "Loot/LootConfig.h"
#include "Loot/LootHeatSubsystem.h"
#include "UI/HudMessageSubsystem.h"
#include "Engine/Engine.h"
#include "Engine/World.h"

ULootBagComponent::ULootBagComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void ULootBagComponent::BeginPlay()
{
	Super::BeginPlay();
	if (UWorld* World = GetWorld())
	{
		if (ULootHeatSubsystem* Heat = World->GetSubsystem<ULootHeatSubsystem>())
		{
			Heat->SetTuning(GetConfig()->GetTuning());
		}
	}
	// A missing table falls back to the C++ defaults without an error, so print where the numbers came from
	// (read this line in a packaged build's log).
	const FLootTuning Tuning = GetConfig()->GetTuning();
	UE_LOG(LogLoot, Log, TEXT("LOOTCFG source=%s capacity=%d ironPerSlot=%d heatPerReel=%.1f maxHeat=%.1f pressure=%.2f..%.2f reelRows=%d"),
		GetConfig()->IsTuningFromTable() ? TEXT("table") : TEXT("c++ defaults"), Tuning.BagCapacitySlots, Tuning.IronPerSlot,
		Tuning.HeatPerRarePickup, Tuning.MaxHeat, Tuning.SpawnPressureAtZeroHeat, Tuning.SpawnPressureAtMaxHeat,
		GetConfig()->GetRelicRows().Num());
}

ULootConfig* ULootBagComponent::GetConfig()
{
	if (Config)
	{
		return Config;
	}
	if (!FallbackConfig)
	{
		FallbackConfig = ULootConfig::CreateFromSettings(this);
	}
	return FallbackConfig;
}

int32 ULootBagComponent::SlotsForIron(int32 Iron)
{
	const int32 PerSlot = FMath::Max(1, GetConfig()->GetTuning().IronPerSlot);
	return FMath::DivideAndRoundUp(FMath::Max(0, Iron), PerSlot);
}

int32 ULootBagComponent::GetUsedSlots()
{
	return SlotsForIron(IronCount) + SlotsForIron(StoneCount) + Relics.Num();
}

int32 ULootBagComponent::GetCapacitySlots()
{
	return GetConfig()->GetTuning().BagCapacitySlots;
}

bool ULootBagComponent::IsFull()
{
	return GetUsedSlots() >= GetCapacitySlots();
}

EBagAddResult ULootBagComponent::CanAdd(int32 IronAmount, bool bWithRelic, ELootResource Resource)
{
	if (IronAmount < 0 || (IronAmount == 0 && !bWithRelic))
	{
		return EBagAddResult::InvalidItem;
	}
	const bool bStone = Resource == ELootResource::Stone;
	const int32 SlotsAfter = SlotsForIron(IronCount + (bStone ? 0 : IronAmount)) + SlotsForIron(StoneCount + (bStone ? IronAmount : 0)) + Relics.Num() + (bWithRelic ? 1 : 0);
	return SlotsAfter > GetCapacitySlots() ? EBagAddResult::BagFull : EBagAddResult::Success;
}

FText ULootBagComponent::GetAddResultText(EBagAddResult Result)
{
	switch (Result)
	{
	case EBagAddResult::Success:     return FText::FromString(TEXT("Added"));
	case EBagAddResult::BagFull:     return FText::FromString(TEXT("Bag is full"));
	case EBagAddResult::InvalidItem: return FText::FromString(TEXT("Nothing to add"));
	}
	return FText::GetEmpty();
}

EBagAddResult ULootBagComponent::TryAddLoot(int32 IronAmount, bool bAddRelic, ERelicType RelicType, ELootResource Resource)
{
	const EBagAddResult Result = CanAdd(IronAmount, bAddRelic, Resource);
	if (Result != EBagAddResult::Success)
	{
		UE_LOG(LogLoot, Log, TEXT("Pickup refused: %s"), *GetAddResultText(Result).ToString());
		UHudMessageSubsystem::Post(this, FString::Printf(TEXT("Pickup refused: %s"), *GetAddResultText(Result).ToString()),
			FLinearColor(0.95f, 0.35f, 0.3f));
		OnPickupRefused.Broadcast(Result);
		return Result;
	}

	(Resource == ELootResource::Stone ? StoneCount : IronCount) += IronAmount;
	if (bAddRelic)
	{
		Relics.Add(RelicType);
		// A lost film reel is the rare pickup: it raises heat.
		if (UWorld* World = GetWorld())
		{
			if (ULootHeatSubsystem* Heat = World->GetSubsystem<ULootHeatSubsystem>())
			{
				Heat->AddRarePickupHeat();
			}
		}
	}
	OnBagChanged.Broadcast();
	return Result;
}

void ULootBagComponent::ClearBag()
{
	IronCount = 0;
	StoneCount = 0;
	Relics.Reset();
	OnBagChanged.Broadcast();
}
