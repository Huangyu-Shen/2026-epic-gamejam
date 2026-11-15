#include "Extraction/ExtractionTypes.h"

DEFINE_LOG_CATEGORY(LogExtraction);

FRunResult FRunSettlement::Compute(int32 BagIron, int32 BagStone, const TArray<ERelicType>& BagRelics, float EarningsMultiplier, ERunOutcome Outcome)
{
	FRunResult Result;
	Result.Outcome = Outcome;
	Result.MultiplierUsed = EarningsMultiplier;

	switch (Outcome)
	{
	case ERunOutcome::Extracted:
		Result.BagIron = BagIron;
		Result.IronGained = FMath::Max(0, FMath::FloorToInt(BagIron * EarningsMultiplier));
		Result.BagStone = BagStone;
		Result.StoneGained = FMath::Max(0, FMath::FloorToInt(BagStone * EarningsMultiplier));
		Result.RelicsBanked = BagRelics;
		break;
	case ERunOutcome::Died:
		Result.BagIron = BagIron;
		Result.IronLost = BagIron;
		Result.BagStone = BagStone;
		Result.StoneLost = BagStone;
		Result.RelicsLost = BagRelics;
		break;
	default:
		break;
	}
	return Result;
}
