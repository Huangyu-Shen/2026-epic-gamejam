// Director's Cut - tech debt TD-004: characterization tests for FRunLoadout. The expected numbers are worked out by hand
// from the default tuning, so they pin the behaviour the subsystem had before the loadout was split out of it.
#include "Misc/AutomationTest.h"
#include "Progression/RunLoadout.h"

#if WITH_DEV_AUTOMATION_TESTS

namespace RunLoadoutTest
{
	FRunLoadout MakeLoadout()
	{
		FRunLoadout Loadout;
		Loadout.LoadDefaults();
		Loadout.ResetOwnership();
		return Loadout;
	}

	/** Adds Count copies of a passive. */
	void StackPassive(FRunLoadout& Loadout, EPassiveType Type, int32 Count)
	{
		for (int32 i = 0; i < Count; ++i)
		{
			Loadout.AddPassiveCopy(Type);
		}
	}
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FRunLoadoutNeutralTest, "Game_01.Progression.NoPassivesMeansNeutralEffects",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FRunLoadoutNeutralTest::RunTest(const FString& Parameters)
{
	// Arrange
	const FRunLoadout Loadout = RunLoadoutTest::MakeLoadout();

	// Act / Assert
	TestEqual(TEXT("move speed"), Loadout.MoveSpeedMultiplier(), 1.0f);
	TestEqual(TEXT("damage"), Loadout.DamageMultiplier(), 1.0f);
	TestEqual(TEXT("xp gain"), Loadout.XpGainMultiplier(), 1.0f);
	TestEqual(TEXT("attack speed"), Loadout.AttackSpeedMultiplier(), 1.0f);
	TestEqual(TEXT("range"), Loadout.RangeMultiplier(), 1.0f);
	TestEqual(TEXT("max health bonus"), Loadout.MaxHealthBonus(), 0.0f);
	TestEqual(TEXT("regen"), Loadout.RegenPerSecond(), 0.0f);
	TestEqual(TEXT("armor"), Loadout.DamageReduction(), 0.0f);
	TestEqual(TEXT("crit chance"), Loadout.CritChance(), 0.0f);
	TestEqual(TEXT("magnet radius is the base"), Loadout.MagnetRadius(), Loadout.BaseMagnetRadius);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FRunLoadoutStarEffectTest, "Game_01.Progression.PassiveEffectGrowsWithCopiesAndStars",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FRunLoadoutStarEffectTest::RunTest(const FString& Parameters)
{
	// Arrange: Swiftness adds 0.10 per copy; 3 copies make 2 stars (x1.5 on the total), 9 copies make 3 stars (x2.5)
	FRunLoadout One = RunLoadoutTest::MakeLoadout();
	FRunLoadout Three = RunLoadoutTest::MakeLoadout();
	FRunLoadout Nine = RunLoadoutTest::MakeLoadout();

	// Act
	RunLoadoutTest::StackPassive(One, EPassiveType::Swiftness, 1);
	RunLoadoutTest::StackPassive(Three, EPassiveType::Swiftness, 3);
	RunLoadoutTest::StackPassive(Nine, EPassiveType::Swiftness, 9);

	// Assert
	TestEqual(TEXT("one copy: 1 + 1 x 0.10"), One.MoveSpeedMultiplier(), 1.10f, 0.0001f);
	TestEqual(TEXT("three copies: 1 + 3 x 0.10 x 1.5"), Three.MoveSpeedMultiplier(), 1.45f, 0.0001f);
	TestEqual(TEXT("nine copies: 1 + 9 x 0.10 x 2.5"), Nine.MoveSpeedMultiplier(), 3.25f, 0.0001f);
	TestEqual(TEXT("three copies are two stars"), Three.PassiveStarsFor(EPassiveType::Swiftness), 2);
	TestEqual(TEXT("nine copies are three stars"), Nine.PassiveStarsFor(EPassiveType::Swiftness), 3);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FRunLoadoutCapsTest, "Game_01.Progression.ArmorAndCritAreCapped",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FRunLoadoutCapsTest::RunTest(const FString& Parameters)
{
	// Arrange: a row that would give far more than the caps
	FRunLoadout Loadout = RunLoadoutTest::MakeLoadout();
	Loadout.Passives.FindChecked(EPassiveType::Armor).DamageReductionAdd = 1.0f;
	Loadout.Passives.FindChecked(EPassiveType::Crit).CritChanceAdd = 1.0f;

	// Act
	RunLoadoutTest::StackPassive(Loadout, EPassiveType::Armor, 3);
	RunLoadoutTest::StackPassive(Loadout, EPassiveType::Crit, 3);

	// Assert
	TestEqual(TEXT("armor is capped at 0.75"), Loadout.DamageReduction(), 0.75f);
	TestEqual(TEXT("crit chance is capped at 0.9"), Loadout.CritChance(), 0.9f);
	TestEqual(TEXT("crit damage multiplier comes from the row"), Loadout.CritMultiplier(), 2.0f);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FRunLoadoutMagnetTest, "Game_01.Progression.MagnetAddsToTheBaseRadius",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FRunLoadoutMagnetTest::RunTest(const FString& Parameters)
{
	// Arrange: Magnet adds 150 cm per copy
	FRunLoadout Loadout = RunLoadoutTest::MakeLoadout();
	Loadout.BaseMagnetRadius = 600.0f;

	// Act
	RunLoadoutTest::StackPassive(Loadout, EPassiveType::Magnet, 2);

	// Assert
	TestEqual(TEXT("600 + 2 x 150"), Loadout.MagnetRadius(), 900.0f, 0.001f);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FRunLoadoutCopyCapTest, "Game_01.Progression.CopiesNeverPassTheCap",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FRunLoadoutCopyCapTest::RunTest(const FString& Parameters)
{
	// Arrange
	FRunLoadout Loadout = RunLoadoutTest::MakeLoadout();
	const int32 Cap = Loadout.MaxCopies(Loadout.Weapons.FindChecked(EWeaponType::Bolt).MaxLevel);

	// Act
	int32 Held = 0;
	for (int32 i = 0; i < Cap + 5; ++i)
	{
		Held = Loadout.AddWeaponCopy(EWeaponType::Bolt);
	}

	// Assert
	TestEqual(TEXT("the cap is the three-star copy count (9)"), Cap, 9);
	TestEqual(TEXT("extra copies are not added"), Held, Cap);
	TestEqual(TEXT("the weapon is at three stars"), Loadout.WeaponStars(EWeaponType::Bolt), 3);
	TestEqual(TEXT("the starter Blade is owned at one copy"), Loadout.WeaponCopies(EWeaponType::Blade), 1);
	TestEqual(TEXT("an unowned weapon has no stars"), Loadout.WeaponStars(EWeaponType::Nova), 0);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FRunLoadoutPoolTest, "Game_01.Progression.PoolSizeCountsOnlyAvailableOptions",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FRunLoadoutPoolTest::RunTest(const FString& Parameters)
{
	// Arrange: of the 6 weapons and 10 passives, Chain, Haste and Magnet start locked
	const FRunLoadout Loadout = RunLoadoutTest::MakeLoadout();

	// Act
	const int32 NothingUnlocked = Loadout.PoolSize({});
	const int32 ChainUnlocked = Loadout.PoolSize({ TEXT("Chain") });
	const int32 AllUnlocked = Loadout.PoolSize({ TEXT("Chain"), TEXT("Haste"), TEXT("Magnet") });

	// Assert
	TestEqual(TEXT("13 options with nothing unlocked"), NothingUnlocked, 13);
	TestEqual(TEXT("14 once Chain is unlocked"), ChainUnlocked, 14);
	TestEqual(TEXT("16 with everything unlocked"), AllUnlocked, 16);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FRunLoadoutStarWeaponTest, "Game_01.Progression.WeaponStarMultipliersFollowTheStars",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FRunLoadoutStarWeaponTest::RunTest(const FString& Parameters)
{
	// Arrange
	FRunLoadout Loadout = RunLoadoutTest::MakeLoadout();

	// Act
	Loadout.SetWeaponCopies(EWeaponType::Gun, 3);
	const float TwoStarDamage = Loadout.WeaponStarDamageMultiplier(EWeaponType::Gun);
	const float TwoStarRange = Loadout.WeaponStarRangeMultiplier(EWeaponType::Gun);
	Loadout.SetWeaponCopies(EWeaponType::Gun, 9);

	// Assert
	TestEqual(TEXT("two stars: damage x2"), TwoStarDamage, 2.0f);
	TestEqual(TEXT("two stars: range x1.25"), TwoStarRange, 1.25f);
	TestEqual(TEXT("three stars: damage x4"), Loadout.WeaponStarDamageMultiplier(EWeaponType::Gun), 4.0f);
	TestEqual(TEXT("three stars: range x1.6"), Loadout.WeaponStarRangeMultiplier(EWeaponType::Gun), 1.6f);
	TestEqual(TEXT("an unowned weapon is neutral"), Loadout.WeaponStarDamageMultiplier(EWeaponType::Nova), 1.0f);
	return true;
}

#endif
