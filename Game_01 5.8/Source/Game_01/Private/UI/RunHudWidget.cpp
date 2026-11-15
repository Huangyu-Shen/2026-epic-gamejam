#include "UI/RunHudWidget.h"
#include "UI/DirectorUi.h"
#include "Blueprint/WidgetTree.h"
#include "Combat/HealthComponent.h"
#include "Components/CanvasPanel.h"
#include "Components/PanelWidget.h"
#include "Components/ProgressBar.h"
#include "Components/TextBlock.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "Extraction/ExtractionTypes.h"
#include "Extraction/ExtractionZone.h"
#include "Extraction/RunOutcomeSubsystem.h"
#include "Kismet/GameplayStatics.h"
#include "Loot/LootBagComponent.h"
#include "Loot/LootHeatSubsystem.h"
#include "Player/HeroCharacter.h"
#include "Progression/RunProgressionSubsystem.h"

namespace
{
	FString HudRelicLabel(ERelicType Type)
	{
		return StaticEnum<ERelicType>()->GetDisplayNameTextByValue(static_cast<int64>(Type)).ToString();
	}

	FString HudRelicNames(const TArray<ERelicType>& Relics)
	{
		FString Out;
		for (const ERelicType Type : Relics)
		{
			Out += (Out.IsEmpty() ? FString() : FString(TEXT(", "))) + HudRelicLabel(Type);
		}
		return Out.IsEmpty() ? FString(TEXT("none")) : Out;
	}

	FString HudOptionNames(const TArray<FName>& Options)
	{
		FString Out;
		for (const FName Option : Options)
		{
			Out += (Out.IsEmpty() ? FString() : FString(TEXT(", "))) + Option.ToString();
		}
		return Out.IsEmpty() ? FString(TEXT("none")) : Out;
	}
}

TSharedRef<SWidget> URunHudWidget::RebuildWidget()
{
	if (WidgetTree && !WidgetTree->RootWidget)
	{
		UCanvasPanel* Canvas = WidgetTree->ConstructWidget<UCanvasPanel>();
		UWidget* ExtractionRootRaw = nullptr;
		UWidget* OfferRootRaw = nullptr;
		UWidget* ResultRootRaw = nullptr;
		WidgetTree->RootWidget = Canvas;

		// Top-left: health, level/XP, heat.
		FUiPlacement TopLeft;
		TopLeft.Position = FVector2D(16.0, 16.0);
		TopLeft.Size = FVector2D(320.0, 0.0);
		UPanelWidget* Stats = DirectorUi::MakePanel(WidgetTree, Canvas, TopLeft);
		HealthText = DirectorUi::AddText(WidgetTree, Stats, TEXT("HP"), 18);
		HealthBar = DirectorUi::AddBar(WidgetTree, Stats, FLinearColor(0.85f, 0.15f, 0.15f), 296.0f, 14.0f);
		LevelText = DirectorUi::AddText(WidgetTree, Stats, TEXT("Level 1"), 18);
		XpBar = DirectorUi::AddBar(WidgetTree, Stats, FLinearColor(0.25f, 0.55f, 1.0f), 296.0f, 10.0f);
		LoadoutText = DirectorUi::AddText(WidgetTree, Stats, TEXT(""), 14, DirectorUi::DimColor);
		LoadoutText->SetAutoWrapText(true);
		HeatText = DirectorUi::AddText(WidgetTree, Stats, TEXT("Heat"), 16);
		HeatBar = DirectorUi::AddBar(WidgetTree, Stats, FLinearColor(1.0f, 0.55f, 0.1f), 296.0f, 8.0f);

		// Top-right: bag view.
		FUiPlacement TopRight;
		TopRight.Anchor = FVector2D(1.0, 0.0);
		TopRight.Alignment = FVector2D(1.0, 0.0);
		TopRight.Position = FVector2D(-16.0, 16.0);
		TopRight.Size = FVector2D(300.0, 0.0);
		UPanelWidget* Bag = DirectorUi::MakePanel(WidgetTree, Canvas, TopRight);
		DirectorUi::AddText(WidgetTree, Bag, TEXT("BAG"), 16, DirectorUi::DimColor);
		BagText = DirectorUi::AddText(WidgetTree, Bag, TEXT(""), 18);
		RelicText = DirectorUi::AddText(WidgetTree, Bag, TEXT(""), 16);

		// Bottom-left: controls reminder.
		FUiPlacement BottomLeft;
		BottomLeft.Anchor = FVector2D(0.0, 1.0);
		BottomLeft.Alignment = FVector2D(0.0, 1.0);
		BottomLeft.Position = FVector2D(16.0, -16.0);
		UPanelWidget* Help = DirectorUi::MakePanel(WidgetTree, Canvas, BottomLeft);
		DirectorUi::AddText(WidgetTree, Help, TEXT("WASD move   Space dodge   E search   1-3 take an upgrade   4-6 cut one   Esc menu"), 14, DirectorUi::DimColor);

		// Bottom-centre: extraction bar.
		FUiPlacement Bottom;
		Bottom.Anchor = FVector2D(0.5, 1.0);
		Bottom.Alignment = FVector2D(0.5, 1.0);
		Bottom.Position = FVector2D(0.0, -80.0);
		Bottom.Size = FVector2D(420.0, 0.0);
		UPanelWidget* Extraction = DirectorUi::MakePanel(WidgetTree, Canvas, Bottom, false, &ExtractionRootRaw);
		DirectorUi::AddText(WidgetTree, Extraction, TEXT("EXTRACTING - stay in the zone"), 18, DirectorUi::GoodColor);
		ExtractionBar = DirectorUi::AddBar(WidgetTree, Extraction, DirectorUi::GoodColor, 396.0f, 18.0f);

		// Centre: level-up panel.
		FUiPlacement Centre;
		Centre.Anchor = FVector2D(0.5, 0.5);
		Centre.Alignment = FVector2D(0.5, 0.5);
		Centre.Position = FVector2D(0.0, 0.0);
		Centre.Size = FVector2D(960.0, 0.0);
		UPanelWidget* Offer = DirectorUi::MakePanel(WidgetTree, Canvas, Centre, false, &OfferRootRaw);
		DirectorUi::AddText(WidgetTree, Offer, TEXT("LEVEL UP"), 30, DirectorUi::GoodColor);
		DirectorUi::AddText(WidgetTree, Offer, TEXT("Take one upgrade. Cutting one first removes it from your pool for good and gives it to the Boss."), 16, DirectorUi::TextColor);
		UPanelWidget* Row = DirectorUi::AddRow(WidgetTree, Offer);
		for (int32 Index = 0; Index < 3; ++Index)
		{
			UWidget* CardRoot = nullptr;
			UPanelWidget* Card = DirectorUi::AddCard(WidgetTree, Row, FLinearColor(0.5f, 0.6f, 0.9f), 290.0f, &CardRoot);
			CardRoots.Add(CardRoot);
			CardTitles.Add(DirectorUi::AddText(WidgetTree, Card, TEXT(""), 26, FLinearColor::White));
			CardDetails.Add(DirectorUi::AddText(WidgetTree, Card, TEXT(""), 20, DirectorUi::TextColor));
			CardNotes.Add(DirectorUi::AddText(WidgetTree, Card, TEXT(""), 16, FLinearColor(1.0f, 0.85f, 0.3f)));
			UUiActionButton* Take = DirectorUi::AddButton(WidgetTree, Card, FString::Printf(TEXT("TAKE   [%d]"), Index + 1), TEXT("Offer"), Index, EUiButtonKind::Primary);
			Take->OnAction.AddDynamic(this, &URunHudWidget::HandleAction);
			OfferButtons.Add(Take);
			UUiActionButton* CutButton = DirectorUi::AddButton(WidgetTree, Card, FString::Printf(TEXT("Cut   [%d]"), Index + 4), TEXT("Cut"), Index, EUiButtonKind::Danger);
			CutButton->OnAction.AddDynamic(this, &URunHudWidget::HandleAction);
			CutButtons.Add(CutButton);
		}
		CutInfoText = DirectorUi::AddText(WidgetTree, Offer, TEXT(""), 16, DirectorUi::DimColor);

		// Centre: result screen.
		FUiPlacement ResultPlacement = Centre;
		ResultPlacement.Size = FVector2D(520.0, 0.0);
		UPanelWidget* Result = DirectorUi::MakePanel(WidgetTree, Canvas, ResultPlacement, false, &ResultRootRaw);
		ResultText = DirectorUi::AddText(WidgetTree, Result, TEXT(""), 22);
		UUiActionButton* Return = DirectorUi::AddButton(WidgetTree, Result, TEXT("Return to Camp"), TEXT("Camp"));
		Return->OnAction.AddDynamic(this, &URunHudWidget::HandleAction);

		Toasts.Build(WidgetTree, Canvas);

		ExtractionRoot = ExtractionRootRaw;
		OfferRoot = OfferRootRaw;
		ResultRoot = ResultRootRaw;
		ExtractionRoot->SetVisibility(ESlateVisibility::Collapsed);
		OfferRoot->SetVisibility(ESlateVisibility::Collapsed);
		ResultRoot->SetVisibility(ESlateVisibility::Collapsed);
	}
	return Super::RebuildWidget();
}

void URunHudWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);

	Toasts.Update(GetGameInstance());

	UWorld* World = GetWorld();
	const AHeroCharacter* Hero = Cast<AHeroCharacter>(UGameplayStatics::GetPlayerPawn(this, 0));
	if (!World || !Hero || !HealthText)
	{
		return;
	}

	// Health.
	const float MaxHealth = FMath::Max(Hero->Health->GetMaxHealth(), 1.0f);
	HealthText->SetText(FText::FromString(FString::Printf(TEXT("HP %.0f / %.0f"), Hero->Health->GetHealth(), MaxHealth)));
	HealthBar->SetPercent(Hero->Health->GetHealth() / MaxHealth);

	// Level and XP.
	if (const URunProgressionSubsystem* Progression = World->GetSubsystem<URunProgressionSubsystem>())
	{
		LevelText->SetText(FText::FromString(FString::Printf(TEXT("Level %d   XP %.0f / %.0f"),
			Progression->GetLevel(), Progression->GetXp(), Progression->GetXpToNext())));
		XpBar->SetPercent(Progression->GetXp() / FMath::Max(Progression->GetXpToNext(), 1.0f));
		LoadoutText->SetText(FText::FromString(Progression->GetLoadoutSummary()));
	}

	// Heat.
	if (const ULootHeatSubsystem* Heat = World->GetSubsystem<ULootHeatSubsystem>())
	{
		HeatText->SetText(FText::FromString(FString::Printf(TEXT("Heat %.0f   (enemy pressure x%.2f)"),
			Heat->GetHeat(), Heat->GetSpawnPressureMultiplier())));
		HeatBar->SetPercent(Heat->GetHeatNormalized());
	}

	// Bag.
	BagText->SetText(FText::FromString(FString::Printf(TEXT("%d / %d slots\nWood %d   Stone %d"),
		Hero->Bag->GetUsedSlots(), Hero->Bag->GetCapacitySlots(), Hero->Bag->GetWoodCount(), Hero->Bag->GetStoneCount())));
	FString Reels;
	for (const ERelicType Held : Hero->Bag->GetRelics())
	{
		Reels += FString::Printf(TEXT("Lost reel: %s\n"), *HudRelicLabel(Held));
	}
	RelicText->SetText(FText::FromString(Reels.IsEmpty() ? FString(TEXT("No lost film reel")) : Reels.TrimEnd()));

	// Extraction bar: a map can hold more than one extraction point, so show the furthest-filled one.
	if (!bZonesFound)
	{
		for (TActorIterator<AExtractionZone> It(World); It; ++It)
		{
			Zones.Add(*It);
		}
		bZonesFound = true;
	}
	float Progress = 0.0f;
	for (const TWeakObjectPtr<AExtractionZone>& Point : Zones)
	{
		if (Point.IsValid())
		{
			Progress = FMath::Max(Progress, Point->GetProgress());
		}
	}
	ExtractionBar->SetPercent(Progress);
	ExtractionRoot->SetVisibility(Progress > 0.0f ? ESlateVisibility::SelfHitTestInvisible : ESlateVisibility::Collapsed);

	RefreshOfferPanel();
	RefreshResultPanel();
}

void URunHudWidget::RefreshOfferPanel()
{
	const URunProgressionSubsystem* Progression = GetWorld()->GetSubsystem<URunProgressionSubsystem>();
	const URunOutcomeSubsystem* Run = GetWorld()->GetSubsystem<URunOutcomeSubsystem>();
	const bool bShow = Progression && Progression->IsOfferPending() && Run && Run->IsRunActive();

	if (bShow)
	{
		const TArray<FOfferCardText> Cards = Progression->GetOfferCards();
		const bool bCanCut = Progression->CanCutFromOffer();
		for (int32 Index = 0; Index < CardRoots.Num(); ++Index)
		{
			const bool bHas = Cards.IsValidIndex(Index);
			CardRoots[Index]->SetVisibility(bHas ? ESlateVisibility::Visible : ESlateVisibility::Collapsed);
			if (!bHas)
			{
				continue;
			}
			CardTitles[Index]->SetText(FText::FromString(Cards[Index].Title));
			CardTitles[Index]->SetColorAndOpacity(FSlateColor(Cards[Index].bHighlight ? FLinearColor(1.0f, 0.85f, 0.3f) : FLinearColor::White));
			CardDetails[Index]->SetText(FText::FromString(Cards[Index].Detail));
			CardNotes[Index]->SetText(FText::FromString(Cards[Index].Note));
			CutButtons[Index]->SetVisibility(bCanCut ? ESlateVisibility::Visible : ESlateVisibility::Collapsed);
		}
		const int32 CutsLeft = Progression->GetPermanentCutsLeft();
		CutInfoText->SetText(FText::FromString(Progression->CanCutFromOffer()
			? FString::Printf(TEXT("Permanent cuts left: %d   (one cut per level-up)"), CutsLeft)
			: (CutsLeft > 0 ? FString(TEXT("You cut one. Now take one of the others."))
			                : FString(TEXT("No permanent cuts left.")))));
	}
	OfferRoot->SetVisibility(bShow ? ESlateVisibility::Visible : ESlateVisibility::Collapsed);
}

void URunHudWidget::RefreshResultPanel()
{
	const URunOutcomeSubsystem* Run = GetWorld()->GetSubsystem<URunOutcomeSubsystem>();
	const bool bShow = Run && !Run->IsRunActive();

	if (bShow && !bResultShown)
	{
		const FRunResult Result = Run->GetLastResult();
		FString Text;
		if (Result.Outcome == ERunOutcome::Extracted)
		{
			Text = FString::Printf(TEXT("EXTRACTED\n\nBag wood %d stone %d x %.2f = +%d wood +%d stone to camp\nLost film reels carried out (%d): %s\nUnlocked for every later run: %s"),
				Result.BagWood, Result.BagStone, Result.MultiplierUsed, Result.WoodGained, Result.StoneGained, Result.RelicsBanked.Num(), *HudRelicNames(Result.RelicsBanked),
				*HudOptionNames(Result.OptionsUnlocked));
			ResultText->SetColorAndOpacity(FSlateColor(DirectorUi::GoodColor));
		}
		else
		{
			Text = FString::Printf(TEXT("YOU DIED\n\nLost %d wood, %d stone and %d lost film reels: %s\nThe reels were lost: nothing was unlocked"),
				Result.WoodLost, Result.StoneLost, Result.RelicsLost.Num(), *HudRelicNames(Result.RelicsLost));
			ResultText->SetColorAndOpacity(FSlateColor(DirectorUi::BadColor));
		}
		ResultText->SetText(FText::FromString(Text));
	}
	ResultRoot->SetVisibility(bShow ? ESlateVisibility::Visible : ESlateVisibility::Collapsed);
	bResultShown = bShow;
}

void URunHudWidget::HandleAction(FName Action, int32 Index)
{
	if (Action == TEXT("Offer"))
	{
		if (URunProgressionSubsystem* Progression = GetWorld()->GetSubsystem<URunProgressionSubsystem>())
		{
			Progression->ChooseOption(Index);
		}
	}
	else if (Action == TEXT("Cut"))
	{
		if (URunProgressionSubsystem* Progression = GetWorld()->GetSubsystem<URunProgressionSubsystem>())
		{
			Progression->CutFromOffer(Index);
		}
	}
	else if (Action == TEXT("Camp"))
	{
		UGameplayStatics::OpenLevel(this, TEXT("L_Camp"));
	}
}
