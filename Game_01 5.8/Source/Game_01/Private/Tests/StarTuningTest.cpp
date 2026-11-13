// Director's Cut - brief v2, MVP items 1 and 2: star-merge thresholds, star multipliers and the permanent-cut budget.
#include "Misc/AutomationTest.h"
#include "Progression/ProgressionTypes.h"

#if WITH_DEV_AUTOMATION_TESTS

namespace StarTuningTest
{
	/** The shipped defaults: 3 copies for two stars, 9 for three, pool never cut below 4. */
	FStarTuning MakeDefaults()
	{
		return FStarTuning();
	}
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FStarTuningStarsForCopiesTest, "Game_01.Progression.StarsFollowCopyThresholds",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FStarTuningStarsForCopiesTest::RunTest(const FString& Parameters)
{
	// Arrange
	const FStarTuning Tuning = StarTuningTest::MakeDefaults();

	// Act / Assert: the boundaries on both sides of each threshold.
	TestEqual(TEXT("not owned has no star"), Tuning.StarsForCopies(0), 0);
	TestEqual(TEXT("first copy is one star"), Tuning.StarsForCopies(1), 1);
	TestEqual(TEXT("one below the two-star threshold"), Tuning.StarsForCopies(Tuning.CopiesForTwoStar - 1), 1);
	TestEqual(TEXT("at the two-star threshold"), Tuning.StarsForCopies(Tuning.CopiesForTwoStar), 2);
	TestEqual(TEXT("one below the three-star threshold"), Tuning.StarsForCopies(Tuning.CopiesForThreeStar - 1), 2);
	TestEqual(TEXT("at the three-star threshold"), Tuning.StarsForCopies(Tuning.CopiesForThreeStar), 3);
	TestEqual(TEXT("beyond the cap stays three stars"), Tuning.StarsForCopies(Tuning.CopiesForThreeStar + 5), 3);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FStarTuningCopiesToNextStarTest, "Game_01.Progression.CopiesToNextStarCountsDown",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FStarTuningCopiesToNextStarTest::RunTest(const FString& Parameters)
{
	// Arrange
	const FStarTuning Tuning = StarTuningTest::MakeDefaults();

	// Act / Assert
	TestEqual(TEXT("one copy needs the rest of the first merge"), Tuning.CopiesToNextStar(1), Tuning.CopiesForTwoStar - 1);
	TestEqual(TEXT("just merged to two stars counts toward three"), Tuning.CopiesToNextStar(Tuning.CopiesForTwoStar),
		Tuning.CopiesForThreeStar - Tuning.CopiesForTwoStar);
	TestEqual(TEXT("one short of three stars"), Tuning.CopiesToNextStar(Tuning.CopiesForThreeStar - 1), 1);
	TestEqual(TEXT("three stars need nothing more"), Tuning.CopiesToNextStar(Tuning.CopiesForThreeStar), 0);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FStarTuningMultipliersTest, "Game_01.Progression.StarMultipliersGrowWithStars",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FStarTuningMultipliersTest::RunTest(const FString& Parameters)
{
	// Arrange
	const FStarTuning Tuning = StarTuningTest::MakeDefaults();

	// Act / Assert: one star is neutral, and each star is at least as strong as the one before.
	TestEqual(TEXT("one-star weapon damage is neutral"), Tuning.WeaponDamageMultiplier(1), 1.0f);
	TestEqual(TEXT("one-star weapon range is neutral"), Tuning.WeaponRangeMultiplier(1), 1.0f);
	TestEqual(TEXT("one-star passive is neutral"), Tuning.PassiveMultiplier(1), 1.0f);
	TestEqual(TEXT("two-star weapon damage uses its setting"), Tuning.WeaponDamageMultiplier(2), Tuning.WeaponDamageTwoStar);
	TestEqual(TEXT("three-star weapon damage uses its setting"), Tuning.WeaponDamageMultiplier(3), Tuning.WeaponDamageThreeStar);
	TestTrue(TEXT("three stars hit harder than two"), Tuning.WeaponDamageMultiplier(3) > Tuning.WeaponDamageMultiplier(2));
	TestTrue(TEXT("three stars reach further than two"), Tuning.WeaponRangeMultiplier(3) > Tuning.WeaponRangeMultiplier(2));
	TestTrue(TEXT("three-star passive beats two-star"), Tuning.PassiveMultiplier(3) > Tuning.PassiveMultiplier(2));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FStarTuningPermanentCutsTest, "Game_01.Progression.PermanentCutsKeepTheMinimumPool",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FStarTuningPermanentCutsTest::RunTest(const FString& Parameters)
{
	// Arrange
	const FStarTuning Tuning = StarTuningTest::MakeDefaults();

	// Act / Assert: cuts = pool - minimum, never negative.
	TestEqual(TEXT("a pool at the minimum allows no permanent cut"), Tuning.PermanentCutsForPool(Tuning.MinPoolSize), 0);
	TestEqual(TEXT("a pool below the minimum allows no permanent cut"), Tuning.PermanentCutsForPool(Tuning.MinPoolSize - 2), 0);
	TestEqual(TEXT("each extra option adds one permanent cut"), Tuning.PermanentCutsForPool(Tuning.MinPoolSize + 3), 3);
	return true;
}

#endif // WITH_DEV_AUTOMATION_TESTS
