// Director's Cut - Story 002. In-memory camp stock; persistence is story 007, camp UI/buildings story 006.
#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Loot/LootTypes.h"
#include "CampStockSubsystem.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnCampStockChanged);

/**
 * Camp Iron and the set of unlocked upgrade options, kept for the whole game session.
 * (Lost film reels are not stored: carrying one out unlocks its option at once, see URunOutcomeSubsystem.)
 * Access: GetGameInstance()->GetSubsystem<UCampStockSubsystem>().
 */
UCLASS()
class GAME_01_API UCampStockSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	/** Fired whenever Iron or the warehouse changes. */
	UPROPERTY(BlueprintAssignable, Category = "Camp")
	FOnCampStockChanged OnStockChanged;

	/** Adds Iron to the camp. Non-positive amounts are ignored. */
	UFUNCTION(BlueprintCallable, Category = "Camp")
	void AddIron(int32 Amount);

	/** Unlocks an upgrade option for every later run. Returns true if it was new (None and repeats return false). */
	UFUNCTION(BlueprintCallable, Category = "Camp")
	bool UnlockOption(FName Option);

	UFUNCTION(BlueprintPure, Category = "Camp")
	bool IsOptionUnlocked(FName Option) const { return UnlockedOptions.Contains(Option); }

	/** Deducts Amount Iron; returns false and changes nothing if the stock is too low (story 006 building upgrade). */
	UFUNCTION(BlueprintCallable, Category = "Camp")
	bool SpendIron(int32 Amount);

	/** Replaces Iron and the unlocked options wholesale (save-game restore, story 007). Negative Iron is clamped to 0. */
	UFUNCTION(BlueprintCallable, Category = "Camp")
	void RestoreState(int32 InIron, const TArray<FName>& InUnlockedOptions);

	UFUNCTION(BlueprintPure, Category = "Camp")
	int32 GetIron() const { return Iron; }

	/** Unlocked upgrade options, in the order they were unlocked. */
	const TArray<FName>& GetUnlockedOptions() const { return UnlockedOptions; }

private:
	UPROPERTY(VisibleInstanceOnly, Category = "Camp", meta = (AllowPrivateAccess = "true"))
	int32 Iron = 0;

	UPROPERTY(VisibleInstanceOnly, Category = "Camp", meta = (AllowPrivateAccess = "true"))
	TArray<FName> UnlockedOptions;
};
