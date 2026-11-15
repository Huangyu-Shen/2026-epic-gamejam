#include "Extraction/CampStockSubsystem.h"
#include "Extraction/ExtractionTypes.h"

void UCampStockSubsystem::AddWood(int32 Amount)
{
	if (Amount <= 0)
	{
		return;
	}
	Wood += Amount;
	UE_LOG(LogExtraction, Log, TEXT("Camp Wood %d (+%d)"), Wood, Amount);
	OnStockChanged.Broadcast();
}

void UCampStockSubsystem::AddStone(int32 Amount)
{
	if (Amount <= 0)
	{
		return;
	}
	Stone += Amount;
	UE_LOG(LogExtraction, Log, TEXT("Camp stone %d (+%d)"), Stone, Amount);
	OnStockChanged.Broadcast();
}

bool UCampStockSubsystem::SpendStone(int32 Amount)
{
	if (Amount <= 0)
	{
		return true;
	}
	if (Stone < Amount)
	{
		return false;
	}
	Stone -= Amount;
	UE_LOG(LogExtraction, Log, TEXT("Camp stone %d (-%d)"), Stone, Amount);
	OnStockChanged.Broadcast();
	return true;
}

bool UCampStockSubsystem::UnlockOption(FName Option)
{
	if (!FReelUnlocks::AddOption(UnlockedOptions, Option))
	{
		return false;
	}
	NewOptions.AddUnique(Option);
	UE_LOG(LogExtraction, Log, TEXT("Upgrade option unlocked: %s (%d unlocked)"), *Option.ToString(), UnlockedOptions.Num());
	OnStockChanged.Broadcast();
	return true;
}

TArray<FName> UCampStockSubsystem::TakeNewOptions()
{
	return MoveTemp(NewOptions);
}

bool UCampStockSubsystem::SpendWood(int32 Amount)
{
	if (Amount <= 0)
	{
		return true;
	}
	if (Wood < Amount)
	{
		return false;
	}
	Wood -= Amount;
	UE_LOG(LogExtraction, Log, TEXT("Camp Wood %d (-%d)"), Wood, Amount);
	OnStockChanged.Broadcast();
	return true;
}

void UCampStockSubsystem::RestoreState(int32 InWood, const TArray<FName>& InUnlockedOptions, const TArray<FName>& InNewOptions, int32 InStone)
{
	Wood = FMath::Max(InWood, 0);
	Stone = FMath::Max(InStone, 0);
	UnlockedOptions.Reset();
	NewOptions.Reset();
	for (const FName Option : InUnlockedOptions)
	{
		FReelUnlocks::AddOption(UnlockedOptions, Option);
	}
	for (const FName Option : InNewOptions)
	{
		if (UnlockedOptions.Contains(Option))
		{
			NewOptions.AddUnique(Option);
		}
	}
	UE_LOG(LogExtraction, Log, TEXT("Camp stock restored: Wood %d, %d unlocked options"), Wood, UnlockedOptions.Num());
	OnStockChanged.Broadcast();
}
