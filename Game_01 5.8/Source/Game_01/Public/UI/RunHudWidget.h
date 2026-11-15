// Director's Cut - Story 007. The in-run screen: HUD, level-up panel, bag view, extraction bar, result screen.
#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "UI/DirectorUi.h"
#include "RunHudWidget.generated.h"

class AExtractionZone;
class UPanelWidget;
class UProgressBar;
class UTextBlock;
class UUiActionButton;
class UWidget;

/**
 * Built entirely in C++ (no widget blueprint). Reads the run subsystems every frame; owns no game state.
 * Shows: HP, level + XP bar, heat, bag usage and lost film reels, the extraction bar while it fills,
 * the 3-choice level-up panel while an offer is pending, and the result screen when the run ends.
 */
UCLASS()
class GAME_01_API URunHudWidget : public UUserWidget
{
	GENERATED_BODY()

protected:
	virtual TSharedRef<SWidget> RebuildWidget() override;
	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;

private:
	UFUNCTION()
	void HandleAction(FName Action, int32 Index);

	void RefreshOfferPanel();
	void RefreshResultPanel();

	/** Short messages (searched, pickup refused...) at the top centre. */
	FToastPanel Toasts;

	// Always-on HUD.
	UPROPERTY() TObjectPtr<UTextBlock> HealthText;
	UPROPERTY() TObjectPtr<UProgressBar> HealthBar;
	UPROPERTY() TObjectPtr<UTextBlock> LevelText;
	UPROPERTY() TObjectPtr<UProgressBar> XpBar;
	UPROPERTY() TObjectPtr<UTextBlock> LoadoutText;
	UPROPERTY() TObjectPtr<UTextBlock> HeatText;
	UPROPERTY() TObjectPtr<UProgressBar> HeatBar;
	UPROPERTY() TObjectPtr<UTextBlock> BagText;
	UPROPERTY() TObjectPtr<UTextBlock> RelicText;

	// Extraction bar (shown only while it is filling). The Root pointers are the bordered panel around the content.
	UPROPERTY() TObjectPtr<UWidget> ExtractionRoot;
	UPROPERTY() TObjectPtr<UProgressBar> ExtractionBar;
	TWeakObjectPtr<AExtractionZone> Zone;

	// Level-up panel.
	UPROPERTY() TObjectPtr<UWidget> OfferRoot;
	UPROPERTY() TArray<TObjectPtr<UUiActionButton>> OfferButtons;
	UPROPERTY() TArray<TObjectPtr<UUiActionButton>> CutButtons;
	UPROPERTY() TArray<TObjectPtr<UWidget>> CardRoots;
	UPROPERTY() TArray<TObjectPtr<UTextBlock>> CardTitles;
	UPROPERTY() TArray<TObjectPtr<UTextBlock>> CardDetails;
	UPROPERTY() TArray<TObjectPtr<UTextBlock>> CardNotes;
	UPROPERTY() TObjectPtr<UTextBlock> CutInfoText;

	// Result screen.
	UPROPERTY() TObjectPtr<UWidget> ResultRoot;
	UPROPERTY() TObjectPtr<UTextBlock> ResultText;
	bool bResultShown = false;
};
