#include "Extraction/RunOutcomeSubsystem.h"
#include "Extraction/CampStockSubsystem.h"
#include "Loot/LootBagComponent.h"
#include "Loot/LootHeatSubsystem.h"
#include "Engine/Engine.h"
#include "Engine/GameInstance.h"
#include "Engine/World.h"

/** Fixed on-screen key so the summary replaces itself and StartNewRun can remove it. */
static constexpr uint64 RunResultMessageKey = 1003;

void URunOutcomeSubsystem::SetEarningsMultiplier(float NewMultiplier)
{
	EarningsMultiplier = FMath::Max(0.0f, NewMultiplier);
}

void URunOutcomeSubsystem::ReportExtracted(ULootBagComponent* Bag)
{
	EndRun(Bag, ERunOutcome::Extracted);
}

void URunOutcomeSubsystem::ReportPlayerDied(ULootBagComponent* Bag)
{
	EndRun(Bag, ERunOutcome::Died);
}

void URunOutcomeSubsystem::StartNewRun()
{
	bRunActive = true;
	Outcome = ERunOutcome::None;
	LastResult = FRunResult();
	if (GEngine)
	{
		GEngine->RemoveOnScreenDebugMessage(RunResultMessageKey);
	}
	UE_LOG(LogExtraction, Log, TEXT("New run started."));
}

void URunOutcomeSubsystem::EndRun(ULootBagComponent* Bag, ERunOutcome NewOutcome)
{
	if (!bRunActive)
	{
		return;
	}
	bRunActive = false;
	Outcome = NewOutcome;

	LastResult = FRunSettlement::Compute(Bag->GetIronCount(), Bag->GetRelics(), EarningsMultiplier, NewOutcome);

	if (UCampStockSubsystem* Camp = GetWorld()->GetGameInstance()->GetSubsystem<UCampStockSubsystem>())
	{
		Camp->AddIron(LastResult.IronGained);
		Camp->AddRelics(LastResult.RelicsBanked);
	}
	Bag->ClearBag();
	if (ULootHeatSubsystem* Heat = GetWorld()->GetSubsystem<ULootHeatSubsystem>())
	{
		Heat->ResetHeat();
	}

	const bool bExtracted = NewOutcome == ERunOutcome::Extracted;
	const FString Summary = bExtracted
		? FString::Printf(TEXT("EXTRACTED: bag Iron %d x %.2f = +%d to camp, %d relics banked"),
			LastResult.BagIron, LastResult.MultiplierUsed, LastResult.IronGained, LastResult.RelicsBanked.Num())
		: FString::Printf(TEXT("DIED: lost %d Iron and %d relics"), LastResult.IronLost, LastResult.RelicsLost.Num());
	UE_LOG(LogExtraction, Log, TEXT("%s"), *Summary);
	if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(RunResultMessageKey, 3600.0f, bExtracted ? FColor::Green : FColor::Red, Summary);
	}

	OnRunEnded.Broadcast(LastResult);
}
