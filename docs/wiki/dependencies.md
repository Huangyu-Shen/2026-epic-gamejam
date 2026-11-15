# Dependencies

Checked against the source on 2026-11-15 (HEAD `bfe9e16`). The project has no releases: the only git tag is `jam-start` (commit `44589d2`, which predates the Unreal project), so "versions" below means the engine version and the plugin descriptors installed on this machine.

## 1. How Modules Depend on Each Other

For the overall dependency graph see [section 3 of architecture.md](architecture.md#3-layers-and-dependency-graph). Here is a per-directory summary of "who references whom", compiled from the `#include "<Directory>/..."` lines of the headers and implementation files under `Source/Game_01/Public` and `Source/Game_01/Private` (recomputed 2026-11-15).

| Directory | References (`#include`) | Referenced by |
|---|---|---|
| `Audio/` | Game | Camp, Combat, Enemies, Extraction, Game, Loot, Progression, UI |
| `Camp/` | Audio, Extraction, Game, Loot, UI | Combat, Editing, Flow, Game, Player, UI |
| `Combat/` | Audio, Camp, Enemies, Extraction, Game, Loot, Progression | Enemies, Game, Player, Progression, UI |
| `Editing/` | Camp, Enemies, Extraction, Game | Player, UI |
| `Enemies/` | Audio, Combat, Extraction, Game, Loot, Player, Progression, UI | Combat, Editing, Game, Player, Progression |
| `Extraction/` | Audio, Flow, Loot, UI | Camp, Combat, Editing, Enemies, Flow, Game, Loot, Player, Progression, StandIn, UI |
| `Flow/` | Camp, Extraction, Loot | Extraction |
| `Game/` | Audio, Camp, Combat, Enemies, Extraction, Loot, Player, Progression, UI | Audio, Camp, Combat, Editing, Enemies, Loot, Player, Progression |
| `Loot/` | Audio, Extraction (`ALootContainer` reads the unlocked options from `UCampStockSubsystem`), Game (`ULootConfig` reads `UDirectorTuningSettings`), UI (calls `UHudMessageSubsystem::Post`) | Camp, Combat, Enemies, Extraction, Flow, Game, Player, StandIn, UI |
| `Player/` | Camp, Combat, Editing, Enemies, Extraction, Game, Loot, Progression, UI | Enemies, Game, Progression, UI |
| `Progression/` | Audio, Combat, Enemies, Extraction, Game, Player, UI | Combat, Enemies, Game, Player, UI |
| `StandIn/` | Extraction, Loot | nobody (see section 5) |
| `UI/` | Audio, Camp, Combat, Editing, Extraction, Loot, Player, Progression | Camp, Enemies, Extraction, Game, Loot, Player, Progression |

Counting method: for each directory, the set of other directories pulled in by an `#include "<other directory>/..."` in its headers and `.cpp` files. `Tests/` was left out. `Game/` is both a hub (`ADirectorGameMode` references the hero, enemies, loot, extraction and progression) and a leaf that everything reads (`UDirectorTuningSettings`). The counts come from `#include` lines only; `GetSubsystem<>` calls always have a matching include, so they add nothing.

`Audio/` is the newest directory (commit `a5f6c32`): `UGameAudioSubsystem` is called by gameplay code through `Play(EGameSound)` and reads its assets and volumes from `UDirectorTuningSettings`. The settings header in turn includes three project headers for its member types: `Audio/GameAudioSubsystem.h` (`EGameSound`), `Enemies/BossRejectsSubsystem.h` (`FBossRejectTuning`) and `Progression/ProgressionTypes.h` (`FStarTuning`). So every file that reads Director Tuning compiles those headers too, and a change to the boss header rebuilds all of them.

**There are cycles** (19 directory pairs reference each other): Audio<->Game, Camp<->Game, Camp<->UI, Combat<->Enemies, Combat<->Game, Combat<->Progression, Enemies<->Game, Enemies<->Player, Enemies<->Progression, Extraction<->Flow, Extraction<->Loot, Extraction<->UI, Game<->Loot, Game<->Player, Game<->Progression, Loot<->UI, Player<->Progression, Player<->UI, Progression<->UI. The reasons: enemies have to damage the hero, the hero has to read progression, progression has to add to hero health, and settlement has to write the save while the save has to read the stock. The `Audio<->Game` pair is only at implementation level (the audio `.cpp` reads the settings; `GameAudioSubsystem.h` includes no project header). All of these cycles are inside the same C++ module `Game_01`, so compilation is fine. No automated check guards the dependency direction, and there is no rules file. When adding a new system, follow the conventions in [section 3 of architecture.md](architecture.md#3-layers-and-dependency-graph): the UI only reads and never modifies camp or run state (the one exception is that its buttons call the subsystems: `ChooseOption`, `CutFromOffer`, `TryUpgrade`, `AddCard`, `OpenLevel`), camp state goes in the GameInstance, and per-run state goes in the World.

## 2. UE Module Dependencies

The project has one game module, `Game_01` (`Runtime`, loading phase `Default`, declared in [Game_01.uproject](../../Game_01%205.8/Game_01.uproject)). Its build rules, [Game_01.Build.cs](../../Game_01%205.8/Source/Game_01/Game_01.Build.cs):

| Type | Module | Purpose |
|---|---|---|
| Public | `Core`, `CoreUObject`, `Engine` | Basics: actors, components, subsystems, `USaveGame`, `UDataTable`, `UGameplayStatics`, sound (`USoundBase`, `UAudioComponent`) and the skeletal and static mesh classes |
| Public | `InputCore` | `EKeys`, raw key bindings (`InputCoreTypes.h`) |
| Public | `EnhancedInput` | **No source file includes an Enhanced Input header.** The module is still loaded at runtime, because [DefaultInput.ini](../../Game_01%205.8/Config/DefaultInput.ini) sets `DefaultPlayerInputClass` and `DefaultInputComponentClass` to the Enhanced Input classes; `BindKey` works on them as on the base input component |
| Public | `DeveloperSettings` | `UDirectorTuningSettings` (the project settings page) |
| Public | `UMG` | UI widgets (`UUserWidget`, `UTextBlock`, `UButton`, `UProgressBar`, `UScrollBox`, ...) and `UWidgetTree` |
| Public | `Slate`, `SlateCore` | `FSlateFontInfo`, `FSlateBrush`, `FButtonStyle`, `FSlateApplication::SetAllUserFocusToGameViewport` in `DirectorUi.cpp` |
| Private | None | - |

Everything the source includes comes from these modules. No source file includes anything from `CommonUI`, `GameplayAbilities`, `AIModule`, `Niagara` or the Online subsystems, so none of those modules is needed. `PCHUsage = UseExplicitOrSharedPCHs`. The target files [Game_01.Target.cs](../../Game_01%205.8/Source/Game_01.Target.cs) (`TargetType.Game`) and [Game_01Editor.Target.cs](../../Game_01%205.8/Source/Game_01Editor.Target.cs) (`TargetType.Editor`) both use `BuildSettingsVersion.V7` and `EngineIncludeOrderVersion.Unreal5_8`, and add the module `Game_01` as an extra module. There is no `Server` or `Client` target.

The automation tests in `Source/Game_01/Private/Tests/` are part of the same module, guarded by `WITH_DEV_AUTOMATION_TESTS`; there is no separate test module. There are 12 tests: five loot and save tests (`ReelUnlockTest.cpp`), four star tests (`StarTuningTest.cpp`) and three unlockable-option tests (`UnlockableOptionsTest.cpp`).

**Unity build pitfall**: UBT merges several `.cpp` files into one compilation unit. Helper functions with the same name in anonymous namespaces will collide (error C2084). It happened with a `RelicName` helper that existed in both `RunHudWidget.cpp` and `CampWidget.cpp`. The remaining relic helpers in `RunHudWidget.cpp` are called `HudRelicLabel`, `HudRelicNames` and `HudOptionNames`, and `CampWidget.cpp` has only `CampBuildingName`. When adding new files, give internal functions names that are specific to the file.

## 3. Engine Plugins

[Game_01.uproject](../../Game_01%205.8/Game_01.uproject) enables four plugins. The engine on this machine is `UE_5.8` build 5.8.4 (changelist 58902052, read from `D:/Game/EPIC/UE_5.8/Engine/Build/Build.version`); the project's `EngineAssociation` is a GUID of that installation, not a version string. The descriptors below were read from `Engine/Plugins` of that install.

| Plugin | Scope in the `.uproject` | Descriptor (engine 5.8.4) | Purpose |
|---|---|---|---|
| `ModelingToolsEditorMode` | Editor target only (`TargetAllowList: Editor`) | `Engine/Plugins/Editor/ModelingToolsEditorMode`, Beta | Editor modeling mode (template default) |
| `ModelContextProtocol` | All targets | `Engine/Plugins/Experimental/ModelContextProtocol`, version 1.0, Experimental, NoRedist, not enabled by default. Modules: `ModelContextProtocol` and `ModelContextProtocolEngine` (Runtime), `ModelContextProtocolEditor` (Editor), plus test modules | Lets AIs such as Claude connect to the editor through MCP. Settings (path `/mcp`, port 8000, auto-start) are in the git-ignored `DefaultEditorPerProjectUserSettings.ini`; see [build-and-run.md](build-and-run.md#9-unreal-mcp-ai-collaboration) |
| `AllToolsets` | All targets | `Engine/Plugins/Experimental/Toolsets/AllToolsets`, version 1.0, Experimental, EditorOnly, NoRedist, no modules of its own; it depends on 21 toolset plugins (for example `EditorToolset`, `AutomationTestToolset`, `PluginToolset`, `ConfigSettingsToolset`, `UMGToolSet`) | MCP toolsets |
| `Terminal` | All targets | `Engine/Plugins/Experimental/Terminal`, version 1.0, Experimental, EditorOnly, NoRedist; its module is `Editor` type | In-editor terminal |

`ModelContextProtocol` and `AllToolsets` are development-time tools and are not needed to run the game. They are enabled for all targets, so the packager treats them as dependencies. What the packaged build actually contains from them: the staged package `Saved/Packages/story-012e` (15 November) lists only `Engine/Plugins/Experimental/ModelContextProtocol/ModelContextProtocol.uplugin` (the descriptor file) in its manifest, and no `AllToolsets` or `Terminal` files. Whether the runtime modules of `ModelContextProtocol` are linked into the monolithic `Game_01.exe` was not checked. Before a release, restricting both plugins to the Editor target (`TargetAllowList`, as `ModelingToolsEditorMode` already does) removes the question.

`DefaultGame.ini` carries a `[/Script/CommonUI.CommonUISettings]` section and three `CommonUI.*` console variables, but the CommonUI plugin is not in the `.uproject` and no game code uses it. `DefaultEngine.ini` contains a plugin-written `AndroidFileServerRuntimeSettings` block (with a `SecurityToken`) that the game does not use.

## 4. Third-Party Dependencies

There are no third-party code libraries and no `Plugins/` folder in the project. The game code only depends on the modules that ship with UE and the engine plugins above. There is no package manager, lock file or vendored code.

There are **third-party content packs** (free Fab downloads) in `Content/`. They are not code, but the code loads them by path, mostly as hard references in constructors (`ConstructorHelpers::FObjectFinder`), so the build needs them to look as intended. They are **not in git** (untracked, not ignored); the author's machine holds them. Where a pack is missing, the code falls back to cubes or silence (see [data-and-storage.md](data-and-storage.md#5-assets-that-are-not-in-git)). Pack names, authors and licences are in the assets table of [docs/devlog/README.md](../devlog/README.md#assets-and-credits); the in-game credits are at the bottom of the Esc menu ([ui.md](ui.md#7-esc-pause-menu-upausemenuwidget)).

| Pack folder in `Content/` | Used by (source) | What it provides |
|---|---|---|
| `RPGHeroSquad` | `AHeroCharacter` constructor | Hero skeletal mesh (`SK_RPGHeroPolyart`), sword, shield, run and walk animations. Without the pack the hero stays an engine cylinder |
| `MonsterForSurvivalGame` | `UEnemySpawnSubsystem` constructor | Five grunt monsters (Slime, Mushroom, Cactus, TurtleShell, Beholder) and the boss (ChestMonster), with run animations. A monster is used only if its mesh loads; with none, the enemies stay cubes |
| `RPGTinyFantasyForest` | `AForestDressing` (land mass, trees, rocks, cliffs, grass, flowers), `ALootContainer` (stump and rock), `AExtractionZone` (portal) | The forest floor and props. Without the land mass the dressing reports no land (`bHasLand`) |
| `WW2Generic` | `ACampBuilding` (smithy: `factory_6_static`), `ACampWarehouse` (`barracks_static`) | Camp buildings (CC BY 4.0, Winyunq) |
| `LowPolyHouse` | `ACampBuilding` (clinic: `SM_Foundation`, `SM_HouseMain`, `SM_Roof`, `SM_Porch`, `SM_PricksTube`) | The clinic; all five parts or none |
| `Free_Sounds_Pack` | `Sounds` in Director Tuning (soft references to 11 cues under `cue/`) | Sound effects |
| `PathOfAdventure` | `CampMusic` and `RunMusic` in Director Tuning (`CUE_Village_Loop`, `CUE_Battle_Loop`) | Music (CC BY, Kounine) |

Imported but referenced by no code, map or setting, so never cooked: `BattleWizardPBR`, `BattleWizardPolyart`, `FourEvilDragonsHP`, `FourEvilDragonsPBR`, `FreeAtmosGameMusic`, `Free_5_Samples`, `SmallSoundKit` and `ZenUV_SciFi_TrimSheet` (together about 1.1 GB on disk; the devlog lists them as "imported, not used"). The engine's own `/Engine/BasicShapes/` cube, cylinder and sphere are the fallbacks, and `Content/Materials/M_FlatColor` is the project's own flat-colour material.

## 5. Things Deliberately Not Done

- **No Enhanced Input assets**: input uses `BindKey` to bind raw keys. The source comments and the stories both state "no input assets", to move fast during the greybox phase. The `EnhancedInput` module entry in `Game_01.Build.cs` is a leftover from the project template.
- **No GAS**: abilities, damage and status are all self-written components and subsystems. [`.claude/agents`](../../.claude/agents/) has a `ue-gas-specialist`, but the project does not use GAS (no `GameplayAbilities` module, no ability system code).
- **No physics collision for enemies**: enemies have no collision and do not Tick (`AEnemy` sets `NoCollision` and `bCanEverTick = false`), to protect the frame rate with 150 enemies. `UEnemyManagerSubsystem` moves them all in one loop.
- **No widget blueprints**: every screen is built in C++ ([ui.md](ui.md)); the `UMG` module is used as a C++ widget library only.
- **Legacy stand-in code is still compiled**: `Source/Game_01/Private/StandIn/` (`AStandInCharacter`, `AStandInGameMode`, the story 001/002 greybox) is referenced by no map and no other source file (`L_Camp.umap` and `L_Greybox.umap` name `CampGameMode` and `DirectorGameMode`). It still compiles and still binds the keys E, K and N, but no level uses it.

## 6. Directories in the Repository That Do Not Take Part in the Build

| Directory | Content |
|---|---|
| `.claude/` | Agents, skills, hooks, scripts and rules for AI collaboration |
| `design/`, `docs/`, `production/` | Design documents, plans, QA evidence |
| `Game_01 5.8/Binaries`, `Intermediate`, `Saved`, `DerivedDataCache`, `Build` | Generated output, gitignored (`Saved/Packages/` holds the packaged test builds) |
| `Game_01 5.8/.claude`, `Game_01 5.8/production` | Local leftovers (a local settings file and stray session logs), not part of the game |
| `Game_01 5.8/Content/<pack folders>` | Untracked content packs, see section 4 |
| `Game_Jam_01/` | Pre-jam warm-up project, gitignored, not in the repository |
| `.tmp/`, `.screenshots/`, `.obsidian/` | Local scratch, screenshots and notes workspace (ignored or per-user) |
