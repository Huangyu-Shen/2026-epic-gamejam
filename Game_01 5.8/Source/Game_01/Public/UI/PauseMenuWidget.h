// Director's Cut - the Esc menu. One widget for both screens; what it offers depends on where the player is.
#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "PauseMenuWidget.generated.h"

class UTextBlock;
class UUiActionButton;

/** Which screen opened the menu. */
UENUM()
enum class EPauseContext : uint8
{
	/** The camp: Resume, Settings, Quit game. */
	Camp,
	/** A run: Resume, Settings, Return to camp (the bag is lost), Quit game. */
	Run,
};

/**
 * The Esc menu. In a run it pauses the game while it is open. Settings are the three volume levels, in steps of 10%,
 * kept for this session only. "Return to camp" in a run abandons it: the bag is lost exactly as if the hero had died,
 * and it needs a second click to confirm.
 */
UCLASS()
class GAME_01_API UPauseMenuWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	void SetContext(EPauseContext InContext) { Context = InContext; }

	bool IsOpen() const { return bOpen; }

	/** Opens the menu, or closes it when it is already open. */
	void Toggle();

	/** Presses a button by action name, as a click would. For the test probe. */
	void PressForProbe(FName Action, int32 Index = 0) { HandleAction(Action, Index); }

	/** What the "Return to camp" button does in a run: the first press asks to confirm, the second leaves. Public for the test probe. */
	void PressReturnToCamp();

protected:
	virtual TSharedRef<SWidget> RebuildWidget() override;
	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;
	virtual FReply NativeOnKeyDown(const FGeometry& MyGeometry, const FKeyEvent& InKeyEvent) override;

private:
	UFUNCTION()
	void HandleAction(FName Action, int32 Index);

	void Open();
	void Close();
	void ShowSettings(bool bShow);
	void RefreshVolumeTexts();

	EPauseContext Context = EPauseContext::Camp;
	bool bOpen = false;
	bool bSettingsShown = false;
	bool bReturnArmed = false;
	double ReturnArmedAt = 0.0;

	UPROPERTY() TObjectPtr<UWidget> PanelRoot;
	UPROPERTY() TObjectPtr<UUiActionButton> ReturnButton;
	UPROPERTY() TArray<TObjectPtr<UWidget>> SettingsWidgets;
	UPROPERTY() TArray<TObjectPtr<UTextBlock>> VolumeTexts;
};
