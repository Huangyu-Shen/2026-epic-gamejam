#include "UI/PauseMenuWidget.h"
#include "UI/DirectorUi.h"
#include "UI/HudMessageSubsystem.h"
#include "Audio/GameAudioSubsystem.h"
#include "Blueprint/WidgetTree.h"
#include "Components/CanvasPanel.h"
#include "Components/PanelWidget.h"
#include "Components/TextBlock.h"
#include "Engine/World.h"
#include "Extraction/RunOutcomeSubsystem.h"
#include "Input/Events.h"
#include "Kismet/GameplayStatics.h"
#include "Player/HeroCharacter.h"

namespace
{
	const TCHAR* BusNames[] = { TEXT("Master"), TEXT("Music"), TEXT("Effects") };
	constexpr int32 BusCount = 3;
	constexpr float VolumeStep = 0.1f;
}

TSharedRef<SWidget> UPauseMenuWidget::RebuildWidget()
{
	if (WidgetTree && !WidgetTree->RootWidget)
	{
		UCanvasPanel* Canvas = WidgetTree->ConstructWidget<UCanvasPanel>();
		WidgetTree->RootWidget = Canvas;

		FUiPlacement Centre;
		Centre.Anchor = FVector2D(0.5, 0.5);
		Centre.Alignment = FVector2D(0.5, 0.5);
		Centre.Position = FVector2D::ZeroVector;
		Centre.Size = FVector2D(440.0, 0.0);
		UWidget* Root = nullptr;
		UPanelWidget* Panel = DirectorUi::MakePanel(WidgetTree, Canvas, Centre, false, &Root);
		PanelRoot = Root;

		DirectorUi::AddText(WidgetTree, Panel, TEXT("PAUSED"), 28, DirectorUi::GoodColor);
		UUiActionButton* Resume = DirectorUi::AddButton(WidgetTree, Panel, TEXT("Resume   (Esc)"), TEXT("Resume"), 0, EUiButtonKind::Primary);
		Resume->OnAction.AddDynamic(this, &UPauseMenuWidget::HandleAction);
		UUiActionButton* Settings = DirectorUi::AddButton(WidgetTree, Panel, TEXT("Settings"), TEXT("Settings"));
		Settings->OnAction.AddDynamic(this, &UPauseMenuWidget::HandleAction);

		// Settings: three volume rows, hidden until the Settings button is pressed.
		SettingsWidgets.Reset();
		VolumeTexts.Reset();
		for (int32 Bus = 0; Bus < BusCount; ++Bus)
		{
			UPanelWidget* Row = DirectorUi::AddRow(WidgetTree, Panel);
			UTextBlock* Text = DirectorUi::AddText(WidgetTree, Row, TEXT(""), 18);
			UUiActionButton* Down = DirectorUi::AddButton(WidgetTree, Row, TEXT("-"), TEXT("Volume"), Bus * 2);
			UUiActionButton* Up = DirectorUi::AddButton(WidgetTree, Row, TEXT("+"), TEXT("Volume"), Bus * 2 + 1);
			Down->OnAction.AddDynamic(this, &UPauseMenuWidget::HandleAction);
			Up->OnAction.AddDynamic(this, &UPauseMenuWidget::HandleAction);
			VolumeTexts.Add(Text);
			SettingsWidgets.Add(Row);
		}

		if (Context == EPauseContext::Run)
		{
			ReturnButton = DirectorUi::AddButton(WidgetTree, Panel, TEXT("Return to camp (lose bag)"), TEXT("Return"), 0, EUiButtonKind::Danger);
			ReturnButton->OnAction.AddDynamic(this, &UPauseMenuWidget::HandleAction);
		}
		UUiActionButton* Quit = DirectorUi::AddButton(WidgetTree, Panel, TEXT("Quit game"), TEXT("Quit"), 0, EUiButtonKind::Danger);
		Quit->OnAction.AddDynamic(this, &UPauseMenuWidget::HandleAction);

		// The controls, so the menu is also the place to look them up.
		DirectorUi::AddText(WidgetTree, Panel, TEXT("\nControls"), 14, DirectorUi::DimColor);
		DirectorUi::AddText(WidgetTree, Panel, Context == EPauseContext::Run
			? TEXT("WASD move   Space dodge   E search\n1-3 take an upgrade   4-6 cut one   Esc menu")
			: TEXT("Click a building to upgrade it\nPick film cards, then START RUN   Esc menu"), 14, DirectorUi::DimColor);

		// Credits. Two of the packs are CC BY: music by Kounine, and the WW2 buildings by Winyunq (scaled for the game).
		DirectorUi::AddText(WidgetTree, Panel, TEXT("\nCredits"), 12, DirectorUi::DimColor);
		UTextBlock* Credits = DirectorUi::AddText(WidgetTree, Panel,
			TEXT("Music: \"Path of Adventure\" by Kounine (CC BY). Buildings: WW2 RTS & SLG Production Buildings Pack by Winyunq (CC BY 4.0, scaled). House: Cozy Low Poly House by Demolition Studios. Heroes, monsters and forest: Dungeon Mason. Sounds: 50 Free Game Sounds Pack by PlaceHolder Inc. All via Fab."), 11, DirectorUi::DimColor);
		Credits->SetAutoWrapText(true);

		PanelRoot->SetVisibility(ESlateVisibility::Collapsed);
		ShowSettings(false);
		RefreshVolumeTexts();
	}
	return Super::RebuildWidget();
}

void UPauseMenuWidget::Toggle()
{
	if (bOpen)
	{
		Close();
	}
	else
	{
		Open();
	}
}

void UPauseMenuWidget::Open()
{
	if (bOpen || !PanelRoot)
	{
		return;
	}
	bOpen = true;
	bReturnArmed = false;
	if (ReturnButton)
	{
		ReturnButton->Label->SetText(FText::FromString(TEXT("Return to camp (lose bag)")));
	}
	ShowSettings(false);
	RefreshVolumeTexts();
	PanelRoot->SetVisibility(ESlateVisibility::Visible);
	if (Context == EPauseContext::Run)
	{
		UGameplayStatics::SetGamePaused(this, true);
	}
}

void UPauseMenuWidget::Close()
{
	if (!bOpen)
	{
		return;
	}
	bOpen = false;
	PanelRoot->SetVisibility(ESlateVisibility::Collapsed);
	UGameplayStatics::SetGamePaused(this, false);
	DirectorUi::ReturnFocusToGame();
}

void UPauseMenuWidget::ShowSettings(bool bShow)
{
	bSettingsShown = bShow;
	for (UWidget* Row : SettingsWidgets)
	{
		Row->SetVisibility(bShow ? ESlateVisibility::Visible : ESlateVisibility::Collapsed);
	}
}

void UPauseMenuWidget::RefreshVolumeTexts()
{
	for (int32 Bus = 0; Bus < VolumeTexts.Num(); ++Bus)
	{
		const float Volume = UGameAudioSubsystem::GetBusVolume(static_cast<EAudioBus>(Bus));
		VolumeTexts[Bus]->SetText(FText::FromString(FString::Printf(TEXT("%s volume  %d%%"), BusNames[Bus], FMath::RoundToInt(Volume * 100.0f))));
	}
}

void UPauseMenuWidget::PressReturnToCamp()
{
	URunOutcomeSubsystem* Run = GetWorld()->GetSubsystem<URunOutcomeSubsystem>();
	const AHeroCharacter* Hero = Cast<AHeroCharacter>(UGameplayStatics::GetPlayerPawn(this, 0));
	if (!Run || !Hero)
	{
		return;
	}

	// A run that already ended has nothing left to lose: go straight back.
	if (Run->IsRunActive())
	{
		if (!bReturnArmed)
		{
			bReturnArmed = true;
			ReturnArmedAt = GetWorld()->GetRealTimeSeconds();
			if (ReturnButton)
			{
				ReturnButton->Label->SetText(FText::FromString(TEXT("Click again: the bag is lost")));
			}
			return;
		}
		// Abandoning a run is a loss, exactly as if the hero had died. The message follows the player to the camp.
		Run->ReportPlayerDied(Hero->Bag);
		UHudMessageSubsystem::Post(this, TEXT("Run abandoned: the bag was lost"), DirectorUi::BadColor, 5.0f);
	}
	UGameplayStatics::SetGamePaused(this, false);
	UGameplayStatics::OpenLevel(this, TEXT("L_Camp"));
}

void UPauseMenuWidget::HandleAction(FName Action, int32 Index)
{
	if (Action == TEXT("Resume"))
	{
		Close();
	}
	else if (Action == TEXT("Settings"))
	{
		ShowSettings(!bSettingsShown);
	}
	else if (Action == TEXT("Volume"))
	{
		const EAudioBus Bus = static_cast<EAudioBus>(Index / 2);
		const float Delta = (Index % 2 == 0) ? -VolumeStep : VolumeStep;
		UGameAudioSubsystem::SetBusVolume(this, Bus, UGameAudioSubsystem::GetBusVolume(Bus) + Delta);
		RefreshVolumeTexts();
	}
	else if (Action == TEXT("Return"))
	{
		PressReturnToCamp();
	}
	else if (Action == TEXT("Quit"))
	{
		FPlatformMisc::RequestExit(false);
	}
}

void UPauseMenuWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);

	// The confirmation lapses after three seconds (real time: the game is paused while the menu is open).
	if (bReturnArmed && GetWorld()->GetRealTimeSeconds() - ReturnArmedAt > 3.0)
	{
		bReturnArmed = false;
		if (ReturnButton)
		{
			ReturnButton->Label->SetText(FText::FromString(TEXT("Return to camp (lose bag)")));
		}
	}
}

FReply UPauseMenuWidget::NativeOnKeyDown(const FGeometry& MyGeometry, const FKeyEvent& InKeyEvent)
{
	// After a click the keyboard focus sits on a button, so Esc arrives here instead of at the pawn.
	if (bOpen && InKeyEvent.GetKey() == EKeys::Escape)
	{
		Close();
		return FReply::Handled();
	}
	return Super::NativeOnKeyDown(MyGeometry, InKeyEvent);
}
