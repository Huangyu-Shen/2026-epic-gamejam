#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Loot/LootTypes.h"
#include "LootBagComponent.generated.h"

class ULootConfig;

/** A relic buff currently in effect. */
USTRUCT(BlueprintType)
struct GAME_01_API FActiveRelicBuff
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "Loot")
	ERelicType RelicType = ERelicType::Swiftstep;

	UPROPERTY(BlueprintReadOnly, Category = "Loot")
	FLootStatModifier Modifier;

	/** World time (seconds) when the buff ends; negative = until reset. */
	UPROPERTY(BlueprintReadOnly, Category = "Loot")
	double ExpireTime = -1.0;
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnBagChanged);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnBagPickupRefused, EBagAddResult, Reason);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnRelicUsed, ERelicType, RelicType);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnBuffsChanged);

/**
 * The player's run bag: Iron plus relics under a hard slot capacity.
 * Iron occupies ceil(Iron / IronPerSlot) slots, each relic one slot.
 * Adding a relic raises heat; using a relic applies its buff and lowers heat;
 * keeping it leaves heat as is. Story 002 reads/clears/settles via the
 * read-only getters and ClearBag().
 * Implements design/game-brief.md MVP feature 3.
 */
UCLASS(ClassGroup = (Loot), meta = (BlueprintSpawnableComponent))
class GAME_01_API ULootBagComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	ULootBagComponent();

	/** Config asset. If unset, a transient config with built-in defaults is used. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Loot")
	TObjectPtr<ULootConfig> Config;

	UPROPERTY(BlueprintAssignable, Category = "Loot")
	FOnBagChanged OnBagChanged;

	/** Fired when an add was refused (e.g. bag full). */
	UPROPERTY(BlueprintAssignable, Category = "Loot")
	FOnBagPickupRefused OnPickupRefused;

	UPROPERTY(BlueprintAssignable, Category = "Loot")
	FOnRelicUsed OnRelicUsed;

	/** Fired when active buffs are added or expire. */
	UPROPERTY(BlueprintAssignable, Category = "Loot")
	FOnBuffsChanged OnBuffsChanged;

	/** Effective config (never null). */
	UFUNCTION(BlueprintPure, Category = "Loot")
	ULootConfig* GetConfig();

	/** Checks whether the loot would fit without adding it. */
	UFUNCTION(BlueprintPure, Category = "Loot")
	EBagAddResult CanAdd(int32 IronAmount, bool bWithRelic);

	/**
	 * Atomically adds Iron and optionally one relic. On failure nothing is added,
	 * OnPickupRefused fires and a debug message is shown. A relic raises heat.
	 */
	UFUNCTION(BlueprintCallable, Category = "Loot")
	EBagAddResult TryAddLoot(int32 IronAmount, bool bAddRelic, ERelicType RelicType);

	UFUNCTION(BlueprintCallable, Category = "Loot")
	EBagAddResult TryAddIron(int32 IronAmount) { return TryAddLoot(IronAmount, false, ERelicType::Swiftstep); }

	UFUNCTION(BlueprintCallable, Category = "Loot")
	EBagAddResult TryAddRelic(ERelicType RelicType) { return TryAddLoot(0, true, RelicType); }

	/** USE: removes the relic at Index, applies its buff and lowers heat. False if the index is invalid. */
	UFUNCTION(BlueprintCallable, Category = "Loot")
	bool UseRelic(int32 Index);

	/** Empties the bag, drops active buffs. For story 002 (death/settlement). Does not touch heat. */
	UFUNCTION(BlueprintCallable, Category = "Loot")
	void ClearBag();

	// ---- Read-only contents ----

	UFUNCTION(BlueprintPure, Category = "Loot")
	int32 GetIronCount() const { return IronCount; }

	/** Relics currently kept in the bag (copy). */
	UFUNCTION(BlueprintPure, Category = "Loot")
	TArray<ERelicType> GetRelics() const { return Relics; }

	UFUNCTION(BlueprintPure, Category = "Loot")
	int32 GetRelicCount() const { return Relics.Num(); }

	UFUNCTION(BlueprintPure, Category = "Loot")
	int32 GetUsedSlots();

	UFUNCTION(BlueprintPure, Category = "Loot")
	int32 GetCapacitySlots();

	UFUNCTION(BlueprintPure, Category = "Loot")
	bool IsFull();

	/** Combined modifier of all active buffs; pawns read this (multipliers are 1.0 when none). */
	UFUNCTION(BlueprintPure, Category = "Loot")
	FLootStatModifier GetActiveModifier() const;

	UFUNCTION(BlueprintPure, Category = "Loot")
	TArray<FActiveRelicBuff> GetActiveBuffs() const { return ActiveBuffs; }

	/** Human-readable text for a result, for debug/HUD. */
	UFUNCTION(BlueprintPure, Category = "Loot")
	static FText GetAddResultText(EBagAddResult Result);

	virtual void BeginPlay() override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

private:
	int32 SlotsForIron(int32 Iron);

	UPROPERTY(Transient)
	TObjectPtr<ULootConfig> FallbackConfig;

	UPROPERTY(VisibleInstanceOnly, Transient, Category = "Loot", meta = (AllowPrivateAccess = "true"))
	int32 IronCount = 0;

	UPROPERTY(VisibleInstanceOnly, Transient, Category = "Loot", meta = (AllowPrivateAccess = "true"))
	TArray<ERelicType> Relics;

	UPROPERTY(Transient)
	TArray<FActiveRelicBuff> ActiveBuffs;
};
