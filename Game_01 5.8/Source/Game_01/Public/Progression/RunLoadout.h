// What a run owns and what it is worth: weapon and passive copies, stars, and every passive's effect.
// Tech debt TD-004 (docs/tech-debt-register.md): split out of URunProgressionSubsystem. A plain struct with no UObject,
// so it can be built and tested without a world.
#pragma once

#include "CoreMinimal.h"
#include "Progression/ProgressionTypes.h"

/**
 * The run's loadout: how many copies of each weapon and passive the hero holds (copies are also levels),
 * the star rules, and the numbers each passive adds. The subsystem owns one and passes questions through.
 * Interface rules: a weapon or passive with no copies is not owned; effects are neutral (1.0 or 0.0) when nothing is stacked;
 * copies never go above MaxCopies of the option's row.
 */
struct GAME_01_API FRunLoadout
{
	/** Loaded tuning. LoadDefaults() fills every row; tables may override rows afterwards. */
	TMap<EWeaponType, FWeaponRow> Weapons;
	TMap<EPassiveType, FPassiveRow> Passives;
	FStarTuning Stars;

	/** Pickup pull radius with no Magnet stacks (cm), from the XP curve row. */
	float BaseMagnetRadius = 600.0f;

	/** The built-in numbers for a weapon or passive, used when a data table or row is missing. */
	static FWeaponRow DefaultWeapon(EWeaponType Type);
	static FPassiveRow DefaultPassive(EPassiveType Type);

	/** The data-table row name of an option, which is also its stable id (what a lost film reel unlocks). */
	static const TCHAR* WeaponRowName(EWeaponType Type);
	static const TCHAR* PassiveRowName(EPassiveType Type);

	/** Fills every weapon and passive row with its default. */
	void LoadDefaults();

	/** A fresh run: nothing owned except the starter weapon (one Blade). */
	void ResetOwnership();

	/** Copies held (0 = not owned) and stars (0 = not owned, otherwise 1 to 3). */
	int32 WeaponCopies(EWeaponType Type) const { return WeaponLevels.FindRef(Type); }
	int32 PassiveCopies(EPassiveType Type) const { return PassiveStacks.FindRef(Type); }
	int32 WeaponStars(EWeaponType Type) const { return Stars.StarsForCopies(WeaponCopies(Type)); }
	int32 PassiveStarsFor(EPassiveType Type) const { return Stars.StarsForCopies(PassiveCopies(Type)); }

	/** The most copies an option with this row MaxLevel can hold. */
	int32 MaxCopies(int32 RowMaxLevel) const { return FMath::Max(RowMaxLevel, Stars.CopiesForThreeStar); }

	/** Adds one copy, up to the cap. Returns the copies held afterwards. */
	int32 AddWeaponCopy(EWeaponType Type);
	int32 AddPassiveCopy(EPassiveType Type);

	/** Test and probe helper: sets the copies of a weapon directly (at least 1, at most the cap). */
	void SetWeaponCopies(EWeaponType Type, int32 Copies);

	/** True when the option is not locked, or the camp has unlocked it. */
	bool IsWeaponAvailable(EWeaponType Type, const TArray<FName>& Unlocked) const;
	bool IsPassiveAvailable(EPassiveType Type, const TArray<FName>& Unlocked) const;

	/** How many weapons and passives can appear in an offer given the unlocked options. */
	int32 PoolSize(const TArray<FName>& Unlocked) const;

	/** Passive effects (1.0 or 0.0 when nothing is stacked). */
	float MoveSpeedMultiplier() const;
	float DamageMultiplier() const;
	float MaxHealthBonus() const;
	float XpGainMultiplier() const;
	float AttackSpeedMultiplier() const;
	float MagnetRadius() const;
	float RangeMultiplier() const;
	/** Armor: share of contact damage ignored, 0 to 0.75. */
	float DamageReduction() const;
	float RegenPerSecond() const;
	/** Crit: chance 0 to 0.9, and the damage multiplier of a crit. */
	float CritChance() const;
	float CritMultiplier() const;

	/** Star multipliers for a weapon (1.0 at one star or not owned). */
	float WeaponStarDamageMultiplier(EWeaponType Type) const { return Stars.WeaponDamageMultiplier(WeaponStars(Type)); }
	float WeaponStarRangeMultiplier(EWeaponType Type) const { return Stars.WeaponRangeMultiplier(WeaponStars(Type)); }

private:
	/** A passive's total effect: stacks x per-stack amount x its star multiplier. */
	float PassiveTotal(EPassiveType Passive, float PerStack) const;

	TMap<EWeaponType, int32> WeaponLevels;
	TMap<EPassiveType, int32> PassiveStacks;
};
