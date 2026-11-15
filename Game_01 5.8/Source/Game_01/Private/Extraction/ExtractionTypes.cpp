#include "Extraction/ExtractionTypes.h"

DEFINE_LOG_CATEGORY(LogExtraction);

FRunResult FRunSettlement::Compute(int32 BagWood, int32 BagStone, const TArray<ERelicType>& BagRelics, float EarningsMultiplier, ERunOutcome Outcome)
{
	FRunResult Result;
	Result.Outcome = Outcome;
	Result.MultiplierUsed = EarningsMultiplier;

	switch (Outcome)
	{
	case ERunOutcome::Extracted:
		Result.BagWood = BagWood;
		Result.WoodGained = FMath::Max(0, FMath::FloorToInt(BagWood * EarningsMultiplier));
		Result.BagStone = BagStone;
		Result.StoneGained = FMath::Max(0, FMath::FloorToInt(BagStone * EarningsMultiplier));
		Result.RelicsBanked = BagRelics;
		break;
	case ERunOutcome::Died:
		Result.BagWood = BagWood;
		Result.WoodLost = BagWood;
		Result.BagStone = BagStone;
		Result.StoneLost = BagStone;
		Result.RelicsLost = BagRelics;
		break;
	default:
		break;
	}
	return Result;
}
