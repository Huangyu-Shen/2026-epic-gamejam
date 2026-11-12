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
