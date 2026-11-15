// Director's Cut - tech debt TD-011: tests for the run settlement and the NEW-unlock ordering of a level-up offer.
#include "Misc/AutomationTest.h"
#include "Extraction/ExtractionTypes.h"
#include "Progression/OfferRules.h"

#if WITH_DEV_AUTOMATION_TESTS

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FRunSettlementExtractedTest, "Game_01.Extraction.ExtractedBagIsMultipliedAndRoundedDown",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FRunSettlementExtractedTest::RunTest(const FString& Parameters)
{
	// Arrange
	const TArray<ERelicType> Reels = { ERelicType::Swiftstep, ERelicType::Aegis };

	// Act
	const FRunResult Result = FRunSettlement::Compute(25, 7, Reels, 1.5f, ERunOutcome::Extracted);

	// Assert
	TestEqual(TEXT("wood gained is floor(25 x 1.5)"), Result.WoodGained, 37);
	TestEqual(TEXT("stone gained is floor(7 x 1.5)"), Result.StoneGained, 10);
	TestEqual(TEXT("the bag wood is reported before the multiplier"), Result.BagWood, 25);
	TestEqual(TEXT("both reels are carried out"), Result.RelicsBanked.Num(), 2);
	TestEqual(TEXT("nothing is lost"), Result.WoodLost + Result.StoneLost + Result.RelicsLost.Num(), 0);
	TestEqual(TEXT("the multiplier used is reported"), Result.MultiplierUsed, 1.5f);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FRunSettlementDiedTest, "Game_01.Extraction.DeathLosesTheWholeBag",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FRunSettlementDiedTest::RunTest(const FString& Parameters)
{
	// Arrange
	const TArray<ERelicType> Reels = { ERelicType::Warbrand };

	// Act
	const FRunResult Result = FRunSettlement::Compute(25, 7, Reels, 2.0f, ERunOutcome::Died);

	// Assert
	TestEqual(TEXT("nothing is gained"), Result.WoodGained + Result.StoneGained + Result.RelicsBanked.Num(), 0);
	TestEqual(TEXT("all the wood is lost"), Result.WoodLost, 25);
	TestEqual(TEXT("all the stone is lost"), Result.StoneLost, 7);
	TestEqual(TEXT("the reel is lost"), Result.RelicsLost.Num(), 1);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FRunSettlementNoOutcomeTest, "Game_01.Extraction.NoOutcomeSettlesNothing",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FRunSettlementNoOutcomeTest::RunTest(const FString& Parameters)
{
	// Arrange / Act
	const FRunResult Result = FRunSettlement::Compute(25, 7, { ERelicType::Aegis }, 2.0f, ERunOutcome::None);

	// Assert
	TestEqual(TEXT("no wood moves"), Result.WoodGained + Result.WoodLost, 0);
	TestEqual(TEXT("no stone moves"), Result.StoneGained + Result.StoneLost, 0);
	TestEqual(TEXT("no reel moves"), Result.RelicsBanked.Num() + Result.RelicsLost.Num(), 0);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FRunSettlementNegativeMultiplierTest, "Game_01.Extraction.NegativeMultiplierNeverGainsLessThanZero",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FRunSettlementNegativeMultiplierTest::RunTest(const FString& Parameters)
{
	// Arrange / Act
	const FRunResult Result = FRunSettlement::Compute(10, 10, {}, -3.0f, ERunOutcome::Extracted);

	// Assert
	TestEqual(TEXT("wood gained is clamped at zero"), Result.WoodGained, 0);
	TestEqual(TEXT("stone gained is clamped at zero"), Result.StoneGained, 0);
	return true;
}

namespace RunSettlementTest
{
	FOfferOption MakeWeapon(EWeaponType Type)
	{
		FOfferOption Option;
		Option.bIsWeapon = true;
		Option.Weapon = Type;
		return Option;
	}

	FOfferOption MakePassive(EPassiveType Type)
	{
		FOfferOption Option;
		Option.bIsWeapon = false;
		Option.Passive = Type;
		return Option;
	}
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FOfferNewUnlockFirstTest, "Game_01.Progression.NewUnlocksGoFirstAndAreMarked",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FOfferNewUnlockFirstTest::RunTest(const FString& Parameters)
{
	// Arrange: the pool holds three options; the last one is the id a reel unlocked
	TArray<FOfferOption> Pool = { RunSettlementTest::MakeWeapon(EWeaponType::Blade), RunSettlementTest::MakePassive(EPassiveType::Swiftness),
		RunSettlementTest::MakeWeapon(EWeaponType::Chain) };
	const FName ChainId = FOfferRules::OptionId(Pool[2]);

	// Act
	FOfferRules::MoveNewUnlocksToFront(Pool, { ChainId });

	// Assert
	TestEqual(TEXT("the pool keeps its size"), Pool.Num(), 3);
	TestTrue(TEXT("the unlocked option is first"), Pool[0].bIsWeapon && Pool[0].Weapon == EWeaponType::Chain);
	TestTrue(TEXT("the unlocked option is marked NEW"), Pool[0].bIsNewUnlock);
	TestFalse(TEXT("the others are not marked"), Pool[1].bIsNewUnlock || Pool[2].bIsNewUnlock);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FOfferNoNewUnlockTest, "Game_01.Progression.NoNewUnlocksLeavesThePoolAlone",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FOfferNoNewUnlockTest::RunTest(const FString& Parameters)
{
	// Arrange
	TArray<FOfferOption> Pool = { RunSettlementTest::MakeWeapon(EWeaponType::Blade), RunSettlementTest::MakePassive(EPassiveType::Swiftness) };

	// Act
	FOfferRules::MoveNewUnlocksToFront(Pool, {});

	// Assert
	TestTrue(TEXT("the first option is still the blade"), Pool[0].bIsWeapon && Pool[0].Weapon == EWeaponType::Blade);
	TestFalse(TEXT("nothing is marked NEW"), Pool[0].bIsNewUnlock || Pool[1].bIsNewUnlock);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FOfferOptionIdTest, "Game_01.Progression.OptionIdsDifferPerOption",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FOfferOptionIdTest::RunTest(const FString& Parameters)
{
	// Arrange / Act
	const FName BladeId = FOfferRules::OptionId(RunSettlementTest::MakeWeapon(EWeaponType::Blade));
	const FName ChainId = FOfferRules::OptionId(RunSettlementTest::MakeWeapon(EWeaponType::Chain));
	const FName SwiftId = FOfferRules::OptionId(RunSettlementTest::MakePassive(EPassiveType::Swiftness));

	// Assert
	TestNotEqual(TEXT("two weapons have different ids"), BladeId, ChainId);
	TestNotEqual(TEXT("a weapon and a passive have different ids"), BladeId, SwiftId);
	TestFalse(TEXT("an id is never empty"), BladeId.IsNone() || SwiftId.IsNone());
	return true;
}

#endif
