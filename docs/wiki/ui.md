# UI

The output of story 007. Everything is written in C++ with no widget blueprints: each widget builds its tree in `RebuildWidget()` using `WidgetTree->ConstructWidget<T>()`. This way the UI and the code are reviewed in one place, at the cost that changing the look requires recompiling. Paths omit the prefix `Game_01 5.8/Source/Game_01/`.

## 1. Files

| File | Content |
|---|---|
| [Public/UI/DirectorUi.h](../../Game_01%205.8/Source/Game_01/Public/UI/DirectorUi.h), [Private/UI/DirectorUi.cpp](../../Game_01%205.8/Source/Game_01/Private/UI/DirectorUi.cpp) | Shared building blocks: panel, text, progress bar, button, toast area, palette |
| [RunHudWidget](../../Game_01%205.8/Source/Game_01/Private/UI/RunHudWidget.cpp) | Run UI |
| [CampWidget](../../Game_01%205.8/Source/Game_01/Private/UI/CampWidget.cpp) | Camp UI |
| [HudMessageSubsystem](../../Game_01%205.8/Source/Game_01/Private/UI/HudMessageSubsystem.cpp) | Data source for on-screen messages |

## 2. Shared Building Blocks `DirectorUi`

One style (light text, semi-transparent dark panels). When the art is swapped later, only this needs to change.

| Name | Signature | Behavior |
|---|---|---|
| `MakePanel` | `UPanelWidget* MakePanel(UWidgetTree*, UCanvasPanel*, const FUiPlacement&, bool bScroll = false, UWidget** OutRoot = nullptr)` | Adds a bordered panel to the canvas and returns the vertical box (or scroll box) that holds the content. `OutRoot` returns the outermost border, used to show or hide the whole block |
| `FUiPlacement` | `Anchor`, `Alignment`, `Position`, `Size`, `bFillHeight`, `BottomMargin` | A zero component of `Size` means size to content. When `bFillHeight` is true the anchor is fixed to the left and the panel stretches vertically from `Position.Y` to `BottomMargin` above the bottom, ignoring `Anchor` and `Alignment` |
| `AddText` | `UTextBlock* AddText(Tree, Parent, Text, Size = 18, Color)` | Wraps automatically, font size set through `FSlateFontInfo` |
| `AddBar` | `UProgressBar* AddBar(Tree, Parent, Fill, Width, Height)` | Wrapped in a `USizeBox` to fix the size |
| `AddButton` | `UUiActionButton* AddButton(Tree, Parent, Caption, Action, Index = 0)` | The button carries a text label and `Init` has already been called |
| `ReturnFocusToGame` | `void ReturnFocusToGame()` | `SetAllUserFocusToGameViewport()`. It must be called after a button click, otherwise the button keeps keyboard focus and a later Space press (dodge) would "press" it |

`UUiActionButton` (inherits `UButton`): after a click it broadcasts `OnAction(FName Action, int32 Index)`, then calls `ReturnFocusToGame()`. The owner binds `OnAction` to its own `HandleAction`, uses the `Action` name to tell which button it was, and uses `Index` to carry an index (building type, card index).

`FToastPanel`: the toast area at top center, 4 lines of text. Every frame `Update(GameInstance)` takes the not-yet-expired messages from `UHudMessageSubsystem::GetActive()` and shows them, and hides the whole block when there are none.

**Why `RebuildWidget` and not `NativeOnInitialized`**: for a `UUserWidget` subclass without a blueprint, the `WidgetTree` is already usable at `RebuildWidget` time and that is earlier than `NativeConstruct`, so it is the most reliable moment to build the tree.

## 3. Run UI `URunHudWidget`

Created by [`AHeroCharacter::CreateHud`](../../Game_01%205.8/Source/Game_01/Private/Player/HeroCharacter.cpp). Every frame `NativeTick` reads the subsystems to refresh, and it does not store game state.

| Area | Position | Content | Data source |
|---|---|---|---|
| Status | Top left | Health text and red bar, level and XP text and blue bar, heat text (including the spawn pressure multiplier) and orange bar | `UHealthComponent`, `URunProgressionSubsystem`, `ULootHeatSubsystem` |
| Bag | Top right | `used/capacity slots  Iron N`, below it one `Lost reel: <name>` line per lost film reel (for example `Lost reel: Fast Cut`), or `No lost film reel` when there is none | `ULootBagComponent` |
| Toasts | Top center | Short messages for searching, a rejected pickup, etc. | `UHudMessageSubsystem` |
| Extraction bar | Bottom center | `EXTRACTING - stay in the zone` plus a green bar. Shown only when progress is > 0 | `AExtractionZone::GetProgress()` (when it is not found the first time, `TActorIterator` takes the first one) |
| Control hints | Bottom left | `WASD move   Space dodge   E search   1-3 cut an option` | Static text |
| Level-up panel | Center | A title plus 3 buttons. Shown only when `IsOfferPending()` is true and a run is in progress. Extra buttons are collapsed | `URunProgressionSubsystem::GetOfferDescriptions()` |
| Results page | Center | `EXTRACTED` or `YOU DIED`, the numbers, and a `Return to Camp` button. Shown after a run ends. On extraction it prints the Iron gained, the lost film reels carried out and `Unlocked for every later run: <options>`; on death it prints the Iron and reels lost and says the reels were lost and nothing was unlocked | `URunOutcomeSubsystem::GetLastResult()` |

Actions: `Offer` (`PickOffer(Index)`), `Camp` (`OpenLevel(L_Camp)`).

There is no search progress bar: container searching completes instantly (`ALootContainer::Interact` returns synchronously), so only the extraction progress bar was built.

## 4. Camp UI `UCampWidget`

Created by [`ACampPawn::BeginPlay`](../../Game_01%205.8/Source/Game_01/Private/Camp/CampPawn.cpp). On the left is one scroll panel that fills the height (leaving 90 pixels at the bottom), and in the bottom left a separate fixed panel holds START RUN, so it can always be clicked.

Panel content: Iron, the result text of the most recent action, buildings (name, level, current bonus, upgrade button, disabled at max level), a `STORAGE - unlocked upgrade options (N)` list (one line per unlocked option, or `None yet. Carry a lost film reel out alive to unlock one.`; there are no buttons in it), and the editing room (one row per card with a description plus an "add/remove" button).

**State signature**: `MakeSignature()` joins the message text, Iron, the unlocked options, building levels and pending cards into one string. `NativeTick` calls `Rebuild()` to clear and rebuild all rows only when it sees the signature has changed. This avoids having to bind change events for every widget, at the cost that the whole panel is rebuilt on every change and button hover state is lost.

Actions: `Upgrade` (`TryUpgrade`), `Card` (add or remove), `Start` (`OpenLevel(L_Greybox)`).

Clicking a building in the 3D scene also upgrades it (`ACampPawn::OnClick`, using `GetHitResultUnderCursor`), and the result goes to the toast area of `UHudMessageSubsystem`.

## 5. Input Mode

Both UIs use `FInputModeGameAndUI` (`DoNotLock`, `SetHideCursorDuringCapture(false)`) and show the mouse: the UI root widget is set to `SelfHitTestInvisible`, so only buttons and panels take clicks and all other clicks pass through to the 3D scene (needed for clicking buildings in the camp).

## 6. On-Screen Messages `UHudMessageSubsystem`

[HudMessageSubsystem.h](../../Game_01%205.8/Source/Game_01/Public/UI/HudMessageSubsystem.h). A GameInstance subsystem that keeps the most recent 4 messages.

| Function | Behavior |
|---|---|
| `static void Post(const UObject* WorldContext, const FString& Text, const FLinearColor& Color, float Seconds = 2.5f)` | Does nothing when there is no World or GameInstance (for example an editor preview world) |
| `TArray<FHudMessage> GetActive() const` | Filters out expired ones, oldest to newest |

Time uses `FPlatformTime::Seconds()` (real time), so messages also expire while the game is paused. Call sites: `ULootBagComponent` (pickup rejected), `ALootContainer` (searching, already searched), `ACampPawn` (click upgrade).

This message system replaces the engine's `AddOnScreenDebugMessage`, which is always drawn at the top left of the screen and would sit on top of the health bar.
