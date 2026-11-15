# UI

The output of story 007, updated by story 008 (lost film reels, level-up "cut one, keep two") and story 009 (the `NEW! ` marker in the level-up panel). Everything is written in C++ with no widget blueprints: each widget builds its tree in `RebuildWidget()` using `WidgetTree->ConstructWidget<T>()`, guarded by `!WidgetTree->RootWidget` so the tree is built only once. This way the UI and the code are reviewed in one place, at the cost that changing the look requires recompiling. Paths omit the prefix `Game_01 5.8/Source/Game_01/`.

## 1. Files

| File | Content |
|---|---|
| [Public/UI/DirectorUi.h](../../Game_01%205.8/Source/Game_01/Public/UI/DirectorUi.h), [Private/UI/DirectorUi.cpp](../../Game_01%205.8/Source/Game_01/Private/UI/DirectorUi.cpp) | Shared building blocks: panel, text, progress bar, button, toast area, palette |
| [RunHudWidget](../../Game_01%205.8/Source/Game_01/Private/UI/RunHudWidget.cpp) ([header](../../Game_01%205.8/Source/Game_01/Public/UI/RunHudWidget.h)) | Run UI |
| [CampWidget](../../Game_01%205.8/Source/Game_01/Private/UI/CampWidget.cpp) ([header](../../Game_01%205.8/Source/Game_01/Public/UI/CampWidget.h)) | Camp UI |
| [HudMessageSubsystem](../../Game_01%205.8/Source/Game_01/Private/UI/HudMessageSubsystem.cpp) ([header](../../Game_01%205.8/Source/Game_01/Public/UI/HudMessageSubsystem.h)) | Data source for on-screen messages |

## 2. Shared Building Blocks `DirectorUi`

One style (light text, semi-transparent dark panels: border color 0.02, 0.02, 0.04 at 0.72 alpha, padding 12). When the art is swapped later, only this needs to change. Palette in the `DirectorUi` namespace: `DimColor` (grey), `GoodColor` (green), `BadColor` (red) and `TextColor`, which nothing uses (`AddText` defaults to `FLinearColor::White`).

| Name | Signature | Behavior |
|---|---|---|
| `MakePanel` | `UPanelWidget* MakePanel(UWidgetTree*, UCanvasPanel*, const FUiPlacement&, bool bScroll = false, UWidget** OutRoot = nullptr)` | Adds a bordered panel to the canvas and returns the vertical box (or scroll box) that holds the content. `OutRoot` returns the outermost border, used to show or hide the whole block |
| `FUiPlacement` | `Anchor`, `Alignment`, `Position` (default 16, 16), `Size`, `bFillHeight`, `BottomMargin` | A zero component of `Size` means size to content. When `bFillHeight` is true the anchor is fixed to the left, `Position.X` is the left offset, `Size.X` the width, and the panel stretches vertically from `Position.Y` to `BottomMargin` above the bottom, ignoring `Anchor` and `Alignment` |
| `AddText` | `UTextBlock* AddText(Tree, Parent, Text, Size = 18, Color = White)` | Wraps automatically, font size set through `FSlateFontInfo` |
| `AddBar` | `UProgressBar* AddBar(Tree, Parent, Fill, Width, Height)` | Wrapped in a `USizeBox` to fix the size |
| `AddButton` | `UUiActionButton* AddButton(Tree, Parent, Caption, Action, Index = 0)` | The button carries a text label (size 16, `Button->Label`) and `Init` has already been called |
| `ReturnFocusToGame` | `void ReturnFocusToGame()` | `SetAllUserFocusToGameViewport()`. It must be called after a button click, otherwise the button keeps keyboard focus and a later Space press (dodge) would "press" it |

`UUiActionButton` (inherits `UButton`): after a click it broadcasts `OnAction(FName Action, int32 Index)`, then calls `ReturnFocusToGame()`. The owner binds `OnAction` to its own `HandleAction`, uses the `Action` name to tell which button it was, and uses `Index` to carry an index (building type, card index, offer entry).

`FToastPanel`: the toast area at top center (520 wide, 16 down), `UHudMessageSubsystem::MaxMessages` (4) lines of text. Every frame `Update(const UGameInstance*)` takes the not-yet-expired messages from `UHudMessageSubsystem::GetActive()` and shows them, and hides the whole block when there are none.

## 3. Run UI `URunHudWidget`

Created by [`AHeroCharacter::CreateHud`](../../Game_01%205.8/Source/Game_01/Private/Player/HeroCharacter.cpp). Every frame `NativeTick` reads the subsystems to refresh, and it does not store game state (the only state it keeps is `bResultShown`, so the result text is built once per run end).

| Area | Position | Content | Data source |
|---|---|---|---|
| Status | Top left (320 wide) | `HP cur / max` and a red bar; `Level N   XP x / y` and a blue bar; a dim loadout line such as `Blade Lv3**   Might Lv1*` (one asterisk per star); `Heat N   (enemy pressure xN.NN)` and an orange bar | `UHealthComponent`, `URunProgressionSubsystem` (`GetLoadoutSummary()`), `ULootHeatSubsystem` |
| Bag | Top right (300 wide) | Heading `BAG`, then `used / capacity slots    Iron N`, below it one `Lost reel: <name>` line per lost film reel (for example `Lost reel: Fast Cut`), or `No lost film reel` when there is none. The name is the `ERelicType` display name, not `FRelicRow::DisplayName` | `ULootBagComponent` |
| Toasts | Top center | Short messages for searching, a rejected pickup, a star-up, a permanent cut, etc. | `UHudMessageSubsystem` |
| Extraction bar | Bottom center (420 wide, 80 above the bottom) | `EXTRACTING - stay in the zone` plus a green bar. Shown only when progress is > 0 | `AExtractionZone::GetProgress()` (looked up with `TActorIterator`, first one found, until it is found) |
| Control hints | Bottom left | `WASD move   Space dodge   E search   1-3 take an upgrade   4-6 cut one   Esc menu` | Static text |
| Level-up panel | Center | **Changed in v3.** Title `LEVEL UP`, the hint `Take one upgrade. Cutting one first removes it from your pool for good and gives it to the Boss.`, three cards (title, level line, star-progress line) each with a green `TAKE [n]` and a red `Cut [n+3]` button, and an info line with the permanent cuts left. The panel was redrawn in story 011; any older layout described elsewhere in this page no longer applies |
| Results page | Center (520 wide) | A `Return to Camp` button under the text, shown after a run ends. On extraction (green): `EXTRACTED`, `Bag Iron N x M = +G Iron to camp`, `Lost film reels carried out (n): <names or none>`, `Unlocked for every later run: <options or none>`. On death (red): `YOU DIED`, `Lost N Iron and n lost film reels: <names or none>`, `The reels were lost: nothing was unlocked` | `URunOutcomeSubsystem::GetLastResult()` |

Option descriptions come from `DescribeOption`: `NEW weapon: Bolt` for a weapon the run does not own, otherwise `<Name>  Lv<a> -> Lv<b>` followed by `   >> 2-STAR!` when that copy makes a star-up, or `   (n more to 2-STAR)` while a star is still missing, and nothing at 3 stars. Story 009 added the `NEW! ` prefix for an option that a lost film reel unlocked since the player last saw an offer (`FOfferOption::bIsNewUnlock`; the next run's first offer always contains it): a passive reads `[1] CUT   NEW! Haste  Lv0 -> Lv1   (2 more to 2-STAR)`, and a weapon that is new to both the camp and the run reads `NEW! Chain  (unlocked weapon)` in place of `NEW weapon: Chain`. The panel only shows the text; the marker is set in `URunProgressionSubsystem::GenerateOffer`. The loadout line in the status block also lists Chain, Haste and Magnet once the run owns them (`GetLoadoutSummary` walks every weapon and passive).

Actions: `Offer` (`ChooseOption(Index)` for TAKE and `CutFromOffer(Index)` for Cut, the same calls the 1-3 and 4-6 keys make from `AHeroCharacter`), `Camp` (`OpenLevel(L_Camp)`).

There is no search progress bar: container searching completes instantly (`ALootContainer::Interact` returns synchronously), so only the extraction progress bar was built.

## 4. Camp UI `UCampWidget`

Created by [`ACampPawn::BeginPlay`](../../Game_01%205.8/Source/Game_01/Private/Camp/CampPawn.cpp). On the left is one scroll panel (460 wide) that fills the height (leaving 90 pixels at the bottom), and in the bottom left a separate fixed panel holds START RUN, so it can always be clicked. A toast area (`FToastPanel`) at the top center shows messages from `UHudMessageSubsystem`.

Panel content, top to bottom: `CAMP` title, `Iron: N`, the result text of the most recent action (green, or red for an error), `BUILDINGS (you can also click them in the scene)`, for each building a line `<Name>   Lv N / Max   (weapon damage +X)` or `(max health +X)` and an upgrade button `Upgrade  (N Iron)` (disabled and captioned `Maximum level` at max level), a `STORAGE - unlocked upgrade options (N)` list (one green line per unlocked option, or `None yet. Carry a lost film reel out alive to unlock one.`; there are no buttons in it), and `EDITING ROOM  (film cards n / 2 - harder run, more Iron)` with one row per card (its description plus an `Add to the run` or `Remove from the run` button). The editing room is skipped when there is no `UFilmCardSubsystem`.

Action messages: `<Building> upgraded to level N`, the refusal reason from `TryUpgrade`, `Card removed`, `Card added to the next run`, `Both film card slots are full`.

**State signature**: `MakeSignature()` joins the message text, Iron, the unlocked options, building levels and pending cards into one string. `NativeTick` calls `Rebuild()` to clear and rebuild all rows only when it sees the signature has changed. This avoids having to bind change events for every widget, at the cost that the whole panel is rebuilt on every change and button hover state is lost.

Actions: `Upgrade` (`TryUpgrade`), `Card` (add or remove), `Start` (`OpenLevel(L_Greybox)`).

Clicking a building in the 3D scene also upgrades it (`ACampPawn::OnClick`, left mouse button, using `GetHitResultUnderCursor`), and the result goes to the toast area of `UHudMessageSubsystem` (not to the panel's result line).

## 5. Input Mode

Both UIs use `FInputModeGameAndUI` (`DoNotLock`, `SetHideCursorDuringCapture(false)`) and show the mouse (set by `AHeroCharacter::CreateHud` and `ACampPawn::BeginPlay`, not by the widgets): the UI root widget is set to `SelfHitTestInvisible`, so only buttons and panels take clicks and all other clicks pass through to the 3D scene (needed for clicking buildings in the camp).

## 6. On-Screen Messages `UHudMessageSubsystem`

[HudMessageSubsystem.h](../../Game_01%205.8/Source/Game_01/Public/UI/HudMessageSubsystem.h). A GameInstance subsystem that keeps the most recent 4 messages.

| Function | Behavior |
|---|---|
| `static void Post(const UObject* WorldContext, const FString& Text, const FLinearColor& Color = White, float Seconds = 2.5f)` | Does nothing when there is no World or GameInstance (for example an editor preview world) |
| `TArray<FHudMessage> GetActive() const` | Filters out expired ones, oldest to newest |

Time uses `FPlatformTime::Seconds()` (real time), so messages also expire while the game is paused. Call sites: `ULootBagComponent` (pickup rejected), `ALootContainer` (searched, already searched), `ACampPawn` (click upgrade), `URunProgressionSubsystem` (`<NAME> is now N-STAR!`, `CUT for good: <name>`).

This message system replaces the engine's `AddOnScreenDebugMessage`, which is always drawn at the top left of the screen and would sit on top of the health bar.
