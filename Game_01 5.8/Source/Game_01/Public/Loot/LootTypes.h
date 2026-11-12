// Director's Cut - Story 001 (design/game-brief.md, MVP feature 3: loot, bag, heat).
#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "LootTypes.generated.h"

DECLARE_LOG_CATEGORY_EXTERN(LogLoot, Log, All);

/** The three relic types. Each can be kept in the bag or used on the spot for its buff. */
UENUM(BlueprintType)
enum class ERelicType : uint8
{
	Swiftstep UMETA(DisplayName = "Swiftstep"),	// move speed buff
	Warbrand  UMETA(DisplayName = "Warbrand"),	// damage dealt buff
	Aegis     UMETA(DisplayName = "Aegis")		// damage taken reduction buff
};

/** Kinds of searchable container. */
UENUM(BlueprintType)
enum class ELootContainerType : uint8
{
	SupplyCrate UMETA(DisplayName = "Supply Crate"),
	Vault       UMETA(DisplayName = "Vault")
};

/** Outcome of a bag add. Anything other than Success means nothing was added. */
UENUM(BlueprintType)
enum class EBagAddResult : uint8
{
	Success,
	BagFull,
	InvalidItem
};

/**
 * Plain stat modifier a pawn can read (see ULootBagComponent::GetActiveModifier).
 * Multipliers of 1.0 are neutral. Values live in data rows, not code.
 */
USTRUCT(BlueprintType)
struct GAME_01_API FLootStatModifier
{
	GENERATED_BODY()

	/** Multiplier on movement speed. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Loot")
	float MoveSpeedMultiplier = 1.0f;

	/** Multiplier on damage dealt. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Loot")
	float DamageMultiplier = 1.0f;

	/** Multiplier on damage taken (below 1 = tougher). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Loot")
	float DamageTakenMultiplier = 1.0f;

	/** Seconds the buff lasts after use. 0 or less = lasts until the bag/run is reset. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Loot")
	float DurationSeconds = 0.0f;

	/** Multiplies the three multipliers of Other into this one (duration is ignored). */
	void Stack(const FLootStatModifier& Other)
	{
		MoveSpeedMultiplier *= Other.MoveSpeedMultiplier;
		DamageMultiplier *= Other.DamageMultiplier;
		DamageTakenMultiplier *= Other.DamageTakenMultiplier;
	}
};

/** Data row: definition of one relic type. */
USTRUCT(BlueprintType)
struct GAME_01_API FRelicRow : public FTableRowBase
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Relic")
	ERelicType RelicType = ERelicType::Swiftstep;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Relic")
	FText DisplayName;

	/** Buff applied when the relic is used. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Relic")
	FLootStatModifier Buff;

	/** Relative chance of being rolled from a container (all zero = uniform). */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Relic", meta = (ClampMin = "0.0"))
	float DropWeight = 1.0f;
};

/** Data row: what one kind of container yields per search. */
USTRUCT(BlueprintType)
struct GAME_01_API FLootContainerRow : public FTableRowBase
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Container")
	ELootContainerType ContainerType = ELootContainerType::SupplyCrate;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Container")
	FText DisplayName;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Container", meta = (ClampMin = "0"))
	int32 IronMin = 0;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Container", meta = (ClampMin = "0"))
	int32 IronMax = 0;

	/** Chance 0..1 that a search also yields a relic. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Container", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float RelicChance = 0.0f;
};

/**
 * Bag / heat / relic-use tuning. These member initializers are the editable
 * defaults used when no config asset or table overrides them.
 */
USTRUCT(BlueprintType)
struct GAME_01_API FLootTuning : public FTableRowBase
{
	GENERATED_BODY()

	/** Hard bag capacity in slots. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Bag", meta = (ClampMin = "1"))
	int32 BagCapacitySlots = 6;

	/** Iron held per slot (iron count is rounded up to whole slots). */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Bag", meta = (ClampMin = "1"))
	int32 IronPerSlot = 10;

	/** Heat added each time a rare item (relic) is picked up. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Heat", meta = (ClampMin = "0.0"))
	float HeatPerRarePickup = 15.0f;

	/** Heat removed when a relic is used. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Heat", meta = (ClampMin = "0.0"))
	float HeatReductionOnRelicUse = 8.0f;

	/** Heat ceiling. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Heat", meta = (ClampMin = "1.0"))
	float MaxHeat = 100.0f;

	/** Spawn pressure multiplier at zero heat (hook for story 003). */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Heat", meta = (ClampMin = "0.0"))
	float SpawnPressureAtZeroHeat = 1.0f;

	/** Spawn pressure multiplier at max heat (hook for story 003). */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Heat", meta = (ClampMin = "0.0"))
	float SpawnPressureAtMaxHeat = 3.0f;
};
