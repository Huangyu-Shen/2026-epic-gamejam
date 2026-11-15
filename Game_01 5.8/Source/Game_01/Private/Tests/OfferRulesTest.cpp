// Director's Cut - tech debt TD-004: tests for FOfferRules, the pool, the offer of three, the cut and the choice.
#include "Misc/AutomationTest.h"
#include "Progression/OfferRules.h"
#include "Progression/RunLoadout.h"

#if WITH_DEV_AUTOMATION_TESTS

namespace OfferRulesTest
{
	FRunLoadout MakeLoadout()
	{
		FRunLoadout Loadout;
		Loadout.LoadDefaults();
		Loadout.ResetOwnership();
		return Loadout;
	}


	bool PoolHasWeapon(const TArray<FOfferOption>& Pool, EWeaponType Type)
	{
		return Pool.ContainsByPredicate([Type](const FOfferOption& O) { return O.bIsWeapon && O.Weapon == Type; });
	}

	bool PoolHasPassive(const TArray<FOfferOption>& Pool, EPassiveType Type)
	{
		return Pool.ContainsByPredicate([Type](const FOfferOption& O) { return !O.bIsWeapon && O.Passive == Type; });
	}

	/** Opens an offer with a seeded shuffle and no NEW ids. */
	void Open(FOfferRules& Rules, const FRunLoadout& Loadout, int32 Seed = 7)
	{
		FRandomStream Random(Seed);
		Rules.Generate(Loadout, {}, Random, [] { return TArray<FName>(); });
	}
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FOfferPoolFilterTest, "Game_01.Offer.PoolLeavesOutLockedCutAndMaxedOptions",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FOfferPoolFilterTest::RunTest(const FString& Parameters)
{
	// Arrange: Chain, Haste and Magnet are locked; cut Nova for good; take Bolt to the copy cap
	FRunLoadout Loadout = OfferRulesTest::MakeLoadout();
	FOfferRules Rules;
	Rules.CutWeapons.Add(EWeaponType::Nova);
	for (int32 i = 0; i < 20; ++i)
	{
		Loadout.AddWeaponCopy(EWeaponType::Bolt);
	}

	// Act
	const TArray<FOfferOption> Pool = Rules.BuildPool(Loadout, {});

	// Assert
	TestFalse(TEXT("a locked weapon is not offered"), OfferRulesTest::PoolHasWeapon(Pool, EWeaponType::Chain));
	TestFalse(TEXT("a locked passive is not offered"), OfferRulesTest::PoolHasPassive(Pool, EPassiveType::Haste));
	TestFalse(TEXT("a cut weapon is not offered"), OfferRulesTest::PoolHasWeapon(Pool, EWeaponType::Nova));
	TestFalse(TEXT("a weapon at the copy cap is not offered"), OfferRulesTest::PoolHasWeapon(Pool, EWeaponType::Bolt));
	TestTrue(TEXT("an owned weapon below the cap is offered"), OfferRulesTest::PoolHasWeapon(Pool, EWeaponType::Blade));
	TestEqual(TEXT("13 options minus the cut Nova and the maxed Bolt"), Pool.Num(), 11);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FOfferPoolNewWeaponTest, "Game_01.Offer.AnUnownedWeaponIsMarkedNew",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FOfferPoolNewWeaponTest::RunTest(const FString& Parameters)
{
	// Arrange
	const FRunLoadout Loadout = OfferRulesTest::MakeLoadout();
	const FOfferRules Rules;

	// Act
	const TArray<FOfferOption> Pool = Rules.BuildPool(Loadout, {});

	// Assert
	const FOfferOption* Blade = Pool.FindByPredicate([](const FOfferOption& O) { return O.bIsWeapon && O.Weapon == EWeaponType::Blade; });
	const FOfferOption* Bolt = Pool.FindByPredicate([](const FOfferOption& O) { return O.bIsWeapon && O.Weapon == EWeaponType::Bolt; });
	if (!TestNotNull(TEXT("the starter Blade is in the pool"), Blade) || !TestNotNull(TEXT("Bolt is in the pool"), Bolt))
	{
		return false;
	}
	TestFalse(TEXT("the owned Blade is not a new weapon"), Blade->bIsNewWeapon);
	TestEqual(TEXT("the owned Blade is at level 1"), Blade->CurrentLevel, 1);
	TestTrue(TEXT("Bolt, not owned, is a new weapon"), Bolt->bIsNewWeapon);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FOfferOpensThreeTest, "Game_01.Offer.ThreeOptionsOpenAnOffer",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FOfferOpensThreeTest::RunTest(const FString& Parameters)
{
	// Arrange
	const FRunLoadout Loadout = OfferRulesTest::MakeLoadout();
	FOfferRules Rules;
	Rules.PermanentCutsLeft = 5;
	FRandomStream Random(3);

	// Act
	const FOfferRules::FGenerated Result = Rules.Generate(Loadout, {}, Random, [] { return TArray<FName>(); });

	// Assert
	TestTrue(TEXT("an offer opened"), Result.bOfferOpened);
	TestEqual(TEXT("nothing is applied at once"), Result.ApplyAtOnce.Num(), 0);
	TestTrue(TEXT("the offer is pending"), Rules.bPending);
	TestEqual(TEXT("three options are shown"), Rules.Offer.Num(), 3);
	TestFalse(TEXT("no cut has been used"), Rules.bCutUsed);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FOfferSameSeedTest, "Game_01.Offer.TheSameSeedGivesTheSameOffer",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FOfferSameSeedTest::RunTest(const FString& Parameters)
{
	// Arrange
	const FRunLoadout Loadout = OfferRulesTest::MakeLoadout();
	FOfferRules First;
	FOfferRules Second;

	// Act
	OfferRulesTest::Open(First, Loadout, 11);
	OfferRulesTest::Open(Second, Loadout, 11);

	// Assert
	if (!TestEqual(TEXT("both offers have three options"), First.Offer.Num() + Second.Offer.Num(), 6))
	{
		return false;
	}
	for (int32 i = 0; i < 3; ++i)
	{
		TestEqual(*FString::Printf(TEXT("option %d is the same"), i), FOfferRules::OptionId(First.Offer[i]), FOfferRules::OptionId(Second.Offer[i]));
	}
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FOfferFewOptionsTest, "Game_01.Offer.FewerThanThreeOptionsAreAppliedAtOnce",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FOfferFewOptionsTest::RunTest(const FString& Parameters)
{
	// Arrange: cut everything but Blade and Swiftness out of the pool
	const FRunLoadout Loadout = OfferRulesTest::MakeLoadout();
	FOfferRules Rules;
	for (uint8 W = 0; W < static_cast<uint8>(EWeaponType::Count); ++W)
	{
		if (static_cast<EWeaponType>(W) != EWeaponType::Blade)
		{
			Rules.CutWeapons.Add(static_cast<EWeaponType>(W));
		}
	}
	for (uint8 P = 0; P < static_cast<uint8>(EPassiveType::Count); ++P)
	{
		if (static_cast<EPassiveType>(P) != EPassiveType::Swiftness)
		{
			Rules.CutPassives.Add(static_cast<EPassiveType>(P));
		}
	}
	FRandomStream Random(1);
	int32 TakeNewCalls = 0;

	// Act
	const FOfferRules::FGenerated Result = Rules.Generate(Loadout, {}, Random, [&TakeNewCalls] { ++TakeNewCalls; return TArray<FName>(); });

	// Assert
	TestFalse(TEXT("no offer opened"), Result.bOfferOpened);
	TestFalse(TEXT("nothing is pending"), Rules.bPending);
	TestEqual(TEXT("the two left are applied at once"), Result.ApplyAtOnce.Num(), 2);
	TestEqual(TEXT("the camp's NEW markers were not consumed"), TakeNewCalls, 0);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FOfferNewUnlockInOfferTest, "Game_01.Offer.ANewlyUnlockedOptionIsFirstInTheOffer",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FOfferNewUnlockInOfferTest::RunTest(const FString& Parameters)
{
	// Arrange: Chain is unlocked and is the camp's NEW marker
	const FRunLoadout Loadout = OfferRulesTest::MakeLoadout();
	FOfferRules Rules;
	FRandomStream Random(5);
	int32 TakeNewCalls = 0;

	// Act
	const FOfferRules::FGenerated Result = Rules.Generate(Loadout, { TEXT("Chain") }, Random, [&TakeNewCalls] { ++TakeNewCalls; return TArray<FName>{ TEXT("Chain") }; });

	// Assert
	TestTrue(TEXT("an offer opened"), Result.bOfferOpened);
	TestEqual(TEXT("the NEW markers were consumed once"), TakeNewCalls, 1);
	if (!TestEqual(TEXT("three options"), Rules.Offer.Num(), 3))
	{
		return false;
	}
	TestTrue(TEXT("Chain is the first option"), Rules.Offer[0].bIsWeapon && Rules.Offer[0].Weapon == EWeaponType::Chain);
	TestTrue(TEXT("Chain is marked NEW"), Rules.Offer[0].bIsNewUnlock);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FOfferCutTest, "Game_01.Offer.ACutRemovesTheOptionForTheRunAndSpendsACut",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FOfferCutTest::RunTest(const FString& Parameters)
{
	// Arrange
	const FRunLoadout Loadout = OfferRulesTest::MakeLoadout();
	FOfferRules Rules;
	Rules.PermanentCutsLeft = 2;
	OfferRulesTest::Open(Rules, Loadout);
	const FOfferOption Target = Rules.Offer[1];

	// Act
	FOfferOption Cut;
	const bool bFirst = Rules.Cut(1, Cut);
	const bool bSecondInSameOffer = Rules.Cut(0, Cut);

	// Assert
	TestTrue(TEXT("the first cut is allowed"), bFirst);
	TestFalse(TEXT("a second cut in the same offer is refused"), bSecondInSameOffer);
	TestEqual(TEXT("the cut option is the one at that index"), FOfferRules::OptionId(Cut), FOfferRules::OptionId(Target));
	TestEqual(TEXT("two options stay on the table"), Rules.Offer.Num(), 2);
	TestEqual(TEXT("one cut is spent"), Rules.PermanentCutsLeft, 1);
	TestEqual(TEXT("the option is on the cutting-room floor"), Rules.CuttingRoomFloor.Num(), 1);
	const TArray<FOfferOption> Pool = Rules.BuildPool(Loadout, {});
	TestFalse(TEXT("the cut option is out of the pool"), Target.bIsWeapon ? OfferRulesTest::PoolHasWeapon(Pool, Target.Weapon) : OfferRulesTest::PoolHasPassive(Pool, Target.Passive));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FOfferNoCutsLeftTest, "Game_01.Offer.NoCutIsAllowedWhenNoPermanentCutsAreLeft",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FOfferNoCutsLeftTest::RunTest(const FString& Parameters)
{
	// Arrange
	const FRunLoadout Loadout = OfferRulesTest::MakeLoadout();
	FOfferRules Rules;
	Rules.PermanentCutsLeft = 0;
	OfferRulesTest::Open(Rules, Loadout);

	// Act
	FOfferOption Cut;
	const bool bCut = Rules.Cut(0, Cut);

	// Assert
	TestFalse(TEXT("CanCut is false"), Rules.CanCut());
	TestFalse(TEXT("the cut is refused"), bCut);
	TestEqual(TEXT("all three options stay"), Rules.Offer.Num(), 3);
	TestEqual(TEXT("nothing is on the cutting-room floor"), Rules.CuttingRoomFloor.Num(), 0);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FOfferChooseTest, "Game_01.Offer.ChoosingClosesTheOfferAndKeepsTheOthersInThePool",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FOfferChooseTest::RunTest(const FString& Parameters)
{
	// Arrange
	const FRunLoadout Loadout = OfferRulesTest::MakeLoadout();
	FOfferRules Rules;
	OfferRulesTest::Open(Rules, Loadout);
	const FOfferOption Expected = Rules.Offer[2];
	const int32 PoolBefore = Rules.BuildPool(Loadout, {}).Num();

	// Act
	FOfferOption Chosen;
	const bool bBadIndex = Rules.Choose(5, Chosen);
	const bool bChosen = Rules.Choose(2, Chosen);
	const bool bAgain = Rules.Choose(0, Chosen);

	// Assert
	TestFalse(TEXT("a bad index changes nothing"), bBadIndex);
	TestTrue(TEXT("the choice is taken"), bChosen);
	TestEqual(TEXT("it is the option at that index"), FOfferRules::OptionId(Chosen), FOfferRules::OptionId(Expected));
	TestFalse(TEXT("the offer is closed"), Rules.bPending);
	TestEqual(TEXT("the table is empty"), Rules.Offer.Num(), 0);
	TestFalse(TEXT("choosing again with no offer is refused"), bAgain);
	TestEqual(TEXT("choosing does not take anything out of the pool"), Rules.BuildPool(Loadout, {}).Num(), PoolBefore);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FOfferResetTest, "Game_01.Offer.AFreshRunClearsCutsAndTheOffer",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FOfferResetTest::RunTest(const FString& Parameters)
{
	// Arrange
	const FRunLoadout Loadout = OfferRulesTest::MakeLoadout();
	FOfferRules Rules;
	Rules.PermanentCutsLeft = 3;
	OfferRulesTest::Open(Rules, Loadout);
	FOfferOption Cut;
	Rules.Cut(0, Cut);

	// Act
	Rules.Reset(9);

	// Assert
	TestEqual(TEXT("permanent cuts are the new number"), Rules.PermanentCutsLeft, 9);
	TestEqual(TEXT("the cutting-room floor is empty"), Rules.CuttingRoomFloor.Num(), 0);
	TestEqual(TEXT("no offer is open"), Rules.Offer.Num(), 0);
	TestFalse(TEXT("nothing is pending"), Rules.bPending);
	TestEqual(TEXT("no weapon stays cut"), Rules.CutWeapons.Num() + Rules.CutPassives.Num(), 0);
	return true;
}

#endif
