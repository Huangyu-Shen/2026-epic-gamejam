#include "Extraction/RunOutcomeSubsystem.h"
#include "Audio/GameAudioSubsystem.h"
#include "Loot/LootConfig.h"
#include "Extraction/CampStockSubsystem.h"
#include "Flow/CampSaveSubsystem.h"
#include "Loot/LootBagComponent.h"
#include "Loot/LootHeatSubsystem.h"
#include "Engine/Engine.h"
#include "Engine/GameInstance.h"
#include "Engine/World.h"

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

	LastResult = FRunSettlement::Compute(Bag->GetWoodCount(), Bag->GetStoneCount(), Bag->GetRelics(), EarningsMultiplier, NewOutcome);

	if (UCampStockSubsystem* Camp = GetWorld()->GetGameInstance()->GetSubsystem<UCampStockSubsystem>())
	{
		Camp->AddIron(LastResult.IronGained);
		Camp->AddStone(LastResult.StoneGained);
		// Carrying a lost film reel out alive unlocks the option it names, for every later run.
		if (NewOutcome == ERunOutcome::Extracted)
		{
			const TArray<FRelicRow> Rows = Bag->GetConfig()->GetRelicRows();
			for (const FName Option : FReelUnlocks::OptionsForReels(LastResult.RelicsBanked, Rows))
			{
				if (Camp->UnlockOption(Option))
				{
					LastResult.OptionsUnlocked.Add(Option);
				}
			}
		}
	}
	// One save slot, written after every run (the stock listener fires only when the stock changed).
	if (UCampSaveSubsystem* Save = GetWorld()->GetGameInstance()->GetSubsystem<UCampSaveSubsystem>())
	{
		Save->SaveNow();
	}
	Bag->ClearBag();
	if (ULootHeatSubsystem* Heat = GetWorld()->GetSubsystem<ULootHeatSubsystem>())
	{
		Heat->ResetHeat();
	}

	const bool bExtracted = NewOutcome == ERunOutcome::Extracted;
	const FString Summary = bExtracted
		? FString::Printf(TEXT("EXTRACTED: bag wood %d stone %d x %.2f = +%d wood +%d stone to camp, %d lost reels carried out, %d options unlocked"),
			LastResult.BagIron, LastResult.BagStone, LastResult.MultiplierUsed, LastResult.IronGained, LastResult.StoneGained, LastResult.RelicsBanked.Num(), LastResult.OptionsUnlocked.Num())
		: FString::Printf(TEXT("DIED: lost %d wood, %d stone and %d lost reels"), LastResult.IronLost, LastResult.StoneLost, LastResult.RelicsLost.Num());
	UE_LOG(LogExtraction, Log, TEXT("%s"), *Summary);

	UGameAudioSubsystem::PlayIn(this, bExtracted ? EGameSound::Extraction : EGameSound::PlayerDeath);
	OnRunEnded.Broadcast(LastResult);
}
