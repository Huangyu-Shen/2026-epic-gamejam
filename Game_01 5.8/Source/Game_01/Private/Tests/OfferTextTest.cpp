// Director's Cut - tech debt TD-004: tests for FOfferText. The expected strings are written out by hand from the wording the
// level-up panel and the HUD had before the text was split out of the subsystem, so they pin that wording.
#include "Misc/AutomationTest.h"
#include "Progression/OfferText.h"
#include "Progression/RunLoadout.h"

#if WITH_DEV_AUTOMATION_TESTS

namespace OfferTextTest
{
	FOfferOption Weapon(EWeaponType Type, int32 Level, bool bNewWeapon = false, bool bNewUnlock = false)
	{
		FOfferOption Option;
		Option.bIsWeapon = true;
		Option.Weapon = Type;
		Option.CurrentLevel = Level;
		Option.bIsNewWeapon = bNewWeapon;
		Option.bIsNewUnlock = bNewUnlock;
		return Option;
	}

	FOfferOption Passive(EPassiveType Type, int32 Level)
	{
		FOfferOption Option;
		Option.bIsWeapon = false;
		Option.Passive = Type;
		Option.CurrentLevel = Level;
		return Option;
	}
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FOfferTextNameTest, "Game_01.OfferText.OptionNamesAreTheDisplayNames",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FOfferTextNameTest::RunTest(const FString& Parameters)
{
	// Arrange / Act / Assert
	TestEqual(TEXT("a weapon"), FOfferText::OptionName(OfferTextTest::Weapon(EWeaponType::Bolt, 0)), FString(TEXT("Bolt")));
	TestEqual(TEXT("a passive"), FOfferText::OptionName(OfferTextTest::Passive(EPassiveType::Swiftness, 0)), FString(TEXT("Swiftness")));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FOfferTextDescribeTest, "Game_01.OfferText.DescriptionShowsTheLevelStepAndTheStarNote",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FOfferTextDescribeTest::RunTest(const FString& Parameters)
{
	// Arrange: default stars, 3 copies make 2 stars and 9 make 3
	const FStarTuning Stars;

	// Act / Assert
	TestEqual(TEXT("a new weapon"), FOfferText::Describe(OfferTextTest::Weapon(EWeaponType::Gun, 0, true), Stars), FString(TEXT("NEW weapon: Gun")));
	TestEqual(TEXT("a weapon a reel unlocked"), FOfferText::Describe(OfferTextTest::Weapon(EWeaponType::Chain, 0, true, true), Stars), FString(TEXT("NEW! Chain  (unlocked weapon)")));
	TestEqual(TEXT("one copy to the next star"), FOfferText::Describe(OfferTextTest::Weapon(EWeaponType::Bolt, 1), Stars), FString(TEXT("Bolt  Lv1 -> Lv2   (1 more to 2-STAR)")));
	TestEqual(TEXT("the copy that makes a star"), FOfferText::Describe(OfferTextTest::Weapon(EWeaponType::Bolt, 2), Stars), FString(TEXT("Bolt  Lv2 -> Lv3   >> 2-STAR!")));
	TestEqual(TEXT("past the second star"), FOfferText::Describe(OfferTextTest::Passive(EPassiveType::Might, 3), Stars), FString(TEXT("Might  Lv3 -> Lv4   (5 more to 3-STAR)")));
	TestEqual(TEXT("at three stars there is no note"), FOfferText::Describe(OfferTextTest::Passive(EPassiveType::Might, 9), Stars), FString(TEXT("Might  Lv9 -> Lv10")));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FOfferTextCardTest, "Game_01.OfferText.CardsShowTitleDetailAndNote",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FOfferTextCardTest::RunTest(const FString& Parameters)
{
	// Arrange
	const FStarTuning Stars;

	// Act
	const FOfferCardText NewWeapon = FOfferText::Card(OfferTextTest::Weapon(EWeaponType::Orbit, 0, true), Stars);
	const FOfferCardText Unlocked = FOfferText::Card(OfferTextTest::Weapon(EWeaponType::Chain, 0, true, true), Stars);
	const FOfferCardText StarUp = FOfferText::Card(OfferTextTest::Weapon(EWeaponType::Bolt, 2), Stars);
	const FOfferCardText Plain = FOfferText::Card(OfferTextTest::Passive(EPassiveType::Greed, 1), Stars);

	// Assert
	TestEqual(TEXT("new weapon title"), NewWeapon.Title, FString(TEXT("Orbit")));
	TestEqual(TEXT("new weapon detail"), NewWeapon.Detail, FString(TEXT("New weapon")));
	TestFalse(TEXT("a new weapon does not glow"), NewWeapon.bHighlight);
	TestEqual(TEXT("unlocked weapon title"), Unlocked.Title, FString(TEXT("NEW!  Chain")));
	TestEqual(TEXT("unlocked weapon detail"), Unlocked.Detail, FString(TEXT("Unlocked weapon")));
	TestTrue(TEXT("an unlocked weapon glows"), Unlocked.bHighlight);
	TestEqual(TEXT("star-up detail"), StarUp.Detail, FString(TEXT("Lv2  >  Lv3")));
	TestEqual(TEXT("star-up note"), StarUp.Note, FString(TEXT("2-STAR UP!")));
	TestTrue(TEXT("a star-up glows"), StarUp.bHighlight);
	TestEqual(TEXT("plain card note counts down to the star"), Plain.Note, FString(TEXT("1 more to 2-STAR")));
	TestFalse(TEXT("a plain card does not glow"), Plain.bHighlight);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FOfferTextLoadoutTest, "Game_01.OfferText.LoadoutLineHasOneAsteriskPerStar",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FOfferTextLoadoutTest::RunTest(const FString& Parameters)
{
	// Arrange: the starter Blade, Bolt at 3 copies (2 stars), Might at 1 copy
	FRunLoadout Loadout;
	Loadout.LoadDefaults();
	Loadout.ResetOwnership();
	Loadout.SetWeaponCopies(EWeaponType::Bolt, 3);
	Loadout.AddPassiveCopy(EPassiveType::Might);

	// Act
	const FString Line = FOfferText::LoadoutSummary(Loadout);

	// Assert
	TestEqual(TEXT("weapons first, then passives, three spaces between"), Line, FString(TEXT("Blade Lv1*   Bolt Lv3**   Might Lv1*")));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FOfferTextEmptyLoadoutTest, "Game_01.OfferText.AnEmptyLoadoutIsAnEmptyLine",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FOfferTextEmptyLoadoutTest::RunTest(const FString& Parameters)
{
	// Arrange: nothing owned (no ResetOwnership, so not even the starter weapon)
	FRunLoadout Loadout;
	Loadout.LoadDefaults();

	// Act / Assert
	TestEqual(TEXT("empty"), FOfferText::LoadoutSummary(Loadout), FString());
	return true;
}

#endif
