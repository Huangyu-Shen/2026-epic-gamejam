// The words of a level-up offer: option names, the one-line description, the card text, and the loadout line of the HUD.
// Tech debt TD-004 (docs/tech-debt-register.md): split out of URunProgressionSubsystem. Plain functions with no UObject
// state, so the wording can be tested without a world.
#pragma once

#include "CoreMinimal.h"
#include "Progression/ProgressionTypes.h"

struct FRunLoadout;

/** What one level-up card shows. */
struct FOfferCardText
{
	/** The option's name, with "NEW!" when a lost film reel just unlocked it. */
	FString Title;
	/** Level change, or "New weapon". */
	FString Detail;
	/** Star progress, e.g. "2 more to 2-STAR" or "2-STAR UP!". Empty when maxed. */
	FString Note;
	/** True for a star-up or a newly unlocked option, so the card can glow. */
	bool bHighlight = false;
};

/** All the wording of the level-up offer in one place. Every function is pure. */
struct GAME_01_API FOfferText
{
	/** The option's display name, for example "Bolt" or "Swiftness". */
	static FString OptionName(const FOfferOption& Option);

	/** One line for the level-up panel, for example "Bolt  Lv2 -> Lv3   (1 more to 2-STAR)". */
	static FString Describe(const FOfferOption& Option, const FStarTuning& Stars);

	/** The three pieces of one level-up card. */
	static FOfferCardText Card(const FOfferOption& Option, const FStarTuning& Stars);

	/** One line for the HUD: every owned weapon and passive with its level and one asterisk per star. */
	static FString LoadoutSummary(const FRunLoadout& Loadout);
};
