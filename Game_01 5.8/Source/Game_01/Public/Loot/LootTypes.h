// Director's Cut - Story 001 (loot, bag, heat), reworked by Story 008 (design/game-brief.md, v2 item 3):
// relics are now "lost film reels". A reel does nothing in a run; carried out alive it unlocks one upgrade option.
#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "LootTypes.generated.h"

DECLARE_LOG_CATEGORY_EXTERN(LogLoot, Log, All);

/**
 * The lost film reel types. A reel cannot be used in a run: carried out alive it unlocks the upgrade option
 * its data row names (FRelicRow::UnlocksOption). The enum name predates the rename and is kept for save compatibility.
 */
UENUM(BlueprintType)
enum class ERelicType : uint8
{
	Swiftstep UMETA(DisplayName = "Fast Cut"),
	Warbrand  UMETA(DisplayName = "Wide Shot"),
	Aegis     UMETA(DisplayName = "Chain Scene")
};

/** Kinds of searchable container. */
UENUM(BlueprintType)
enum class ELootContainerType : uint8
{
	SupplyCrate UMETA(DisplayName = "Supply Crate"),
	Vault       UMETA(DisplayName = "Vault")
};

/** What a container yields and what the camp spends: crates (tree stumps) give wood, vaults (boulders) give stone. */
UENUM(BlueprintType)
enum class ELootResource : uint8
{
	Wood,
	Stone
};

/** Outcome of a bag add. Anything other than Success means nothing was added. */
UENUM(BlueprintType)
enum class EBagAddResult : uint8
{
	Success,
	BagFull,
	InvalidItem
};

/** Data row: definition of one lost film reel type. */
USTRUCT(BlueprintType)
struct GAME_01_API FRelicRow : public FTableRowBase
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Reel")
	ERelicType RelicType = ERelicType::Swiftstep;

	/** Name shown for the reel on the HUD and the result screen. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Reel")
	FText DisplayName;

	/**
	 * Id of the upgrade option this reel unlocks (a weapon or passive row name, e.g. "Haste"). None = the option
	 * does not exist yet, and the reel never drops.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Reel")
	FName UnlocksOption = NAME_None;

	/** Relative chance of being rolled from a container (all zero = uniform). */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Reel", meta = (ClampMin = "0.0"))
	float DropWeight = 1.0f;

	/**
	 * Whether this reel may drop: it unlocks a real option the camp does not have yet, and the bag does not
	 * already hold one of this type.
	 */
	bool IsDroppable(const TArray<FName>& UnlockedOptions, const TArray<ERelicType>& ReelsInBag) const
	{
		return !UnlocksOption.IsNone() && !UnlockedOptions.Contains(UnlocksOption) && !ReelsInBag.Contains(RelicType);
	}
};

/** Pure helpers for the reel-to-option mapping and the unlocked set. They touch no UObjects, so tests need no world. */
struct GAME_01_API FReelUnlocks
{
	/** Adds Option to Unlocked unless it is None or already there. Returns true if it was added. */
	static bool AddOption(TArray<FName>& Unlocked, FName Option)
	{
		if (Option.IsNone() || Unlocked.Contains(Option))
		{
			return false;
		}
		Unlocked.Add(Option);
		return true;
	}

	/** The options these reels unlock, in reel order, without duplicates. Reels with no mapped option are skipped. */
	static TArray<FName> OptionsForReels(const TArray<ERelicType>& Reels, const TArray<FRelicRow>& Rows)
	{
		TArray<FName> Options;
		for (const ERelicType Reel : Reels)
		{
			for (const FRelicRow& Row : Rows)
			{
				if (Row.RelicType == Reel)
				{
					AddOption(Options, Row.UnlocksOption);
					break;
				}
			}
		}
		return Options;
	}
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

	/** The least and the most a container holds, of wood (a crate) or stone (a vault). The columns keep their old names so the data table keeps its values. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Container", meta = (ClampMin = "0"))
	int32 IronMin = 0;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Container", meta = (ClampMin = "0"))
	int32 IronMax = 0;

	/** Chance 0..1 that a search also yields a lost film reel (when one can still drop). */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Container", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float RelicChance = 0.0f;
};

/**
 * Bag and heat tuning. These member initializers are the editable defaults used when no config asset
 * or table overrides them.
 */
USTRUCT(BlueprintType)
struct GAME_01_API FLootTuning : public FTableRowBase
{
	GENERATED_BODY()

	/** Hard bag capacity in slots. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Bag", meta = (ClampMin = "1"))
	int32 BagCapacitySlots = 6;

	/** Wood or stone held per slot (counts are rounded up to whole slots). The column keeps its old name so the data table keeps its value. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Bag", meta = (ClampMin = "1"))
	int32 IronPerSlot = 10;

	/** Heat added each time a lost film reel is picked up. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Heat", meta = (ClampMin = "0.0"))
	float HeatPerRarePickup = 15.0f;

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
