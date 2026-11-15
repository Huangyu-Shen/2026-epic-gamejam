// Director's Cut - story 009 (design/game-brief.md, v2 item 4): options that only a lost film reel unlocks.
#include "Misc/AutomationTest.h"
#include "Progression/ProgressionTypes.h"

#if WITH_DEV_AUTOMATION_TESTS

namespace UnlockableOptionsTest
{
	/** The shipped pool: thirteen options from the start, three that start locked. */
	struct FOption
	{
		FName Id;
		bool bStartsLocked;
	};

	TArray<FOption> MakePool()
	{
		return {
			{ TEXT("Blade"), false }, { TEXT("Bolt"), false }, { TEXT("Nova"), false }, { TEXT("Gun"), false }, { TEXT("Orbit"), false },
			{ TEXT("Swiftness"), false }, { TEXT("Might"), false }, { TEXT("Vitality"), false }, { TEXT("Greed"), false },
			{ TEXT("Reach"), false }, { TEXT("Armor"), false }, { TEXT("Regen"), false }, { TEXT("Crit"), false },
			{ TEXT("Chain"), true }, { TEXT("Haste"), true }, { TEXT("Magnet"), true } };
	}

	int32 CountAvailable(const TArray<FName>& Unlocked)
	{
		int32 Count = 0;
		for (const FOption& Option : MakePool())
		{
			Count += FOptionUnlock::IsAvailable(Option.bStartsLocked, Option.Id, Unlocked) ? 1 : 0;
		}
		return Count;
	}
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FLockedOptionIsNeverOfferedTest, "Game_01.Progression.LockedOptionIsNeverOfferedUntilUnlocked",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FLockedOptionIsNeverOfferedTest::RunTest(const FString& Parameters)
{
	// Arrange / Act / Assert
	TestFalse(TEXT("a locked option is unavailable on a new save"), FOptionUnlock::IsAvailable(true, TEXT("Haste"), {}));
	TestFalse(TEXT("unlocking a different option does not help"), FOptionUnlock::IsAvailable(true, TEXT("Haste"), { TEXT("Chain") }));
	TestTrue(TEXT("a locked option is available once unlocked"), FOptionUnlock::IsAvailable(true, TEXT("Haste"), { TEXT("Haste") }));
	TestTrue(TEXT("an option that never starts locked is always available"), FOptionUnlock::IsAvailable(false, TEXT("Blade"), {}));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FPoolGrowsWithUnlocksTest, "Game_01.Progression.PoolAndPermanentCutsGrowWithEachUnlock",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FPoolGrowsWithUnlocksTest::RunTest(const FString& Parameters)
{
	// Arrange
	const FStarTuning Tuning;

	// Act
	const int32 PoolNone = UnlockableOptionsTest::CountAvailable({});
	const int32 PoolOne = UnlockableOptionsTest::CountAvailable({ TEXT("Haste") });
	const int32 PoolAll = UnlockableOptionsTest::CountAvailable({ TEXT("Haste"), TEXT("Magnet"), TEXT("Chain") });

	// Assert: 13 options and 9 permanent cuts at the start; every unlock adds one option and one cut.
	TestEqual(TEXT("pool with no unlock"), PoolNone, 13);
	TestEqual(TEXT("permanent cuts with no unlock"), Tuning.PermanentCutsForPool(PoolNone), 9);
	TestEqual(TEXT("pool with one unlock"), PoolOne, 14);
	TestEqual(TEXT("permanent cuts with one unlock"), Tuning.PermanentCutsForPool(PoolOne), 10);
	TestEqual(TEXT("pool with all three unlocked"), PoolAll, 16);
	TestEqual(TEXT("permanent cuts with all three unlocked"), Tuning.PermanentCutsForPool(PoolAll), 12);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FUnknownUnlockedIdIsIgnoredTest, "Game_01.Progression.AnUnlockedIdThatMatchesNoOptionChangesNothing",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FUnknownUnlockedIdIsIgnoredTest::RunTest(const FString& Parameters)
{
	// Arrange / Act: a save may hold an id from a newer build or a renamed option.
	const int32 Pool = UnlockableOptionsTest::CountAvailable({ TEXT("Nonexistent"), TEXT("haste_typo") });

	// Assert
	TestEqual(TEXT("the pool stays at the starting thirteen"), Pool, 13);
	return true;
}

#endif // WITH_DEV_AUTOMATION_TESTS
