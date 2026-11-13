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
