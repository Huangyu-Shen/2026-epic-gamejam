#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Loot/LootTypes.h"
#include "LootBagComponent.generated.h"

class ULootConfig;

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnBagChanged);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnBagPickupRefused, EBagAddResult, Reason);

/**
 * The player's run bag: wood, stone and lost film reels under a hard slot capacity.
 * Wood and stone each occupy ceil(count / IronPerSlot) slots (the table column keeps its old name), each reel one slot.
 * Picking up a reel raises heat. A reel cannot be used: carried out alive it unlocks an upgrade option
 * (story 008). Story 002 reads/clears/settles via the read-only getters and ClearBag().
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

	/** Effective config (never null). */
	UFUNCTION(BlueprintPure, Category = "Loot")
	ULootConfig* GetConfig();

	/** Checks whether the loot would fit without adding it. */
	UFUNCTION(BlueprintPure, Category = "Loot")
	EBagAddResult CanAdd(int32 Amount, bool bWithRelic, ELootResource Resource = ELootResource::Wood);

	/**
	 * Atomically adds wood or stone and optionally one lost film reel. On failure nothing is added,
	 * OnPickupRefused fires and a debug message is shown. A reel raises heat.
	 */
	UFUNCTION(BlueprintCallable, Category = "Loot")
	EBagAddResult TryAddLoot(int32 Amount, bool bAddRelic, ERelicType RelicType, ELootResource Resource = ELootResource::Wood);

	UFUNCTION(BlueprintCallable, Category = "Loot")
	EBagAddResult TryAddWood(int32 WoodAmount) { return TryAddLoot(WoodAmount, false, ERelicType::Swiftstep); }

	UFUNCTION(BlueprintCallable, Category = "Loot")
	EBagAddResult TryAddRelic(ERelicType RelicType) { return TryAddLoot(0, true, RelicType); }

	/** Empties the bag. For story 002 (death/settlement). Does not touch heat. */
	UFUNCTION(BlueprintCallable, Category = "Loot")
	void ClearBag();

	// ---- Read-only contents ----

	/** Wood in the bag. */
	UFUNCTION(BlueprintPure, Category = "Loot")
	int32 GetWoodCount() const { return WoodCount; }

	UFUNCTION(BlueprintPure, Category = "Loot")
	int32 GetStoneCount() const { return StoneCount; }

	/** Lost film reels currently carried (copy). */
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

	/** Human-readable text for a result, for debug/HUD. */
	UFUNCTION(BlueprintPure, Category = "Loot")
	static FText GetAddResultText(EBagAddResult Result);

	virtual void BeginPlay() override;

private:
	int32 SlotsForWood(int32 Amount);

	UPROPERTY(Transient)
	TObjectPtr<ULootConfig> FallbackConfig;

	UPROPERTY(VisibleInstanceOnly, Transient, Category = "Loot", meta = (AllowPrivateAccess = "true"))
	int32 WoodCount = 0;

	UPROPERTY(VisibleInstanceOnly, Transient, Category = "Loot", meta = (AllowPrivateAccess = "true"))
	int32 StoneCount = 0;

	UPROPERTY(VisibleInstanceOnly, Transient, Category = "Loot", meta = (AllowPrivateAccess = "true"))
	TArray<ERelicType> Relics;
};
