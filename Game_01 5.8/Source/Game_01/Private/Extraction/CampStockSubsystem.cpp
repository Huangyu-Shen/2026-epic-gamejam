#include "Extraction/CampStockSubsystem.h"
#include "Extraction/ExtractionTypes.h"

void UCampStockSubsystem::AddIron(int32 Amount)
{
	if (Amount <= 0)
	{
		return;
	}
	Iron += Amount;
	UE_LOG(LogExtraction, Log, TEXT("Camp Iron %d (+%d)"), Iron, Amount);
	OnStockChanged.Broadcast();
}

bool UCampStockSubsystem::UnlockOption(FName Option)
{
	if (!FReelUnlocks::AddOption(UnlockedOptions, Option))
	{
		return false;
	}
	UE_LOG(LogExtraction, Log, TEXT("Upgrade option unlocked: %s (%d unlocked)"), *Option.ToString(), UnlockedOptions.Num());
	OnStockChanged.Broadcast();
	return true;
}

bool UCampStockSubsystem::SpendIron(int32 Amount)
{
	if (Amount <= 0)
	{
		return true;
	}
	if (Iron < Amount)
	{
		return false;
	}
	Iron -= Amount;
	UE_LOG(LogExtraction, Log, TEXT("Camp Iron %d (-%d)"), Iron, Amount);
	OnStockChanged.Broadcast();
	return true;
}

void UCampStockSubsystem::RestoreState(int32 InIron, const TArray<FName>& InUnlockedOptions)
{
	Iron = FMath::Max(InIron, 0);
	UnlockedOptions.Reset();
	for (const FName Option : InUnlockedOptions)
	{
		FReelUnlocks::AddOption(UnlockedOptions, Option);
	}
	UE_LOG(LogExtraction, Log, TEXT("Camp stock restored: Iron %d, %d unlocked options"), Iron, UnlockedOptions.Num());
	OnStockChanged.Broadcast();
}
