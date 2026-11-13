// Director's Cut - Story 004 (design/game-brief.md, MVP feature 2: in-run growth).
#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "ProgressionTypes.generated.h"

/** The three weapons, each with a distinct auto-attack pattern. */
UENUM(BlueprintType)
enum class EWeaponType : uint8
{
	Blade UMETA(DisplayName = "Blade"),   // nearest single target
	Bolt  UMETA(DisplayName = "Bolt"),    // piercing line
	Nova  UMETA(DisplayName = "Nova"),    // radial burst
	Count UMETA(Hidden),
};

/** The four passives; each applies its effect immediately on pick. */
UENUM(BlueprintType)
enum class EPassiveType : uint8
{
	Swiftness UMETA(DisplayName = "Swiftness"), // +move speed
	Might     UMETA(DisplayName = "Might"),     // +damage
	Vitality  UMETA(DisplayName = "Vitality"),  // +max health
	Greed     UMETA(DisplayName = "Greed"),     // +XP gain
	Count     UMETA(Hidden),
};

/** XP curve. Usable as a DataTable row; the initializers are the fallback. */
USTRUCT(BlueprintType)
struct GAME_01_API FXpCurveRow : public FTableRowBase
{
	GENERATED_BODY()

	/** XP needed to go from level 1 to 2. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Progression", meta = (ClampMin = "1.0"))
	float BaseXp = 30.0f;

	/** Multiplier on the requirement for each further level. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Progression", meta = (ClampMin = "1.0"))
	float Growth = 1.3f;

	/** XP one enemy drop is worth (before the Greed multiplier). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Progression", meta = (ClampMin = "0.0"))
	float XpPerKill = 5.0f;
};

/** One weapon's stats and level cap. Usable as a DataTable row; defaults are the Blade. */
USTRUCT(BlueprintType)
struct GAME_01_API FWeaponRow : public FTableRowBase
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon")
	EWeaponType WeaponType = EWeaponType::Blade;

	/** Damage at level 1. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon", meta = (ClampMin = "0.0"))
	float Damage = 15.0f;

	/** Extra damage added for every level above 1. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon", meta = (ClampMin = "0.0"))
	float DamagePerLevel = 8.0f;

	/** Seconds between attacks. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon", meta = (ClampMin = "0.05"))
	float Interval = 0.4f;

	/** Blade: reach to the target; Bolt: line length; Nova: radius (cm). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon", meta = (ClampMin = "0.0"))
	float Range = 600.0f;

	/** Bolt only: half-width (cm) of the piercing line. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon", meta = (ClampMin = "0.0"))
	float LineHalfWidth = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon", meta = (ClampMin = "1"))
	int32 MaxLevel = 5;
};

/** One passive's per-stack effect and cap. Usable as a DataTable row. */
USTRUCT(BlueprintType)
struct GAME_01_API FPassiveRow : public FTableRowBase
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Passive")
	EPassiveType PassiveType = EPassiveType::Swiftness;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Passive", meta = (ClampMin = "1"))
	int32 MaxLevel = 5;

	/** Swiftness: added to the move-speed multiplier per stack. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Passive")
	float MoveSpeedAdd = 0.0f;

	/** Might: added to the damage multiplier per stack. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Passive")
	float DamageAdd = 0.0f;

	/** Vitality: flat max-health added (and healed) per stack. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Passive", meta = (ClampMin = "0.0"))
	float MaxHealthAdd = 0.0f;

	/** Greed: added to the XP-gain multiplier per stack. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Passive")
	float XpGainAdd = 0.0f;
};

/**
 * Rules for "cut one, keep two" and the three-into-one star merge (design/game-brief.md v2, MVP items 1 and 2).
 * Every pick adds one copy of a weapon or passive. Copies still count as its level; reaching the thresholds
 * below also raises its star, which multiplies its effect on top of the per-level growth.
 * Pure functions with no world access, so they are unit tested directly.
 */
USTRUCT(BlueprintType)
struct GAME_01_API FStarTuning
{
	GENERATED_BODY()

	/** Copies needed for two stars (three one-star copies merge into one). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stars", meta = (ClampMin = "2"))
	int32 CopiesForTwoStar = 3;

	/** Copies needed for three stars (three two-star sets). Also the most copies one item can hold. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stars", meta = (ClampMin = "3"))
	int32 CopiesForThreeStar = 9;

	/** Weapon damage multiplier at two and three stars. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stars", meta = (ClampMin = "1.0"))
	float WeaponDamageTwoStar = 2.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stars", meta = (ClampMin = "1.0"))
	float WeaponDamageThreeStar = 4.0f;

	/** Weapon reach multiplier (range, line width, radius) at two and three stars: the visible part of a star-up. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stars", meta = (ClampMin = "1.0"))
	float WeaponRangeTwoStar = 1.25f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stars", meta = (ClampMin = "1.0"))
	float WeaponRangeThreeStar = 1.6f;

	/** Multiplier on a passive's total effect at two and three stars. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stars", meta = (ClampMin = "1.0"))
	float PassiveTwoStar = 1.5f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stars", meta = (ClampMin = "1.0"))
	float PassiveThreeStar = 2.5f;

	/** Options that always stay in the level-up pool; a cut is only permanent while the pool is larger than this. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cut", meta = (ClampMin = "3"))
	int32 MinPoolSize = 4;

	/** 0 for an item the run does not own, otherwise 1 to 3. */
	int32 StarsForCopies(int32 Copies) const
	{
		if (Copies <= 0)
		{
			return 0;
		}
		return Copies >= CopiesForThreeStar ? 3 : (Copies >= CopiesForTwoStar ? 2 : 1);
	}

	/** Copies still missing for the next star; 0 at three stars. */
	int32 CopiesToNextStar(int32 Copies) const
	{
		if (Copies >= CopiesForThreeStar)
		{
			return 0;
		}
		return (Copies >= CopiesForTwoStar ? CopiesForThreeStar : CopiesForTwoStar) - FMath::Max(Copies, 0);
	}

	float WeaponDamageMultiplier(int32 Stars) const { return Stars >= 3 ? WeaponDamageThreeStar : (Stars == 2 ? WeaponDamageTwoStar : 1.0f); }
	float WeaponRangeMultiplier(int32 Stars) const { return Stars >= 3 ? WeaponRangeThreeStar : (Stars == 2 ? WeaponRangeTwoStar : 1.0f); }
	float PassiveMultiplier(int32 Stars) const { return Stars >= 3 ? PassiveThreeStar : (Stars == 2 ? PassiveTwoStar : 1.0f); }

	/** How many cuts in one run remove an option for good: the pool never shrinks below MinPoolSize. */
	int32 PermanentCutsForPool(int32 PoolSize) const { return FMath::Max(PoolSize - MinPoolSize, 0); }
};
