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

void UCampStockSubsystem::AddRelics(const TArray<ERelicType>& NewRelics)
{
	if (NewRelics.IsEmpty())
	{
		return;
	}
	WarehouseRelics.Append(NewRelics);
	UE_LOG(LogExtraction, Log, TEXT("Warehouse relics %d (+%d)"), WarehouseRelics.Num(), NewRelics.Num());
	OnStockChanged.Broadcast();
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

void UCampStockSubsystem::RestoreState(int32 InIron, const TArray<ERelicType>& InRelics)
{
	Iron = FMath::Max(InIron, 0);
	WarehouseRelics = InRelics;
	UE_LOG(LogExtraction, Log, TEXT("Camp stock restored: Iron %d, %d relics"), Iron, WarehouseRelics.Num());
	OnStockChanged.Broadcast();
}

bool UCampStockSubsystem::RemoveRelicAt(int32 Index)
{
	if (!WarehouseRelics.IsValidIndex(Index))
	{
		return false;
	}
	WarehouseRelics.RemoveAt(Index);
	UE_LOG(LogExtraction, Log, TEXT("Warehouse relic removed at %d (%d left)"), Index, WarehouseRelics.Num());
	OnStockChanged.Broadcast();
	return true;
}
