#include "Extraction/ExtractionTypes.h"

DEFINE_LOG_CATEGORY(LogExtraction);

FRunResult FRunSettlement::Compute(int32 BagIron, const TArray<ERelicType>& BagRelics, float EarningsMultiplier, ERunOutcome Outcome)
{
	FRunResult Result;
	Result.Outcome = Outcome;
	Result.MultiplierUsed = EarningsMultiplier;

	switch (Outcome)
	{
	case ERunOutcome::Extracted:
		Result.BagIron = BagIron;
		Result.IronGained = FMath::Max(0, FMath::FloorToInt(BagIron * EarningsMultiplier));
		Result.RelicsBanked = BagRelics;
		break;
	case ERunOutcome::Died:
		Result.BagIron = BagIron;
		Result.IronLost = BagIron;
		Result.RelicsLost = BagRelics;
		break;
	default:
		break;
	}
	return Result;
}
