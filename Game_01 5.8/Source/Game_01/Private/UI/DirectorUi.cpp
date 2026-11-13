#include "UI/DirectorUi.h"
#include "Blueprint/WidgetTree.h"
#include "Components/Border.h"
#include "Components/CanvasPanel.h"
#include "Components/CanvasPanelSlot.h"
#include "Components/PanelWidget.h"
#include "Components/ProgressBar.h"
#include "Components/ScrollBox.h"
#include "Components/SizeBox.h"
#include "Components/TextBlock.h"
#include "Components/VerticalBox.h"
#include "Components/VerticalBoxSlot.h"
#include "Engine/GameInstance.h"
#include "Framework/Application/SlateApplication.h"
#include "UI/HudMessageSubsystem.h"

namespace DirectorUi
{
	const FLinearColor TextColor(0.92f, 0.92f, 0.92f);
	const FLinearColor DimColor(0.6f, 0.6f, 0.6f);
	const FLinearColor GoodColor(0.35f, 0.9f, 0.4f);
	const FLinearColor BadColor(0.95f, 0.35f, 0.3f);

	static FSlateBrush SolidBrush(const FLinearColor& Tint)
	{
		FSlateBrush Brush;
		Brush.DrawAs = ESlateBrushDrawType::Image;
		Brush.TintColor = FSlateColor(Tint);
		return Brush;
	}

	UPanelWidget* MakePanel(UWidgetTree* Tree, UCanvasPanel* Canvas, const FUiPlacement& Placement, bool bScroll, UWidget** OutRoot)
	{
		UBorder* Border = Tree->ConstructWidget<UBorder>();
		Border->SetBrush(SolidBrush(FLinearColor(0.02f, 0.02f, 0.04f, 0.72f)));
		Border->SetPadding(FMargin(12.0f));

		UPanelWidget* Content = nullptr;
		if (bScroll)
		{
			Content = Tree->ConstructWidget<UScrollBox>();
		}
		else
		{
			Content = Tree->ConstructWidget<UVerticalBox>();
		}

		if (!Placement.bFillHeight && (Placement.Size.X > 0.0 || Placement.Size.Y > 0.0))
		{
			USizeBox* Box = Tree->ConstructWidget<USizeBox>();
			if (Placement.Size.X > 0.0)
			{
				Box->SetWidthOverride(static_cast<float>(Placement.Size.X));
			}
			if (Placement.Size.Y > 0.0)
			{
				Box->SetHeightOverride(static_cast<float>(Placement.Size.Y));
			}
			Box->AddChild(Content);
			Border->SetContent(Box);
		}
		else
		{
			Border->SetContent(Content);
		}

		if (UCanvasPanelSlot* Slot = Canvas->AddChildToCanvas(Border))
		{
			if (Placement.bFillHeight)
			{
				// Anchored to the left edge, stretched from the top margin to the bottom margin.
				Slot->SetAnchors(FAnchors(0.0f, 0.0f, 0.0f, 1.0f));
				Slot->SetOffsets(FMargin(static_cast<float>(Placement.Position.X), static_cast<float>(Placement.Position.Y),
					static_cast<float>(Placement.Size.X), Placement.BottomMargin));
				Slot->SetAutoSize(false);
			}
			else
			{
				Slot->SetAnchors(FAnchors(static_cast<float>(Placement.Anchor.X), static_cast<float>(Placement.Anchor.Y)));
				Slot->SetAlignment(Placement.Alignment);
				Slot->SetPosition(Placement.Position);
				Slot->SetAutoSize(true);
			}
		}
		if (OutRoot)
		{
			*OutRoot = Border;
		}
		return Content;
	}

	UTextBlock* AddText(UWidgetTree* Tree, UPanelWidget* Parent, const FString& Text, int32 Size, const FLinearColor& Color)
	{
		UTextBlock* Block = Tree->ConstructWidget<UTextBlock>();
		Block->SetText(FText::FromString(Text));
		Block->SetColorAndOpacity(FSlateColor(Color));
		// Long lines (building effects, film-card descriptions) wrap instead of running off the panel.
		Block->SetAutoWrapText(true);
		FSlateFontInfo Font = Block->GetFont();
		Font.Size = Size;
		Block->SetFont(Font);
		Parent->AddChild(Block);
		return Block;
	}

	UProgressBar* AddBar(UWidgetTree* Tree, UPanelWidget* Parent, const FLinearColor& Fill, float Width, float Height)
	{
		UProgressBar* Bar = Tree->ConstructWidget<UProgressBar>();
		Bar->SetFillColorAndOpacity(Fill);
		Bar->SetPercent(0.0f);

		USizeBox* Box = Tree->ConstructWidget<USizeBox>();
		Box->SetWidthOverride(Width);
		Box->SetHeightOverride(Height);
		Box->AddChild(Bar);
		Parent->AddChild(Box);
		return Bar;
	}

	UUiActionButton* AddButton(UWidgetTree* Tree, UPanelWidget* Parent, const FString& Caption, FName Action, int32 Index)
	{
		UUiActionButton* Button = Tree->ConstructWidget<UUiActionButton>();
		Button->Label = Tree->ConstructWidget<UTextBlock>();
		Button->Label->SetText(FText::FromString(Caption));
		FSlateFontInfo Font = Button->Label->GetFont();
		Font.Size = 16;
		Button->Label->SetFont(Font);
		Button->AddChild(Button->Label);
		Button->Init(Action, Index);
		Parent->AddChild(Button);
		return Button;
	}

	void ReturnFocusToGame()
	{
		if (FSlateApplication::IsInitialized())
		{
			FSlateApplication::Get().SetAllUserFocusToGameViewport();
		}
	}
}

void FToastPanel::Build(UWidgetTree* Tree, UCanvasPanel* Canvas)
{
	FUiPlacement Placement;
	Placement.Anchor = FVector2D(0.5, 0.0);
	Placement.Alignment = FVector2D(0.5, 0.0);
	Placement.Position = FVector2D(0.0, 16.0);
	Placement.Size = FVector2D(520.0, 0.0);
	UPanelWidget* Content = DirectorUi::MakePanel(Tree, Canvas, Placement, false, &Root);
	for (int32 Index = 0; Index < UHudMessageSubsystem::MaxMessages; ++Index)
	{
		Lines.Add(DirectorUi::AddText(Tree, Content, TEXT(""), 18));
	}
	Root->SetVisibility(ESlateVisibility::Collapsed);
}

void FToastPanel::Update(const UGameInstance* GameInstance)
{
	const UHudMessageSubsystem* Messages = UGameInstance::GetSubsystem<UHudMessageSubsystem>(GameInstance);
	const TArray<FHudMessage> Active = Messages ? Messages->GetActive() : TArray<FHudMessage>();
	if (!Root)
	{
		return;
	}
	Root->SetVisibility(Active.Num() > 0 ? ESlateVisibility::SelfHitTestInvisible : ESlateVisibility::Collapsed);
	for (int32 Index = 0; Index < Lines.Num(); ++Index)
	{
		const bool bHas = Active.IsValidIndex(Index);
		Lines[Index]->SetVisibility(bHas ? ESlateVisibility::HitTestInvisible : ESlateVisibility::Collapsed);
		if (bHas)
		{
			Lines[Index]->SetText(FText::FromString(Active[Index].Text));
			Lines[Index]->SetColorAndOpacity(FSlateColor(Active[Index].Color));
		}
	}
}

void UUiActionButton::Init(FName InAction, int32 InIndex)
{
	Action = InAction;
	Index = InIndex;
	OnClicked.AddDynamic(this, &UUiActionButton::HandleClicked);
}

void UUiActionButton::HandleClicked()
{
	OnAction.Broadcast(Action, Index);
	DirectorUi::ReturnFocusToGame();
}
