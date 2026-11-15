#include "UI/DirectorUi.h"
#include "Blueprint/WidgetTree.h"
#include "Components/Border.h"
#include "Components/ButtonSlot.h"
#include "Components/HorizontalBox.h"
#include "Components/HorizontalBoxSlot.h"
#include "Styling/SlateTypes.h"
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

	/** A rounded rectangle with a thin outline: panels, cards and buttons all share it so their edges are clear. */
	static FSlateBrush BoxBrush(const FLinearColor& Fill, const FLinearColor& Outline, float OutlineWidth = 1.5f, float Radius = 8.0f)
	{
		FSlateBrush Brush;
		Brush.DrawAs = ESlateBrushDrawType::RoundedBox;
		Brush.TintColor = FSlateColor(Fill);
		Brush.OutlineSettings.CornerRadii = FVector4(Radius, Radius, Radius, Radius);
		Brush.OutlineSettings.RoundingType = ESlateBrushRoundingType::FixedRadius;
		Brush.OutlineSettings.Color = FSlateColor(Outline);
		Brush.OutlineSettings.Width = OutlineWidth;
		return Brush;
	}

	UPanelWidget* MakePanel(UWidgetTree* Tree, UCanvasPanel* Canvas, const FUiPlacement& Placement, bool bScroll, UWidget** OutRoot)
	{
		UBorder* Border = Tree->ConstructWidget<UBorder>();
		Border->SetBrush(BoxBrush(FLinearColor(0.03f, 0.035f, 0.06f, 0.95f), FLinearColor(0.55f, 0.6f, 0.75f, 0.9f), 2.0f, 10.0f));
		Border->SetPadding(FMargin(16.0f));

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

	UUiActionButton* AddButton(UWidgetTree* Tree, UPanelWidget* Parent, const FString& Caption, FName Action, int32 Index, EUiButtonKind Kind)
	{
		FLinearColor Fill(0.16f, 0.18f, 0.26f);
		FLinearColor Edge(0.6f, 0.65f, 0.8f);
		if (Kind == EUiButtonKind::Primary)
		{
			Fill = FLinearColor(0.08f, 0.38f, 0.16f);
			Edge = FLinearColor(0.45f, 1.0f, 0.55f);
		}
		else if (Kind == EUiButtonKind::Danger)
		{
			Fill = FLinearColor(0.36f, 0.09f, 0.08f);
			Edge = FLinearColor(1.0f, 0.5f, 0.45f);
		}

		UUiActionButton* Button = Tree->ConstructWidget<UUiActionButton>();
		FButtonStyle Style;
		Style.SetNormal(BoxBrush(Fill, Edge));
		Style.SetHovered(BoxBrush(Fill * 1.6f + FLinearColor(0.05f, 0.05f, 0.05f, 0.0f), FLinearColor::White, 2.5f));
		Style.SetPressed(BoxBrush(Fill * 0.7f, Edge));
		Style.SetDisabled(BoxBrush(FLinearColor(0.1f, 0.1f, 0.12f), FLinearColor(0.3f, 0.3f, 0.35f)));
		Button->SetStyle(Style);

		Button->Label = Tree->ConstructWidget<UTextBlock>();
		Button->Label->SetText(FText::FromString(Caption));
		Button->Label->SetColorAndOpacity(FSlateColor(FLinearColor::White));
		Button->Label->SetJustification(ETextJustify::Center);
		FSlateFontInfo Font = Button->Label->GetFont();
		Font.Size = 18;
		Font.TypefaceFontName = TEXT("Bold");
		Button->Label->SetFont(Font);
		if (UButtonSlot* Slot = Cast<UButtonSlot>(Button->AddChild(Button->Label)))
		{
			Slot->SetPadding(FMargin(16.0f, 9.0f));
			Slot->SetHorizontalAlignment(HAlign_Center);
		}
		Button->Init(Action, Index);
		Parent->AddChild(Button);
		if (UVerticalBoxSlot* BoxSlot = Cast<UVerticalBoxSlot>(Button->Slot))
		{
			BoxSlot->SetPadding(FMargin(0.0f, 4.0f));
		}
		return Button;
	}

	UPanelWidget* AddRow(UWidgetTree* Tree, UPanelWidget* Parent)
	{
		UHorizontalBox* Row = Tree->ConstructWidget<UHorizontalBox>();
		Parent->AddChild(Row);
		return Row;
	}

	UPanelWidget* AddCard(UWidgetTree* Tree, UPanelWidget* Row, const FLinearColor& Accent, float Width, UWidget** OutRoot)
	{
		USizeBox* Size = Tree->ConstructWidget<USizeBox>();
		Size->SetWidthOverride(Width);
		UBorder* Card = Tree->ConstructWidget<UBorder>();
		Card->SetBrush(BoxBrush(FLinearColor(0.07f, 0.08f, 0.13f, 1.0f), Accent, 2.5f, 10.0f));
		Card->SetPadding(FMargin(14.0f));
		UVerticalBox* Content = Tree->ConstructWidget<UVerticalBox>();
		Card->SetContent(Content);
		Size->AddChild(Card);
		Row->AddChild(Size);
		if (UHorizontalBoxSlot* Slot = Cast<UHorizontalBoxSlot>(Size->Slot))
		{
			Slot->SetPadding(FMargin(6.0f, 0.0f));
		}
		if (OutRoot)
		{
			*OutRoot = Size;
		}
		return Content;
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
