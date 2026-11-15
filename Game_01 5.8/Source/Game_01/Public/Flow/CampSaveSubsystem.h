// Director's Cut - Story 007. Loads the camp save at startup and auto-saves on every camp change.
#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "CampSaveSubsystem.generated.h"

/**
 * One save slot. Loads in Initialize (after the stock and meta subsystems exist), then writes the slot
 * after a stock or building change: "after each run" (extraction banks loot into the stock) and "after each
 * upgrade" (an upgrade spends wood or stone and raises a level). Several changes in one frame (an upgrade, a
 * run settlement) are written once, on the next tick, so no file holds a half-finished state.
 * SaveNow writes at once and cancels the pending write.
 */
UCLASS()
class GAME_01_API UCampSaveSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	/** Writes the current camp state to the slot. Returns false if the write failed. */
	UFUNCTION(BlueprintCallable, Category = "Save")
	bool SaveNow();

	/** Deletes the slot and resets stock and buildings (developer/new-game helper). */
	UFUNCTION(BlueprintCallable, Category = "Save")
	void DeleteSaveAndReset();

	/** Slot name of the single auto-save. */
	static const FString& GetSlotName();

private:
	UFUNCTION()
	void HandleCampChanged();

	/** Suppresses saving while a load is being applied. */
	bool bApplyingLoad = false;

	/** A change was made and the slot has not been written yet; one write on the next tick covers every change of the frame. */
	bool bSavePending = false;
};
