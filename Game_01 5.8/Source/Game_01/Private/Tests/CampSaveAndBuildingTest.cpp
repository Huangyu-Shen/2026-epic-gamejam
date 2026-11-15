// Director's Cut - tech debt TD-011 and TD-014: tests for the save version check and the building cost and bonus math.
#include "Misc/AutomationTest.h"
#include "Camp/CampTypes.h"
#include "Flow/CampSaveGame.h"

#if WITH_DEV_AUTOMATION_TESTS

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FSaveVersionClassifyTest, "Game_01.Save.VersionDecidesWhetherToLoadMigrateOrRefuse",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FSaveVersionClassifyTest::RunTest(const FString& Parameters)
{
	// Arrange: the game writes version 2
	const int32 Current = 2;

	// Act
	const FSaveVersionCheck::EResult SameVersion = FSaveVersionCheck::Classify(2, Current);
	const FSaveVersionCheck::EResult OlderVersion = FSaveVersionCheck::Classify(1, Current);
	const FSaveVersionCheck::EResult NewerVersion = FSaveVersionCheck::Classify(3, Current);

	// Assert
	TestTrue(TEXT("a file of this version loads as it is"), SameVersion == FSaveVersionCheck::EResult::Current);
	TestTrue(TEXT("an older file is migrated"), OlderVersion == FSaveVersionCheck::EResult::NeedsMigration);
	TestTrue(TEXT("a newer file is refused and never overwritten"), NewerVersion == FSaveVersionCheck::EResult::TooNew);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FSaveFileHeaderTest, "Game_01.Save.OnlyAFileThatStartsWithGVASIsHandedToTheLoader",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FSaveFileHeaderTest::RunTest(const FString& Parameters)
{
	// Arrange
	const uint8 Good[] = { 'G', 'V', 'A', 'S', 3, 0, 0, 0 };
	const uint8 Garbage[] = { 'n', 'o', 't', ' ', 'a', ' ', 's', 'a', 'v', 'e' };
	const uint8 Short[] = { 'G', 'V' };

	// Act / Assert
	TestTrue(TEXT("a GVAS header is accepted"), FSaveVersionCheck::LooksLikeSaveFile(Good, UE_ARRAY_COUNT(Good)));
	TestFalse(TEXT("garbage is refused"), FSaveVersionCheck::LooksLikeSaveFile(Garbage, UE_ARRAY_COUNT(Garbage)));
	TestFalse(TEXT("a file shorter than the tag is refused"), FSaveVersionCheck::LooksLikeSaveFile(Short, UE_ARRAY_COUNT(Short)));
	TestFalse(TEXT("no bytes at all is refused"), FSaveVersionCheck::LooksLikeSaveFile(nullptr, 0));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FSaveVersionDefaultTest, "Game_01.Save.TheClassDefaultVersionStaysOneSoOldFilesMigrate",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FSaveVersionDefaultTest::RunTest(const FString& Parameters)
{
	// Arrange / Act: a v1 file stores no SaveVersion and reads back as the class default
	const UCampSaveGame* Defaults = GetDefault<UCampSaveGame>();

	// Assert
	TestEqual(TEXT("the default version is 1"), Defaults->SaveVersion, 1);
	TestTrue(TEXT("a file with the default version is seen as older than the game's"), FSaveVersionCheck::Classify(Defaults->SaveVersion, UCampSaveGame::CurrentSaveVersion) == FSaveVersionCheck::EResult::NeedsMigration);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FBuildingCostTest, "Game_01.Camp.UpgradeCostGrowsByTheGrowthFactorAndRoundsDown",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FBuildingCostTest::RunTest(const FString& Parameters)
{
	// Arrange
	FBuildingRow Doubling;
	Doubling.BaseCost = 10;
	Doubling.CostGrowth = 2.0f;
	FBuildingRow Gentle;
	Gentle.BaseCost = 10;
	Gentle.CostGrowth = 1.5f;

	// Act / Assert
	TestEqual(TEXT("level 0 costs the base"), Doubling.CostForLevel(0), 10);
	TestEqual(TEXT("level 1 doubles"), Doubling.CostForLevel(1), 20);
	TestEqual(TEXT("level 4 is 10 x 2^4"), Doubling.CostForLevel(4), 160);
	TestEqual(TEXT("1.5 growth at level 1 is 15"), Gentle.CostForLevel(1), 15);
	TestEqual(TEXT("1.5 growth at level 2 rounds 22.5 down"), Gentle.CostForLevel(2), 22);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FBuildingBonusTest, "Game_01.Camp.SmithyAndClinicBonusesAreLinearInTheLevel",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FBuildingBonusTest::RunTest(const FString& Parameters)
{
	// Arrange
	FBuildingRow Row;
	Row.DamagePerLevel = 5.0f;
	Row.MaxHealthPerLevel = 25.0f;

	// Act / Assert
	TestEqual(TEXT("no Smithy, no bonus"), Row.DamageBonusForLevel(0), 0.0f);
	TestEqual(TEXT("Smithy level 3"), Row.DamageBonusForLevel(3), 15.0f);
	TestEqual(TEXT("no Clinic, no bonus"), Row.MaxHealthBonusForLevel(0), 0.0f);
	TestEqual(TEXT("Clinic level 2"), Row.MaxHealthBonusForLevel(2), 50.0f);
	return true;
}

#endif
