// Director's Cut - story 008 (design/game-brief.md, v2 item 3): lost film reels unlock upgrade options.
#include "Misc/AutomationTest.h"
#include "Loot/LootTypes.h"
#include "Loot/LootConfig.h"
#include "Flow/CampSaveGame.h"

#if WITH_DEV_AUTOMATION_TESTS

namespace ReelUnlockTest
{
	FRelicRow MakeRow(ERelicType Type, FName Option)
	{
		FRelicRow Row;
		Row.RelicType = Type;
		Row.UnlocksOption = Option;
		return Row;
	}

	/** Three reel types; the third has no option yet. */
	TArray<FRelicRow> MakeRows()
	{
		return { MakeRow(ERelicType::Swiftstep, TEXT("Haste")), MakeRow(ERelicType::Warbrand, TEXT("Magnet")),
			MakeRow(ERelicType::Aegis, NAME_None) };
	}
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FReelUnlockSetTest, "Game_01.Loot.UnlockedSetAddsOnceAndSkipsNone",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FReelUnlockSetTest::RunTest(const FString& Parameters)
{
	// Arrange
	TArray<FName> Unlocked;

	// Act / Assert
	TestTrue(TEXT("a new option is added"), FReelUnlocks::AddOption(Unlocked, TEXT("Haste")));
	TestFalse(TEXT("the same option twice is refused"), FReelUnlocks::AddOption(Unlocked, TEXT("Haste")));
	TestFalse(TEXT("None is never added"), FReelUnlocks::AddOption(Unlocked, NAME_None));
	TestEqual(TEXT("only one option is held"), Unlocked.Num(), 1);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FReelUnlockOptionsForReelsTest, "Game_01.Loot.ReelsMapToTheirOptions",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FReelUnlockOptionsForReelsTest::RunTest(const FString& Parameters)
{
	// Arrange
	const TArray<FRelicRow> Rows = ReelUnlockTest::MakeRows();

	// Act
	const TArray<FName> Both = FReelUnlocks::OptionsForReels({ ERelicType::Swiftstep, ERelicType::Warbrand }, Rows);
	const TArray<FName> Twice = FReelUnlocks::OptionsForReels({ ERelicType::Swiftstep, ERelicType::Swiftstep }, Rows);
	const TArray<FName> Unmapped = FReelUnlocks::OptionsForReels({ ERelicType::Aegis }, Rows);

	// Assert
	TestEqual(TEXT("two reels unlock two options"), Both.Num(), 2);
	TestEqual(TEXT("in reel order"), Both[0], FName(TEXT("Haste")));
	TestEqual(TEXT("two reels of one type unlock one option"), Twice.Num(), 1);
	TestEqual(TEXT("a reel with no option unlocks nothing"), Unmapped.Num(), 0);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FReelDropRulesTest, "Game_01.Loot.OnlyUsefulReelsDrop",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FReelDropRulesTest::RunTest(const FString& Parameters)
{
	// Arrange
	const TArray<FRelicRow> Rows = ReelUnlockTest::MakeRows();
	const FRelicRow& Haste = Rows[0];
	const FRelicRow& NoOption = Rows[2];

	// Act / Assert
	TestTrue(TEXT("a reel for a locked option drops"), Haste.IsDroppable({}, {}));
	TestFalse(TEXT("a reel whose option is unlocked does not drop"), Haste.IsDroppable({ TEXT("Haste") }, {}));
	TestFalse(TEXT("a reel type already in the bag does not drop again"), Haste.IsDroppable({}, { ERelicType::Swiftstep }));
	TestFalse(TEXT("a reel with no option never drops"), NoOption.IsDroppable({}, {}));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FReelPickRandomTest, "Game_01.Loot.PickRandomSkipsUselessReels",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FReelPickRandomTest::RunTest(const FString& Parameters)
{
	// Arrange: a config whose rows are the test rows (no tables, so it needs no world).
	ULootConfig* Config = NewObject<ULootConfig>(GetTransientPackage());
	Config->DefaultRelics = ReelUnlockTest::MakeRows();
	ERelicType Picked = ERelicType::Aegis;

	// Act / Assert: with Haste unlocked and a Magnet reel in the bag, nothing can drop.
	TestFalse(TEXT("nothing drops when every mapped option is unlocked or carried"),
		Config->PickRandomRelic({ TEXT("Haste") }, { ERelicType::Warbrand }, Picked));
	// With nothing unlocked, only the two mapped reels can drop, never the unmapped one.
	bool bUnmappedDropped = false;
	for (int32 Roll = 0; Roll < 200; ++Roll)
	{
		if (Config->PickRandomRelic({}, {}, Picked) && Picked == ERelicType::Aegis)
		{
			bUnmappedDropped = true;
		}
	}
	TestFalse(TEXT("the reel with no option never drops"), bUnmappedDropped);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FReelSaveMigrationTest, "Game_01.Loot.V1SaveRelicsBecomeUnlockedOptions",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FReelSaveMigrationTest::RunTest(const FString& Parameters)
{
	// Arrange: a v1 save holding a duplicate relic and one with no option.
	const TArray<ERelicType> V1Warehouse = { ERelicType::Swiftstep, ERelicType::Swiftstep, ERelicType::Aegis };

	// Act: the same call the save subsystem makes when SaveVersion < 2.
	const TArray<FName> Migrated = FReelUnlocks::OptionsForReels(V1Warehouse, ReelUnlockTest::MakeRows());

	// Assert
	TestEqual(TEXT("duplicates collapse and unmapped relics are dropped"), Migrated.Num(), 1);
	TestEqual(TEXT("the surviving option is the mapped one"), Migrated[0], FName(TEXT("Haste")));
	// A save file stores only values that differ from the class default, so an old file has no SaveVersion and reads
	// back as the default. That default must be 1, or a v1 file would look like v2 and skip the migration.
	const UCampSaveGame* Unmarked = NewObject<UCampSaveGame>(GetTransientPackage());
	TestEqual(TEXT("a save with no stored version reads as version 1"), Unmarked->SaveVersion, 1);
	TestEqual(TEXT("the game writes version 2"), UCampSaveGame::CurrentSaveVersion, 2);

	return true;
}

#endif // WITH_DEV_AUTOMATION_TESTS
