// The rules of a level-up offer: which options may appear, the three that are shown, the cut, and the choice.
// Tech debt TD-004 (docs/tech-debt-register.md): split out of URunProgressionSubsystem. A plain struct with no UObject
// and no sound or text, so it can be tested without a world.
#pragma once

#include "CoreMinimal.h"
#include "Math/RandomStream.h"
#include "Progression/ProgressionTypes.h"

struct FRunLoadout;

/**
 * One run's offer state and the rules that change it.
 *
 * Interface rules:
 * - The pool is every option that is unlocked, not cut for good, and below the copy cap.
 * - With fewer than three options in the pool there is no offer: Generate returns them to be applied at once.
 * - Otherwise Generate shows three of them, options a lost film reel just unlocked first and marked NEW.
 * - One cut per offer, and only while permanent cuts are left; a cut option leaves the pool for the run and goes to
 *   the cutting-room floor in the order it was cut.
 * - Choose closes the offer; the options not chosen stay in the pool.
 */
struct GAME_01_API FOfferRules
{
	/** The options on the table; empty when no offer is open. */
	TArray<FOfferOption> Offer;
	bool bPending = false;
	bool bCutUsed = false;

	/** Options cut for good this run, by kind. */
	TSet<EWeaponType> CutWeapons;
	TSet<EPassiveType> CutPassives;

	/** Cuts still left that remove the cut option from the pool for good. */
	int32 PermanentCutsLeft = 0;

	/** Everything cut this run, oldest first (kept for the boss). */
	TArray<FOfferOption> CuttingRoomFloor;

	/** What Generate decided. */
	struct FGenerated
	{
		/** Fewer than three options were left: these are to be applied at once and no offer was opened. */
		TArray<FOfferOption> ApplyAtOnce;
		/** True when an offer of three is now open. */
		bool bOfferOpened = false;
	};

	/** A fresh run: nothing cut, no offer open, this many permanent cuts. */
	void Reset(int32 NewPermanentCuts);

	/** The options that may be offered now, in loadout order (not shuffled). */
	TArray<FOfferOption> BuildPool(const FRunLoadout& Loadout, const TArray<FName>& Unlocked) const;

	/**
	 * Builds the pool, shuffles it with Random and opens an offer of three, or returns the few that are left.
	 * TakeNewIds is called only when an offer opens (it consumes the camp's NEW markers); it returns the ids to mark NEW.
	 */
	FGenerated Generate(const FRunLoadout& Loadout, const TArray<FName>& Unlocked, FRandomStream& Random, TFunctionRef<TArray<FName>()> TakeNewIds);

	/** One cut per offer, only while permanent cuts remain and another option would stay on the table. */
	bool CanCut() const;

	/** Cuts offer entry Index. Returns false (and changes nothing) when a cut is not allowed or the index is bad. */
	bool Cut(int32 Index, FOfferOption& OutCut);

	/** Takes offer entry Index and closes the offer. Returns false when no offer is open or the index is bad. */
	bool Choose(int32 Index, FOfferOption& OutChosen);

	/** The option's stable id: its data-table row name, which is also what a lost film reel unlocks. */
	static FName OptionId(const FOfferOption& Option);

	/**
	 * Marks every option whose id is in NewIds as a NEW unlock and moves those options to the front of the pool
	 * (their relative order is not kept).
	 */
	static void MoveNewUnlocksToFront(TArray<FOfferOption>& Pool, const TArray<FName>& NewIds);
};
