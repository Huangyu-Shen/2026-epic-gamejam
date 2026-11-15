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

void UCampStockSubsystem::RestoreState(int32 InIron, const TArray<FName>& InUnlockedOptions, const TArray<FName>& InNewOptions, int32 InStone)
{
	Iron = FMath::Max(InIron, 0);
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
	UE_LOG(LogExtraction, Log, TEXT("Camp stock restored: Iron %d, %d unlocked options"), Iron, UnlockedOptions.Num());
	OnStockChanged.Broadcast();
}
