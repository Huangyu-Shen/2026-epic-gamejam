#include "Progression/OfferRules.h"
#include "Progression/RunLoadout.h"

void FOfferRules::Reset(int32 NewPermanentCuts)
{
	Offer.Reset();
	bPending = false;
	bCutUsed = false;
	CutWeapons.Reset();
	CutPassives.Reset();
	CuttingRoomFloor.Reset();
	PermanentCutsLeft = NewPermanentCuts;
}

TArray<FOfferOption> FOfferRules::BuildPool(const FRunLoadout& Loadout, const TArray<FName>& Unlocked) const
{
	TArray<FOfferOption> Pool;
	for (uint8 W = 0; W < static_cast<uint8>(EWeaponType::Count); ++W)
	{
		const EWeaponType Type = static_cast<EWeaponType>(W);
		const int32 Copies = Loadout.WeaponCopies(Type);
		if (Loadout.IsWeaponAvailable(Type, Unlocked) && !CutWeapons.Contains(Type) && Copies < Loadout.MaxCopies(Loadout.Weapons.FindChecked(Type).MaxLevel))
		{
			FOfferOption Opt;
			Opt.bIsWeapon = true;
			Opt.Weapon = Type;
			Opt.bIsNewWeapon = (Copies == 0);
			Opt.CurrentLevel = Copies;
			Pool.Add(Opt);
		}
	}
	for (uint8 P = 0; P < static_cast<uint8>(EPassiveType::Count); ++P)
	{
		const EPassiveType Type = static_cast<EPassiveType>(P);
		const int32 Copies = Loadout.PassiveCopies(Type);
		if (Loadout.IsPassiveAvailable(Type, Unlocked) && !CutPassives.Contains(Type) && Copies < Loadout.MaxCopies(Loadout.Passives.FindChecked(Type).MaxLevel))
		{
			FOfferOption Opt;
			Opt.bIsWeapon = false;
			Opt.Passive = Type;
			Opt.CurrentLevel = Copies;
			Pool.Add(Opt);
		}
	}
	return Pool;
}

FOfferRules::FGenerated FOfferRules::Generate(const FRunLoadout& Loadout, const TArray<FName>& Unlocked, FRandomStream& Random, TFunctionRef<TArray<FName>()> TakeNewIds)
{
	FGenerated Result;
	Offer.Reset();

	TArray<FOfferOption> Pool = BuildPool(Loadout, Unlocked);

	// Fisher-Yates shuffle, then take up to three.
	for (int32 i = Pool.Num() - 1; i > 0; --i)
	{
		Pool.Swap(i, Random.RandRange(0, i));
	}

	// With fewer than three options there is nothing to cut: the player simply gets what is left.
	if (Pool.Num() < 3)
	{
		Result.ApplyAtOnce = MoveTemp(Pool);
		return Result;
	}

	// Options a lost film reel unlocked since the player last saw an offer go first and are marked NEW, so
	// "carry a reel out, see the new option in the next run" always happens in the first offer.
	MoveNewUnlocksToFront(Pool, TakeNewIds());

	for (int32 i = 0; i < 3; ++i)
	{
		Offer.Add(Pool[i]);
	}
	bPending = true;
	bCutUsed = false;
	Result.bOfferOpened = true;
	return Result;
}

bool FOfferRules::CanCut() const
{
	// One cut per level-up, only while permanent cuts remain (that rule keeps the pool from shrinking below an offer).
	return bPending && !bCutUsed && PermanentCutsLeft > 0 && Offer.Num() > 1;
}

bool FOfferRules::Cut(int32 Index, FOfferOption& OutCut)
{
	if (!CanCut() || !Offer.IsValidIndex(Index))
	{
		return false;
	}

	// The cut option leaves the pool for good and goes to the cutting-room floor, where the boss takes it.
	OutCut = Offer[Index];
	CuttingRoomFloor.Add(OutCut);
	--PermanentCutsLeft;
	if (OutCut.bIsWeapon)
	{
		CutWeapons.Add(OutCut.Weapon);
	}
	else
	{
		CutPassives.Add(OutCut.Passive);
	}
	Offer.RemoveAt(Index);
	bCutUsed = true;
	return true;
}

bool FOfferRules::Choose(int32 Index, FOfferOption& OutChosen)
{
	if (!bPending || !Offer.IsValidIndex(Index))
	{
		return false;
	}

	// Take the chosen option. The others stay in the pool and may be offered again; only a cut removes one.
	OutChosen = Offer[Index];
	bPending = false;
	Offer.Reset();
	return true;
}

FName FOfferRules::OptionId(const FOfferOption& Option)
{
	return Option.bIsWeapon ? FName(FRunLoadout::WeaponRowName(Option.Weapon)) : FName(FRunLoadout::PassiveRowName(Option.Passive));
}

void FOfferRules::MoveNewUnlocksToFront(TArray<FOfferOption>& Pool, const TArray<FName>& NewIds)
{
	int32 Front = 0;
	for (int32 i = 0; i < Pool.Num(); ++i)
	{
		if (NewIds.Contains(OptionId(Pool[i])))
		{
			Pool[i].bIsNewUnlock = true;
			Pool.Swap(i, Front++);
		}
	}
}
