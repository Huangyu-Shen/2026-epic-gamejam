// Director's Cut - Story 007. Shared UMG building blocks: one readable style (white text on a dark panel)
// so the Should-tier art pass can restyle every screen from this one place.
#pragma once

#include "CoreMinimal.h"
#include "Components/Button.h"
#include "DirectorUi.generated.h"

class UBorder;
class UCanvasPanel;
class UGameInstance;
class UPanelWidget;
class UProgressBar;
class UTextBlock;
class UWidget;
class UWidgetTree;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnUiAction, FName, Action, int32, Index);

/** A button that carries an action name and an index and reports both when clicked. */
UCLASS()
class GAME_01_API UUiActionButton : public UButton
{
	GENERATED_BODY()

public:
	/** Fired when the button is clicked. */
	UPROPERTY()
	FOnUiAction OnAction;

	UPROPERTY()
	FName Action;

	UPROPERTY()
	int32 Index = 0;

	/** The caption (owned by the widget tree). */
	UPROPERTY()
	TObjectPtr<UTextBlock> Label;

	/** Hooks the click; call once after construction. */
	void Init(FName InAction, int32 InIndex);

private:
	UFUNCTION()
	void HandleClicked();
};

/** Where a panel sits on the screen. */
struct FUiPlacement
{
	FVector2D Anchor = FVector2D(0.0, 0.0);
	FVector2D Alignment = FVector2D(0.0, 0.0);
	FVector2D Position = FVector2D(16.0, 16.0);
	/** Fixed size; a zero component means "size to content". */
	FVector2D Size = FVector2D::ZeroVector;
	/**
	 * Stretch from Position.Y down to BottomMargin above the screen bottom (Size.X is the width), so a long list
	 * uses all the free height and scrolls only when it really has to. Anchor/Alignment are ignored.
	 */
	bool bFillHeight = false;
	float BottomMargin = 0.0f;
};

/** A small stack of message lines at the top centre, fed by UHudMessageSubsystem. Owned by one widget. */
struct FToastPanel
{
	UWidget* Root = nullptr;
	TArray<UTextBlock*> Lines;

	/** Creates the panel on the canvas, hidden. */
	void Build(UWidgetTree* Tree, UCanvasPanel* Canvas);

	/** Shows the session's live messages (or hides the panel when there are none). */
	void Update(const UGameInstance* GameInstance);
};

namespace DirectorUi
{
	/** Shared palette. */
	extern const FLinearColor TextColor;
	extern const FLinearColor DimColor;
	extern const FLinearColor GoodColor;
	extern const FLinearColor BadColor;

	/**
	 * Adds a dark panel to the canvas and returns the vertical box (or scroll box) its content goes into.
	 * OutRoot, when given, receives the outermost widget (the border) so a caller can show or hide the whole panel.
	 */
	UPanelWidget* MakePanel(UWidgetTree* Tree, UCanvasPanel* Canvas, const FUiPlacement& Placement, bool bScroll = false,
		UWidget** OutRoot = nullptr);

	UTextBlock* AddText(UWidgetTree* Tree, UPanelWidget* Parent, const FString& Text, int32 Size = 18,
		const FLinearColor& Color = FLinearColor::White);

	UProgressBar* AddBar(UWidgetTree* Tree, UPanelWidget* Parent, const FLinearColor& Fill, float Width, float Height);

	UUiActionButton* AddButton(UWidgetTree* Tree, UPanelWidget* Parent, const FString& Caption, FName Action, int32 Index = 0);

	/** Gives keyboard focus back to the game so Space/WASD never "press" a button that was just clicked. */
	void ReturnFocusToGame();
}
