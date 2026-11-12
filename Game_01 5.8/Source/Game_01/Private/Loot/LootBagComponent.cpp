#include "Loot/LootBagComponent.h"
#include "Loot/LootConfig.h"
#include "Loot/LootHeatSubsystem.h"
#include "Engine/Engine.h"
#include "Engine/World.h"

ULootBagComponent::ULootBagComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.bStartWithTickEnabled = false; // only ticks while buffs are active
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
}

ULootConfig* ULootBagComponent::GetConfig()
{
	if (Config)
	{
		return Config;
	}
	if (!FallbackConfig)
	{
		FallbackConfig = NewObject<ULootConfig>(this, TEXT("FallbackLootConfig"));
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
	return SlotsForIron(IronCount) + Relics.Num();
}

int32 ULootBagComponent::GetCapacitySlots()
{
	return GetConfig()->GetTuning().BagCapacitySlots;
}

bool ULootBagComponent::IsFull()
{
	return GetUsedSlots() >= GetCapacitySlots();
}

EBagAddResult ULootBagComponent::CanAdd(int32 IronAmount, bool bWithRelic)
{
	if (IronAmount < 0 || (IronAmount == 0 && !bWithRelic))
	{
		return EBagAddResult::InvalidItem;
	}
	const int32 SlotsAfter = SlotsForIron(IronCount + IronAmount) + Relics.Num() + (bWithRelic ? 1 : 0);
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

EBagAddResult ULootBagComponent::TryAddLoot(int32 IronAmount, bool bAddRelic, ERelicType RelicType)
{
	const EBagAddResult Result = CanAdd(IronAmount, bAddRelic);
	if (Result != EBagAddResult::Success)
	{
		UE_LOG(LogLoot, Log, TEXT("Pickup refused: %s"), *GetAddResultText(Result).ToString());
		if (GEngine)
		{
			GEngine->AddOnScreenDebugMessage(-1, 2.5f, FColor::Red,
				FString::Printf(TEXT("Pickup refused: %s"), *GetAddResultText(Result).ToString()));
		}
		OnPickupRefused.Broadcast(Result);
		return Result;
	}

	IronCount += IronAmount;
	if (bAddRelic)
	{
		Relics.Add(RelicType);
		// A relic is the rare pickup: it raises heat.
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

bool ULootBagComponent::UseRelic(int32 Index)
{
	if (!Relics.IsValidIndex(Index))
	{
		return false;
	}

	const ERelicType Type = Relics[Index];
	Relics.RemoveAt(Index);

	FRelicRow Row;
	if (GetConfig()->FindRelicRow(Type, Row))
	{
		FActiveRelicBuff Buff;
		Buff.RelicType = Type;
		Buff.Modifier = Row.Buff;
		const UWorld* World = GetWorld();
		Buff.ExpireTime = (Row.Buff.DurationSeconds > 0.0f && World)
			? World->GetTimeSeconds() + Row.Buff.DurationSeconds
			: -1.0;
		ActiveBuffs.Add(Buff);
		SetComponentTickEnabled(true);
	}
	else
	{
		UE_LOG(LogLoot, Warning, TEXT("UseRelic: no data row for relic type %d; no buff applied."), static_cast<int32>(Type));
	}

	if (UWorld* World = GetWorld())
	{
		if (ULootHeatSubsystem* Heat = World->GetSubsystem<ULootHeatSubsystem>())
		{
			Heat->ReduceHeatForRelicUse();
		}
	}

	if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(-1, 2.5f, FColor::Green,
			FString::Printf(TEXT("Used relic: %s"), *StaticEnum<ERelicType>()->GetDisplayNameTextByValue(static_cast<int64>(Type)).ToString()));
	}

	OnRelicUsed.Broadcast(Type);
	OnBagChanged.Broadcast();
	OnBuffsChanged.Broadcast();
	return true;
}

void ULootBagComponent::ClearBag()
{
	const bool bHadBuffs = ActiveBuffs.Num() > 0;
	IronCount = 0;
	Relics.Reset();
	ActiveBuffs.Reset();
	SetComponentTickEnabled(false);
	OnBagChanged.Broadcast();
	if (bHadBuffs)
	{
		OnBuffsChanged.Broadcast();
	}
}

FLootStatModifier ULootBagComponent::GetActiveModifier() const
{
	FLootStatModifier Combined;
	for (const FActiveRelicBuff& Buff : ActiveBuffs)
	{
		Combined.Stack(Buff.Modifier);
	}
	return Combined;
}

void ULootBagComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	const UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}
	const double Now = World->GetTimeSeconds();
	const int32 Removed = ActiveBuffs.RemoveAll([Now](const FActiveRelicBuff& Buff)
	{
		return Buff.ExpireTime >= 0.0 && Now >= Buff.ExpireTime;
	});
	if (Removed > 0)
	{
		OnBuffsChanged.Broadcast();
	}

	// Stop ticking when no timed buff remains.
	const bool bAnyTimed = ActiveBuffs.ContainsByPredicate([](const FActiveRelicBuff& Buff) { return Buff.ExpireTime >= 0.0; });
	if (!bAnyTimed)
	{
		SetComponentTickEnabled(false);
	}
}
