// Director's Cut - Story 005: the pre-run film-card (editing-room) state and application.
#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "Editing/FilmCardTypes.h"
#include "Extraction/ExtractionTypes.h"
#include "FilmCardSubsystem.generated.h"

/**
 * Holds the player's pre-run film-card selection (two slots), applies the active cards when a run
 * starts (forcing horde/boss and setting the earnings multiplier), and clears them when a run ends
 * so the choice never leaks into the next run. The editing-room UI itself is story 007; here the
 * slots are driven by debug keys.
 */
UCLASS()
class GAME_01_API UFilmCardSubsystem : public UWorldSubsystem
{
	GENERATED_BODY()

public:
	static constexpr int32 MaxSlots = 2;

	virtual void Initialize(FSubsystemCollectionBase& Collection) override;

	/** Cards currently in the slots (in insertion order). */
	UFUNCTION(BlueprintPure, Category = "Editing")
	const TArray<FName>& GetActiveCards() const { return ActiveCards; }

	/** Every card the data table defines (its row keys). */
	UFUNCTION(BlueprintPure, Category = "Editing")
	TArray<FName> GetAvailableCards() const { return AvailableCards; }

	/** Slot a card. Fails when the slots are full or the card is already slotted. */
	UFUNCTION(BlueprintCallable, Category = "Editing")
	bool AddCard(FName CardId);

	/** Unslot a card. */
	UFUNCTION(BlueprintCallable, Category = "Editing")
	bool RemoveCard(FName CardId);

	/** Product of the active cards' multipliers (1.0 with none slotted). */
	UFUNCTION(BlueprintPure, Category = "Editing")
	float GetEarningsMultiplier() const;

	/** Apply the active cards at run start: force horde/boss and set the settlement multiplier. */
	UFUNCTION(BlueprintCallable, Category = "Editing")
	void ApplyActiveCards();

	/** Clear the slots and reset the settlement multiplier to 1.0. */
	UFUNCTION(BlueprintCallable, Category = "Editing")
	void ResetCards();

	/** One readable line for the editing-room panel: id, effect and earnings multiplier. */
	UFUNCTION(BlueprintPure, Category = "Editing")
	FString DescribeCard(FName CardId) const;

private:
	/** Copies the slots into the game-instance store so they outlive this world. */
	void StoreCards() const;

	UFUNCTION()
	void HandleRunEnded(const FRunResult& Result);

	const FFilmCardRow* FindRow(FName CardId) const;

	UPROPERTY(Transient)
	TArray<FName> ActiveCards;

	UPROPERTY(Transient)
	TArray<FName> AvailableCards;

	UPROPERTY(Transient)
	TObjectPtr<UDataTable> CardTable;
};
