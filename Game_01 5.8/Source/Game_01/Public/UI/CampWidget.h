// Director's Cut - Story 007. The camp screen: wood and stone, building upgrades, the unlocked options, the editing room, Start Run.
#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "UI/DirectorUi.h"
#include "CampWidget.generated.h"

class UPanelWidget;

/**
 * Built in C++. Rebuilds its rows whenever the camp state changes (wood, stone, levels, unlocked options, slotted cards),
 * so it always shows what the game actually holds. It changes game state only through the camp subsystems.
 */
UCLASS()
class GAME_01_API UCampWidget : public UUserWidget
{
	GENERATED_BODY()

protected:
	virtual TSharedRef<SWidget> RebuildWidget() override;
	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;

private:
	UFUNCTION()
	void HandleAction(FName Action, int32 Index);

	/** Tears down and rebuilds every row. */
	void Rebuild();

	/** Cheap fingerprint of everything the panel shows; a change triggers Rebuild. */
	FString MakeSignature() const;

	UPROPERTY() TObjectPtr<UPanelWidget> Content;

	/** Short messages (building upgraded, not enough wood...) at the top centre. */
	FToastPanel Toasts;

	FString LastSignature;

	/** Result of the last action, shown at the top (upgrade refused, building upgraded...). */
	FString Message;
	bool bMessageIsError = false;
};
