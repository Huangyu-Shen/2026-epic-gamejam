#include "Progression/OfferText.h"
#include "Progression/RunLoadout.h"
#include "UObject/Class.h"

FString FOfferText::OptionName(const FOfferOption& Option)
{
	return Option.bIsWeapon
		? StaticEnum<EWeaponType>()->GetDisplayNameTextByValue(static_cast<int64>(Option.Weapon)).ToString()
		: StaticEnum<EPassiveType>()->GetDisplayNameTextByValue(static_cast<int64>(Option.Passive)).ToString();
}

FString FOfferText::Describe(const FOfferOption& Option, const FStarTuning& Stars)
{
	const FString Name = OptionName(Option);
	const FString NewMark = Option.bIsNewUnlock ? FString(TEXT("NEW! ")) : FString();
	if (Option.bIsWeapon && Option.bIsNewWeapon)
	{
		// An unlocked weapon is both new to the camp and new to the run: say it once.
		return Option.bIsNewUnlock ? FString::Printf(TEXT("NEW! %s  (unlocked weapon)"), *Name)
		                           : FString::Printf(TEXT("NEW weapon: %s"), *Name);
	}

	// What one more copy does to the star: a star-up, or how far the next one is.
	const int32 After = Option.CurrentLevel + 1;
	const int32 StarsAfter = Stars.StarsForCopies(After);
	FString StarNote;
	if (StarsAfter > Stars.StarsForCopies(Option.CurrentLevel) && Option.CurrentLevel > 0)
	{
		StarNote = FString::Printf(TEXT("   >> %d-STAR!"), StarsAfter);
	}
	else if (StarsAfter < 3)
	{
		StarNote = FString::Printf(TEXT("   (%d more to %d-STAR)"), Stars.CopiesToNextStar(After), StarsAfter + 1);
	}
	return FString::Printf(TEXT("%s%s  Lv%d -> Lv%d%s"), *NewMark, *Name, Option.CurrentLevel, After, *StarNote);
}

FOfferCardText FOfferText::Card(const FOfferOption& Option, const FStarTuning& Stars)
{
	FOfferCardText Card;
	Card.Title = (Option.bIsNewUnlock ? FString(TEXT("NEW!  ")) : FString()) + OptionName(Option);
	Card.bHighlight = Option.bIsNewUnlock;
	if (Option.bIsWeapon && Option.bIsNewWeapon)
	{
		Card.Detail = Option.bIsNewUnlock ? FString(TEXT("Unlocked weapon")) : FString(TEXT("New weapon"));
		return Card;
	}

	const int32 After = Option.CurrentLevel + 1;
	const int32 StarsAfter = Stars.StarsForCopies(After);
	Card.Detail = FString::Printf(TEXT("Lv%d  >  Lv%d"), Option.CurrentLevel, After);
	if (StarsAfter > Stars.StarsForCopies(Option.CurrentLevel) && Option.CurrentLevel > 0)
	{
		Card.Note = FString::Printf(TEXT("%d-STAR UP!"), StarsAfter);
		Card.bHighlight = true;
	}
	else if (StarsAfter < 3)
	{
		Card.Note = FString::Printf(TEXT("%d more to %d-STAR"), Stars.CopiesToNextStar(After), StarsAfter + 1);
	}
	return Card;
}

FString FOfferText::LoadoutSummary(const FRunLoadout& Loadout)
{
	// "Blade Lv3**   Might Lv1*": one asterisk per star.
	FString Out;
	auto Append = [&Out, &Loadout](const FString& Name, int32 Copies)
	{
		if (Copies > 0)
		{
			Out += FString::Printf(TEXT("%s%s Lv%d%s"), Out.IsEmpty() ? TEXT("") : TEXT("   "), *Name, Copies,
				*FString::ChrN(Loadout.Stars.StarsForCopies(Copies), TEXT('*')));
		}
	};
	for (uint8 W = 0; W < static_cast<uint8>(EWeaponType::Count); ++W)
	{
		Append(StaticEnum<EWeaponType>()->GetDisplayNameTextByValue(W).ToString(), Loadout.WeaponCopies(static_cast<EWeaponType>(W)));
	}
	for (uint8 P = 0; P < static_cast<uint8>(EPassiveType::Count); ++P)
	{
		Append(StaticEnum<EPassiveType>()->GetDisplayNameTextByValue(P).ToString(), Loadout.PassiveCopies(static_cast<EPassiveType>(P)));
	}
	return Out;
}
