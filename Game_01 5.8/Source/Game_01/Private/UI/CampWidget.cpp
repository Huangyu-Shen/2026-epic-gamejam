#include "UI/CampWidget.h"
#include "UI/DirectorUi.h"
#include "Blueprint/WidgetTree.h"
#include "Camp/CampMetaSubsystem.h"
#include "Components/CanvasPanel.h"
#include "Components/PanelWidget.h"
#include "Editing/FilmCardSubsystem.h"
#include "Engine/GameInstance.h"
#include "Engine/World.h"
#include "Extraction/CampStockSubsystem.h"
#include "Kismet/GameplayStatics.h"
#include "Loot/LootTypes.h"

namespace
{
	FString CampBuildingName(EBuildingType Type)
	{
		return StaticEnum<EBuildingType>()->GetDisplayNameTextByValue(static_cast<int64>(Type)).ToString();
	}

	FString CampRelicLabel(ERelicType Type)
	{
		return StaticEnum<ERelicType>()->GetDisplayNameTextByValue(static_cast<int64>(Type)).ToString();
	}
}

TSharedRef<SWidget> UCampWidget::RebuildWidget()
{
	if (WidgetTree && !WidgetTree->RootWidget)
	{
		UCanvasPanel* Canvas = WidgetTree->ConstructWidget<UCanvasPanel>();
		WidgetTree->RootWidget = Canvas;

		FUiPlacement Left;
		Left.Position = FVector2D(16.0, 16.0);
		Left.Size = FVector2D(460.0, 0.0);
		Left.bFillHeight = true;
		Left.BottomMargin = 90.0f; // leaves room for the START RUN panel
		Content = DirectorUi::MakePanel(WidgetTree, Canvas, Left, /*bScroll=*/true);
		Toasts.Build(WidgetTree, Canvas);

		// START RUN lives in its own panel pinned to the bottom-left so it never scrolls out of reach.
		FUiPlacement Bottom;
		Bottom.Anchor = FVector2D(0.0, 1.0);
		Bottom.Alignment = FVector2D(0.0, 1.0);
		Bottom.Position = FVector2D(16.0, -16.0);
		Bottom.Size = FVector2D(460.0, 0.0);
		UPanelWidget* StartPanel = DirectorUi::MakePanel(WidgetTree, Canvas, Bottom);
		UUiActionButton* Start = DirectorUi::AddButton(WidgetTree, StartPanel, TEXT("START RUN"), TEXT("Start"));
		Start->OnAction.AddDynamic(this, &UCampWidget::HandleAction);
	}
	return Super::RebuildWidget();
}

FString UCampWidget::MakeSignature() const
{
	FString Sig = Message;
	if (const UGameInstance* GI = GetGameInstance())
	{
		if (const UCampStockSubsystem* Stock = GI->GetSubsystem<UCampStockSubsystem>())
		{
			Sig += FString::Printf(TEXT("|I%d"), Stock->GetIron());
			for (const ERelicType Relic : Stock->GetWarehouseRelics())
			{
				Sig += FString::Printf(TEXT(",%d"), static_cast<int32>(Relic));
			}
		}
		if (const UCampMetaSubsystem* Camp = GI->GetSubsystem<UCampMetaSubsystem>())
		{
			for (uint8 B = 0; B < static_cast<uint8>(EBuildingType::Count); ++B)
			{
				Sig += FString::Printf(TEXT("|B%d"), Camp->GetBuildingLevel(static_cast<EBuildingType>(B)));
			}
			for (const FName Card : Camp->PendingFilmCards)
			{
				Sig += TEXT("|C") + Card.ToString();
			}
		}
	}
	return Sig;
}

void UCampWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);

	Toasts.Update(GetGameInstance());

	const FString Sig = MakeSignature();
	if (Sig != LastSignature)
	{
		LastSignature = Sig;
		Rebuild();
	}
}

void UCampWidget::Rebuild()
{
	UGameInstance* GI = GetGameInstance();
	UWorld* World = GetWorld();
	UCampStockSubsystem* Stock = GI ? GI->GetSubsystem<UCampStockSubsystem>() : nullptr;
	UCampMetaSubsystem* Camp = GI ? GI->GetSubsystem<UCampMetaSubsystem>() : nullptr;
	UFilmCardSubsystem* Cards = World ? World->GetSubsystem<UFilmCardSubsystem>() : nullptr;
	if (!Content || !Stock || !Camp)
	{
		return;
	}

	Content->ClearChildren();

	DirectorUi::AddText(WidgetTree, Content, TEXT("CAMP"), 28);
	DirectorUi::AddText(WidgetTree, Content, FString::Printf(TEXT("Iron: %d"), Stock->GetIron()), 22, DirectorUi::GoodColor);
	if (!Message.IsEmpty())
	{
		DirectorUi::AddText(WidgetTree, Content, Message, 16, bMessageIsError ? DirectorUi::BadColor : DirectorUi::GoodColor);
	}

	// Buildings.
	DirectorUi::AddText(WidgetTree, Content, TEXT("\nBUILDINGS (you can also click them in the scene)"), 16, DirectorUi::DimColor);
	for (uint8 B = 0; B < static_cast<uint8>(EBuildingType::Count); ++B)
	{
		const EBuildingType Type = static_cast<EBuildingType>(B);
		const int32 Level = Camp->GetBuildingLevel(Type);
		const FBuildingRow& Row = Camp->GetBuildingRow(Type);
		const FString Effect = Type == EBuildingType::Smithy
			? FString::Printf(TEXT("weapon damage +%.0f"), Camp->GetWeaponDamageBonus())
			: FString::Printf(TEXT("max health +%.0f"), Camp->GetMaxHealthBonus());
		DirectorUi::AddText(WidgetTree, Content, FString::Printf(TEXT("%s   Lv %d / %d   (%s)"),
			*CampBuildingName(Type), Level, Row.MaxLevel, *Effect), 18);
		const FString Caption = Camp->IsMaxLevel(Type)
			? FString(TEXT("Maximum level"))
			: FString::Printf(TEXT("Upgrade  (%d Iron)"), Camp->GetUpgradeCost(Type));
		UUiActionButton* Button = DirectorUi::AddButton(WidgetTree, Content, Caption, TEXT("Upgrade"), B);
		Button->SetIsEnabled(!Camp->IsMaxLevel(Type));
		Button->OnAction.AddDynamic(this, &UCampWidget::HandleAction);
	}

	// Warehouse.
	const TArray<ERelicType> Relics = Stock->GetWarehouseRelics();
	DirectorUi::AddText(WidgetTree, Content, FString::Printf(TEXT("\nWAREHOUSE (%d relics)"), Relics.Num()), 16, DirectorUi::DimColor);
	if (Relics.IsEmpty())
	{
		DirectorUi::AddText(WidgetTree, Content, TEXT("Empty. Extract with relics to store them here."), 16, DirectorUi::DimColor);
	}
	for (int32 Index = 0; Index < Relics.Num(); ++Index)
	{
		UUiActionButton* Button = DirectorUi::AddButton(WidgetTree, Content,
			FString::Printf(TEXT("Dismantle %s  (+%d Iron)"), *CampRelicLabel(Relics[Index]), Camp->GetRelicDismantleIron()),
			TEXT("Dismantle"), Index);
		Button->OnAction.AddDynamic(this, &UCampWidget::HandleAction);
	}

	// Editing room.
	if (Cards)
	{
		DirectorUi::AddText(WidgetTree, Content,
			FString::Printf(TEXT("\nEDITING ROOM  (film cards %d / %d - harder run, more Iron)"),
				Cards->GetActiveCards().Num(), UFilmCardSubsystem::MaxSlots), 16, DirectorUi::DimColor);
		const TArray<FName> Available = Cards->GetAvailableCards();
		for (int32 Index = 0; Index < Available.Num(); ++Index)
		{
			const bool bSlotted = Cards->GetActiveCards().Contains(Available[Index]);
			DirectorUi::AddText(WidgetTree, Content, Cards->DescribeCard(Available[Index]), 16);
			UUiActionButton* Button = DirectorUi::AddButton(WidgetTree, Content,
				bSlotted ? TEXT("Remove from the run") : TEXT("Add to the run"), TEXT("Card"), Index);
			Button->OnAction.AddDynamic(this, &UCampWidget::HandleAction);
		}
	}

}

void UCampWidget::HandleAction(FName Action, int32 Index)
{
	UGameInstance* GI = GetGameInstance();
	UCampMetaSubsystem* Camp = GI ? GI->GetSubsystem<UCampMetaSubsystem>() : nullptr;
	UFilmCardSubsystem* Cards = GetWorld() ? GetWorld()->GetSubsystem<UFilmCardSubsystem>() : nullptr;
	if (!Camp)
	{
		return;
	}

	if (Action == TEXT("Upgrade"))
	{
		const EBuildingType Type = static_cast<EBuildingType>(Index);
		FText Reason;
		if (Camp->TryUpgrade(Type, Reason))
		{
			Message = FString::Printf(TEXT("%s upgraded to level %d"), *CampBuildingName(Type), Camp->GetBuildingLevel(Type));
			bMessageIsError = false;
		}
		else
		{
			Message = Reason.ToString();
			bMessageIsError = true;
		}
	}
	else if (Action == TEXT("Dismantle"))
	{
		const int32 Gained = Camp->DismantleRelic(Index);
		Message = Gained > 0 ? FString::Printf(TEXT("Relic dismantled: +%d Iron"), Gained) : FString(TEXT("Nothing to dismantle"));
		bMessageIsError = Gained <= 0;
	}
	else if (Action == TEXT("Card") && Cards)
	{
		const TArray<FName> Available = Cards->GetAvailableCards();
		if (Available.IsValidIndex(Index))
		{
			const FName Card = Available[Index];
			if (Cards->GetActiveCards().Contains(Card))
			{
				Cards->RemoveCard(Card);
				Message = TEXT("Card removed");
				bMessageIsError = false;
			}
			else if (Cards->AddCard(Card))
			{
				Message = TEXT("Card added to the next run");
				bMessageIsError = false;
			}
			else
			{
				Message = TEXT("Both film card slots are full");
				bMessageIsError = true;
			}
		}
	}
	else if (Action == TEXT("Start"))
	{
		UGameplayStatics::OpenLevel(this, TEXT("L_Greybox"));
	}
}
