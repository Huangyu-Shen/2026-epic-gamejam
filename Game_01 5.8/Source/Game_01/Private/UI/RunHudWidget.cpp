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
		DirectorUi::AddText(WidgetTree, Help, TEXT("WASD move   Space dodge   E search a crate   1-6 use a relic"), 14, DirectorUi::DimColor);

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
		Centre.Size = FVector2D(640.0, 0.0); // wide enough for "[2] CUT   Swiftness  Lv5 -> Lv6   (3 more to 3-STAR)"
		UPanelWidget* Offer = DirectorUi::MakePanel(WidgetTree, Canvas, Centre, false, &OfferRootRaw);
		DirectorUi::AddText(WidgetTree, Offer, TEXT("LEVEL UP - CUT one, keep the other two"), 20, DirectorUi::GoodColor);
		DirectorUi::AddText(WidgetTree, Offer, TEXT("Click or press 1-3 on the one to cut"), 14, DirectorUi::DimColor);
		for (int32 Index = 0; Index < 3; ++Index)
		{
			UUiActionButton* Button = DirectorUi::AddButton(WidgetTree, Offer, TEXT(""), TEXT("Offer"), Index);
			Button->OnAction.AddDynamic(this, &URunHudWidget::HandleAction);
			OfferButtons.Add(Button);
		}
		CutInfoText = DirectorUi::AddText(WidgetTree, Offer, TEXT(""), 14, DirectorUi::DimColor);

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
	BagText->SetText(FText::FromString(FString::Printf(TEXT("%d / %d slots    Iron %d"),
		Hero->Bag->GetUsedSlots(), Hero->Bag->GetCapacitySlots(), Hero->Bag->GetIronCount())));
	FString Relics;
	const TArray<ERelicType> Held = Hero->Bag->GetRelics();
	for (int32 Index = 0; Index < Held.Num(); ++Index)
	{
		Relics += FString::Printf(TEXT("[%d] %s\n"), Index + 1, *HudRelicLabel(Held[Index]));
	}
	RelicText->SetText(FText::FromString(Relics.IsEmpty() ? FString(TEXT("No relics")) : Relics.TrimEnd()));

	// Extraction bar.
	if (!Zone.IsValid())
	{
		for (TActorIterator<AExtractionZone> It(World); It; ++It)
		{
			Zone = *It;
			break;
		}
	}
	const float Progress = Zone.IsValid() ? Zone->GetProgress() : 0.0f;
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
		const TArray<FString> Lines = Progression->GetOfferDescriptions();
		for (int32 Index = 0; Index < OfferButtons.Num(); ++Index)
		{
			UUiActionButton* Button = OfferButtons[Index];
			const bool bHas = Lines.IsValidIndex(Index);
			Button->SetVisibility(bHas ? ESlateVisibility::Visible : ESlateVisibility::Collapsed);
			if (bHas)
			{
				Button->Label->SetText(FText::FromString(FString::Printf(TEXT("[%d] CUT   %s"), Index + 1, *Lines[Index])));
			}
		}
		const int32 CutsLeft = Progression->GetPermanentCutsLeft();
		CutInfoText->SetText(FText::FromString(CutsLeft > 0
			? FString::Printf(TEXT("Permanent cuts left: %d - what you cut never comes back this run"), CutsLeft)
			: FString(TEXT("No permanent cuts left - what you cut is only skipped this time"))));
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
			Text = FString::Printf(TEXT("EXTRACTED\n\nBag Iron %d x %.2f = +%d Iron to camp\nRelics banked (%d): %s"),
				Result.BagIron, Result.MultiplierUsed, Result.IronGained, Result.RelicsBanked.Num(), *HudRelicNames(Result.RelicsBanked));
			ResultText->SetColorAndOpacity(FSlateColor(DirectorUi::GoodColor));
		}
		else
		{
			Text = FString::Printf(TEXT("YOU DIED\n\nLost %d Iron and %d relics: %s"),
				Result.IronLost, Result.RelicsLost.Num(), *HudRelicNames(Result.RelicsLost));
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
			Progression->CutOffer(Index);
		}
	}
	else if (Action == TEXT("Camp"))
	{
		UGameplayStatics::OpenLevel(this, TEXT("L_Camp"));
	}
}
