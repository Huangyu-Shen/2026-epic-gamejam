# UI

Everything is written in C++ with no widget blueprints: each widget builds its tree in `RebuildWidget()` using `WidgetTree->ConstructWidget<T>()`, guarded by `!WidgetTree->RootWidget` so the tree is built only once. This way the UI and the code are reviewed in one place, at the cost that changing the look requires recompiling. Checked against the source on 2026-11-15 (HEAD `bfe9e16`). Paths omit the prefix `Game_01 5.8/Source/Game_01/`.

There are four screens: the run HUD with the level-up panel and the result page (`URunHudWidget`), the camp screen (`UCampWidget`), the Esc menu (`UPauseMenuWidget`, used in both), and the toast area that the first two share (`FToastPanel`, fed by `UHudMessageSubsystem`).

## 1. Files

| File | Content |
|---|---|
| [Public/UI/DirectorUi.h](../../Game_01%205.8/Source/Game_01/Public/UI/DirectorUi.h), [Private/UI/DirectorUi.cpp](../../Game_01%205.8/Source/Game_01/Private/UI/DirectorUi.cpp) | Shared building blocks: panel, text, progress bar, button, row, card, toast area, palette |
| [RunHudWidget](../../Game_01%205.8/Source/Game_01/Private/UI/RunHudWidget.cpp) ([header](../../Game_01%205.8/Source/Game_01/Public/UI/RunHudWidget.h)) | Run UI: HUD, bag, extraction bar, level-up panel, result page |
| [CampWidget](../../Game_01%205.8/Source/Game_01/Private/UI/CampWidget.cpp) ([header](../../Game_01%205.8/Source/Game_01/Public/UI/CampWidget.h)) | Camp UI |
| [PauseMenuWidget](../../Game_01%205.8/Source/Game_01/Private/UI/PauseMenuWidget.cpp) ([header](../../Game_01%205.8/Source/Game_01/Public/UI/PauseMenuWidget.h)) | The Esc menu (camp and run) |
| [HudMessageSubsystem](../../Game_01%205.8/Source/Game_01/Private/UI/HudMessageSubsystem.cpp) ([header](../../Game_01%205.8/Source/Game_01/Public/UI/HudMessageSubsystem.h)) | Data source for on-screen messages |

## 2. Shared Building Blocks `DirectorUi`

One style: light text on dark rounded panels. When the art is swapped later, only this needs to change. A panel is a `UBorder` with a rounded-box brush (fill 0.03, 0.035, 0.06 at 0.95 alpha, outline 0.55, 0.6, 0.75 at 0.9 alpha and width 2, corner radius 10) and padding 16. Palette in the `DirectorUi` namespace: `TextColor` (light grey 0.92), `DimColor` (grey 0.6), `GoodColor` (green) and `BadColor` (red); `AddText` itself defaults to `FLinearColor::White`.

| Name | Signature | Behavior |
|---|---|---|
| `MakePanel` | `UPanelWidget* MakePanel(UWidgetTree*, UCanvasPanel*, const FUiPlacement&, bool bScroll = false, UWidget** OutRoot = nullptr)` | Adds a bordered panel to the canvas and returns the vertical box (or scroll box when `bScroll`) that holds the content. `OutRoot` returns the outermost border, used to show or hide the whole block |
| `FUiPlacement` | `Anchor`, `Alignment`, `Position` (default 16, 16), `Size`, `bFillHeight`, `BottomMargin` | A zero component of `Size` means size to content. When `bFillHeight` is true the anchor is fixed to the left, `Position.X` is the left offset, `Size.X` the width, and the panel stretches vertically from `Position.Y` to `BottomMargin` above the bottom, ignoring `Anchor` and `Alignment` |
| `AddText` | `UTextBlock* AddText(Tree, Parent, Text, Size = 18, Color = White)` | Wraps automatically, font size set through `FSlateFontInfo` |
| `AddBar` | `UProgressBar* AddBar(Tree, Parent, Fill, Width, Height)` | Wrapped in a `USizeBox` to fix the size |
| `AddButton` | `UUiActionButton* AddButton(Tree, Parent, Caption, Action, Index = 0, EUiButtonKind Kind = Normal)` | The button carries a bold text label (size 18, padding 16 by 9, `Button->Label`) and `Init` has already been called. `EUiButtonKind`: `Normal` (blue-grey), `Primary` (green, for TAKE, START RUN and Resume) and `Danger` (red, for Cut, Return to camp and Quit). Hovered, pressed and disabled looks are set |
| `AddRow` | `UPanelWidget* AddRow(Tree, Parent)` | A horizontal box added to a vertical box (cards side by side, the volume rows) |
| `AddCard` | `UPanelWidget* AddCard(Tree, Row, Accent, Width, UWidget** OutRoot)` | A fixed-width bordered card (fill 0.07, 0.08, 0.13, outline `Accent`, padding 14) in a row; returns the vertical box for its content |
| `ReturnFocusToGame` | `void ReturnFocusToGame()` | `SetAllUserFocusToGameViewport()`. It must be called after a button click, otherwise the button keeps keyboard focus and a later Space press (dodge) would "press" it |

`UUiActionButton` (inherits `UButton`): after a click it broadcasts `OnAction(FName Action, int32 Index)`, then calls `ReturnFocusToGame()`. The owner binds `OnAction` to its own `HandleAction`, uses the `Action` name to tell which button it was, and uses `Index` to carry an index (building type, card index, offer entry, volume bus and direction).

`FToastPanel`: the toast area at top center (520 wide, 16 down), `UHudMessageSubsystem::MaxMessages` (4) lines of text. Every frame `Update(const UGameInstance*)` takes the not-yet-expired messages from `UHudMessageSubsystem::GetActive()` and shows them with their own colors, and collapses the whole block when there are none.

## 3. Run UI `URunHudWidget`

Created by [`AHeroCharacter::CreateHud`](../../Game_01%205.8/Source/Game_01/Private/Player/HeroCharacter.cpp) (once, from `BeginPlay`, together with the Esc menu). Every frame `NativeTick` reads the subsystems to refresh, and it does not store game state (the only state it keeps is `bResultShown`, so the result text is built once per run end).

| Area | Position | Content | Data source |
|---|---|---|---|
| Status | Top left (320 wide) | `HP cur / max` and a red bar; `Level N   XP x / y` and a blue bar; a dim loadout line such as `Blade Lv3**   Might Lv1*` (one asterisk per star, every owned weapon and passive); `Heat N   (enemy pressure xN.NN)` and an orange bar | `UHealthComponent`, `URunProgressionSubsystem` (`GetLoadoutSummary()`), `ULootHeatSubsystem` |
| Bag | Top right (300 wide) | Heading `BAG`, then `used / capacity slots` and, on a second line, `Wood N   Stone N`, below it one `Lost reel: <name>` line per lost film reel (for example `Lost reel: Fast Cut`), or `No lost film reel` when there is none. The name is the `ERelicType` display name, not `FRelicRow::DisplayName` | `ULootBagComponent` (`GetWoodCount`, `GetStoneCount`, `GetRelics`) |
| Toasts | Top center | Short messages, see section 6 | `UHudMessageSubsystem` |
| Extraction bar | Bottom center (420 wide, 80 above the bottom) | `EXTRACTING - stay in the zone` plus a green bar. Shown only when progress is above 0. A map can hold more than one extraction point, so the loop over all `AExtractionZone` actors (`TActorIterator`, every frame) shows the furthest-filled one | `AExtractionZone::GetProgress()` |
| Control hints | Bottom left | `WASD move   Space dodge   E search   1-3 take an upgrade   4-6 cut one   Esc menu` | Static text |
| Level-up panel | Center (960 wide) | See below. Shown only while an offer is pending and the run is active | `URunProgressionSubsystem` |
| Results page | Center (520 wide) | A `Return to Camp` button under the text, shown once the run is no longer active. On extraction (green): `EXTRACTED`, `Bag wood W stone S x M = +G wood +H stone to camp`, `Lost film reels carried out (n): <names or none>`, `Unlocked for every later run: <options or none>`. On death (red): `YOU DIED`, `Lost W wood, S stone and n lost film reels: <names or none>`, `The reels were lost: nothing was unlocked` | `URunOutcomeSubsystem::GetLastResult()` |

**Level-up panel.** Title `LEVEL UP`, the hint `Take one upgrade. Cutting one first removes it from your pool for good and gives it to the Boss.`, then a row of three cards (290 wide each). A card holds a title (26 pt), a detail line (20 pt), a note line (16 pt, gold), a green `TAKE   [n]` button (n is 1 to 3) and a red `Cut   [n+3]` button (4 to 6). Under the row an info line: `Permanent cuts left: N   (one cut per level-up)` while a cut is possible, `You cut one. Now take one of the others.` after the cut when cuts remain, `No permanent cuts left.` otherwise. The text of a card comes from `URunProgressionSubsystem::GetOfferCards()` (`FOfferCardText`: `Title`, `Detail`, `Note`, `bHighlight`):

- Title: the option's display name, prefixed with `NEW!  ` when a lost film reel unlocked it since the player last saw an offer (`FOfferOption::bIsNewUnlock`; the next run's first offer always contains it). The title is gold when `bHighlight` is set (NEW, or a star-up).
- Detail: `New weapon` (a weapon the run does not own), `Unlocked weapon` (the same, but newly unlocked by a reel), otherwise `Lv<a>  >  Lv<b>`.
- Note: `<n>-STAR UP!` when this copy makes a star-up of an owned option, `<n> more to <m>-STAR` while a star is still missing, and nothing at 3 stars.
- The Cut buttons are hidden when `CanCutFromOffer()` is false (one cut per level-up, cuts left, more than one card). After a cut the offer holds two cards and the third card is hidden. When the pool has fewer than three options the offer is never shown: the remaining options are applied at once (`GenerateOffer`).

The older one-line text (`DescribeOption`, `GetOfferDescriptions`) still exists in `URunProgressionSubsystem` but the panel does not use it. The loadout line in the status block lists every owned weapon and passive, including Chain, Haste and Magnet once the run owns them.

Actions: `Offer` (`ChooseOption(Index)` for TAKE and `CutFromOffer(Index)` for Cut, the same calls the 1-3 and 4-6 keys make from `AHeroCharacter`), `Camp` (`OpenLevel(L_Camp)`). Keys 1-6 are bound on the hero as raw key bindings (`BindWeakLambda` on `FInputKeyBinding`); outside an offer they do nothing.

There is no search progress bar: container searching completes instantly (`ALootContainer::Interact` returns synchronously), so only the extraction progress bar was built.

## 4. Camp UI `UCampWidget`

Created by [`ACampPawn::BeginPlay`](../../Game_01%205.8/Source/Game_01/Private/Camp/CampPawn.cpp). On the left is one scroll panel (460 wide) that fills the height (leaving 100 pixels at the bottom), in the bottom left a separate fixed panel (460 wide) holds the green START RUN button so it can always be clicked, and alone in the top right corner a `MENU   (Esc)` button opens the Esc menu (`ACampPawn::TogglePauseMenu`). A toast area (`FToastPanel`) at the top center shows messages from `UHudMessageSubsystem`.

Panel content, top to bottom: `CAMP` title, `Wood: N     Stone: N` (green), the result text of the most recent action (green, or red for an error), `BUILDINGS (you can also click them in the scene)`, for each building a line `<Name>   Lv N / Max   (weapon damage +X)` or `(max health +X)` and an upgrade button `Upgrade  (N stone)` for the smithy or `Upgrade  (N wood)` for the clinic (`UCampMetaSubsystem::GetCostResource`; disabled and captioned `Maximum level` at max level), a `STORAGE - unlocked upgrade options (N)` list (one green line per unlocked option, or `None yet. Carry a lost film reel out alive to unlock one.`; there are no buttons in it), and `EDITING ROOM  (film cards n / 2 - harder run, more loot)` with one row per card (its description plus an `Add to the run` or `Remove from the run` button). The editing room is skipped when there is no `UFilmCardSubsystem`.

Action messages: `<Building> upgraded to level N`, the refusal reason from `TryUpgrade` (`Not enough stone (need N)`, `Not enough wood (need N)` or `<Building> is already at max level`), `Card removed`, `Card added to the next run`, `Both film card slots are full`.

**State signature**: `MakeSignature()` joins the message text, wood, stone, the unlocked options, building levels and pending cards into one string. `NativeTick` calls `Rebuild()` to clear and rebuild all rows only when it sees the signature has changed. This avoids having to bind change events for every widget, at the cost that the whole panel is rebuilt on every change and button hover state is lost. The signature does not include the NEW markers, which the camp does not show.

Actions: `Upgrade` (`TryUpgrade`), `Card` (add or remove), `Menu` (open the Esc menu), `Start` (`OpenLevel(L_Greybox)`).

Clicking a building in the 3D scene also upgrades it (`ACampPawn::OnClick`, left mouse button, using `GetHitResultUnderCursor`; ignored while the Esc menu is open), and the result goes to the toast area of `UHudMessageSubsystem` (not to the panel's result line).

## 5. Input Mode

Both screens use `FInputModeGameAndUI` (`DoNotLock`, `SetHideCursorDuringCapture(false)`) and show the mouse (set by `AHeroCharacter::CreateHud` and `ACampPawn::BeginPlay`, not by the widgets): the UI root widget is set to `SelfHitTestInvisible`, so only buttons and panels take clicks and all other clicks pass through to the 3D scene (needed for clicking buildings in the camp). The Esc menu widget is added at z-order 50, above the HUD or the camp screen, and its panel stays collapsed until opened.

Keys: Esc is bound on `AHeroCharacter` (with `bExecuteWhenPaused`, so it can close the menu while the game is paused) and on `ACampPawn`; the hero ignores Esc while a level-up offer is pending. Because a click leaves keyboard focus on a button, Esc then arrives at the menu widget instead, and `UPauseMenuWidget::NativeOnKeyDown` closes the menu itself.

## 6. On-Screen Messages `UHudMessageSubsystem`

[HudMessageSubsystem.h](../../Game_01%205.8/Source/Game_01/Public/UI/HudMessageSubsystem.h). A GameInstance subsystem that keeps the most recent 4 messages, so a message posted in a run is still there after the level changes to the camp (until it expires).

| Function | Behavior |
|---|---|
| `static void Post(const UObject* WorldContext, const FString& Text, const FLinearColor& Color = White, float Seconds = 2.5f)` | Does nothing when there is no World or GameInstance (for example an editor preview world) |
| `TArray<FHudMessage> GetActive() const` | Filters out expired ones, oldest to newest |

Time uses `FPlatformTime::Seconds()` (real time), so messages also expire while the game is paused. Call sites (text, seconds):

| Where | Message |
|---|---|
| `ULootBagComponent` | `Pickup refused: <reason>` (red) |
| `ALootContainer` | `Searched: +N wood` or `+N stone`, plus ` + lost reel <name>` (gold, 3 s); `Already searched` (grey, 2 s) |
| `ACampPawn` | `<Building> upgraded to level N` (green) or the refusal (red), for a click on a building |
| `URunProgressionSubsystem` | `<NAME> is now N-STAR!` (gold, 4 s); `CUT: <name> now belongs to the Boss` (3 s) |
| `UBossRejectsSubsystem` | `THE BOSS TAKES <NAME> - it <gain>` (3 s) |
| `AExtractionZone` | `EXTRACTING - stay inside the green circle` (3 s) |
| `UPauseMenuWidget` | `Run abandoned: the bag was lost` (red, 5 s) |

This message system replaces the engine's `AddOnScreenDebugMessage`, which is always drawn at the top left of the screen and would sit on top of the health bar. `URunProgressionSubsystem` still calls `AddOnScreenDebugMessage` with empty text to clear message ids it used earlier; that is a leftover and draws nothing.

## 7. Esc Pause Menu `UPauseMenuWidget`

Added in commit `5b82d25`. One widget for both screens, created by `AHeroCharacter::CreateHud` (context `Run`) and `ACampPawn::BeginPlay` (context `Camp`); `EPauseContext` decides which buttons it builds. A centered panel 440 wide, hidden until `Toggle()`.

| Item | Camp | Run | Action |
|---|---|---|---|
| `PAUSED` title | yes | yes | |
| `Resume   (Esc)` (green) | yes | yes | `Close()` |
| `Settings` | yes | yes | Shows or hides three rows: `Master volume N%`, `Music volume N%`, `Effects volume N%`, each with `-` and `+` buttons (steps of 10 %) |
| `Return to camp (lose bag)` (red) | no | yes | Two clicks (see below) |
| `Quit game` (red) | yes | yes | `FPlatformMisc::RequestExit(false)` |
| `Controls` text | click a building, pick film cards, START RUN, Esc | WASD, Space, E, 1-3, 4-6, Esc | Static |
| `Credits` text | yes | yes | The CC BY credits (Kounine's music, Winyunq's buildings) and the other packs' authors, all "via Fab" |

- **Pausing**: in a run, opening the menu calls `SetGamePaused(true)` and closing it unpauses; in the camp the game is not paused. Closing always calls `SetGamePaused(false)` and `ReturnFocusToGame()`.
- **Volumes** are read and written through `UGameAudioSubsystem::GetBusVolume` and `SetBusVolume` (clamped 0 to 1). They are kept for the session only and are not saved; the starting values come from Director Tuning ([data-and-storage.md](data-and-storage.md#21-audio-settings-in-director-tuning)). Changing music or master applies at once to the music that is playing.
- **Return to camp** in a run: while the run is active, the first click changes the caption to `Click again: the bag is lost`; the confirmation lapses after 3 seconds of real time. The second click ends the run as a death (`URunOutcomeSubsystem::ReportPlayerDied`, so the bag is lost and the camp is saved), posts `Run abandoned: the bag was lost` and opens `L_Camp`. If the run has already ended (the result page is up), the button goes straight to the camp.
- **History**: before the menu existed, a red "Abandon run (lose bag)" button under the bag panel did the same job (two clicks, then a "RUN ABANDONED" screen). It was removed when the menu was added (dev log, day 3 entry 14); nothing of it is left in `Source/`. The screenshots of it are in `production/qa/evidence/abandon-button/`, which is untracked.

## 8. Who Creates What

| Screen | Created by | Z-order | Visibility rule |
|---|---|---|---|
| `URunHudWidget` | `AHeroCharacter::CreateHud` | default | Always on in a run; its panels show or collapse by state |
| Run `UPauseMenuWidget` | `AHeroCharacter::CreateHud` | 50 | Collapsed until Esc |
| `UCampWidget` | `ACampPawn::BeginPlay` | default | Always on in the camp |
| Camp `UPauseMenuWidget` | `ACampPawn::BeginPlay` | 50 | Collapsed until Esc or the MENU button |
