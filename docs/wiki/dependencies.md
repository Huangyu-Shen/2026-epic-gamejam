# Dependencies

## 1. How Modules Depend on Each Other

For the overall dependency graph see [section 3 of architecture.md](architecture.md#3-layers-and-dependency-graph). Here is a per-directory summary of "who references whom", compiled from the `#include "<Directory>/..."` lines of the headers and implementation files under `Source/Game_01/Public` and `Source/Game_01/Private` (checked 2026-11-14).

| Directory | References (`#include`) | Referenced by |
|---|---|---|
| `Camp/` | Extraction, Game, UI | Combat, Editing, Flow, Player, UI |
| `Combat/` | Camp, Enemies, Extraction, Game, Loot, Progression | Game, Player, Progression, UI |
| `Editing/` | Camp, Enemies, Extraction, Game | Player, UI |
| `Enemies/` | Extraction, Game, Loot, Player, Progression | Combat, Editing, Game, Player, Progression |
| `Extraction/` | Flow, Loot | Camp, Combat, Editing, Enemies, Flow, Game, Loot, Player, Progression, StandIn, UI |
| `Flow/` | Camp, Extraction, Loot | Extraction |
| `Game/` | Combat, Enemies, Extraction, Loot, Player, Progression | Camp, Combat, Editing, Enemies, Loot, Progression |
| `Loot/` | Extraction (`ALootContainer` reads the unlocked options from `UCampStockSubsystem`), Game (`ULootConfig` reads `UDirectorTuningSettings`), UI (only calls `UHudMessageSubsystem::Post`) | Combat, Enemies, Extraction, Flow, Game, Player, StandIn, UI |
| `Player/` | Camp, Combat, Editing, Enemies, Extraction, Loot, Progression, UI | Enemies, Game, Progression, UI |
| `Progression/` | Combat, Enemies, Extraction, Game, Player, UI | Combat, Enemies, Game, Player, UI |
| `StandIn/` | Extraction, Loot | nobody (see section 5) |
| `UI/` | Camp, Combat, Editing, Extraction, Loot, Player, Progression | Camp, Loot, Player, Progression |

Counting method: for each directory, the set of other directories pulled in by an `#include "<other directory>/..."` in its headers and `.cpp` files. `Tests/` was left out. `Game/` is both a hub (`ADirectorGameMode` references the hero, enemies, loot, extraction and progression) and a leaf that everything reads (`UDirectorTuningSettings`). The counts come from `#include` lines only; `GetSubsystem<>` calls always have a matching include, so they add nothing.

**There are cycles**: Enemies<->Player<->Progression, Progression<->Combat, Progression<->Game, Progression<->UI, Combat<->Game, Enemies<->Game, Extraction<->Flow, Camp<->UI, Loot<->UI, Loot<->Extraction, Loot<->Game. The reasons: enemies have to damage the hero, the hero has to read progression, progression has to add to hero health, and settlement has to write the save while the save has to read the stock. All of these cycles are inside the same C++ module `Game_01`, so compilation is fine. No automated check guards the dependency direction, and there is no rules file. When adding a new system, follow the conventions in [section 3 of architecture.md](architecture.md#3-layers-and-dependency-graph): the UI only reads and never modifies, camp state goes in the GameInstance, and per-run state goes in the World.

## 2. UE Module Dependencies

The project has one game module, `Game_01` (`Runtime`, loading phase `Default`, declared in [Game_01.uproject](../../Game_01%205.8/Game_01.uproject)). Its build rules, [Game_01.Build.cs](../../Game_01%205.8/Source/Game_01/Game_01.Build.cs):

| Type | Module | Purpose |
|---|---|---|
| Public | `Core`, `CoreUObject`, `Engine` | Basics |
| Public | `InputCore` | `EKeys`, raw key bindings (`InputCoreTypes.h`) |
| Public | `EnhancedInput` | **No source file includes an Enhanced Input header.** The module is still loaded at runtime, because [DefaultInput.ini](../../Game_01%205.8/Config/DefaultInput.ini) sets `DefaultPlayerInputClass` and `DefaultInputComponentClass` to the Enhanced Input classes; `BindKey` works on them as on the base input component |
| Public | `DeveloperSettings` | `UDirectorTuningSettings` (the project settings page) |
| Public | `UMG` | UI widgets (`UUserWidget`, `UTextBlock`, `UButton`, ...) |
| Public | `Slate`, `SlateCore` | `FSlateFontInfo`, `FSlateBrush`, `FSlateApplication::SetAllUserFocusToGameViewport` in `DirectorUi.cpp` |
| Private | None | - |

`PCHUsage = UseExplicitOrSharedPCHs`. The target files [Game_01.Target.cs](../../Game_01%205.8/Source/Game_01.Target.cs) (`TargetType.Game`) and [Game_01Editor.Target.cs](../../Game_01%205.8/Source/Game_01Editor.Target.cs) (`TargetType.Editor`) both use `BuildSettingsVersion.V7` and `EngineIncludeOrderVersion.Unreal5_8`, and add the module `Game_01` as an extra module. There is no `Server` or `Client` target.

The automation tests in `Source/Game_01/Private/Tests/` are part of the same module, guarded by `WITH_DEV_AUTOMATION_TESTS`; there is no separate test module.

**Unity build pitfall**: UBT merges several `.cpp` files into one compilation unit. Helper functions with the same name in anonymous namespaces will collide (error C2084). It happened with a `RelicName` helper that existed in both `RunHudWidget.cpp` and `CampWidget.cpp`. The remaining relic helpers in `RunHudWidget.cpp` are called `HudRelicLabel` and `HudRelicNames`, and `CampWidget.cpp` has no relic helper any more. When adding new files, give internal functions names that are specific to the file.

## 3. Engine Plugins

[Game_01.uproject](../../Game_01%205.8/Game_01.uproject) enables four plugins. The engine on this machine is `UE_5.8` build 5.8.4 (changelist 58902052, read from `D:/Game/EPIC/UE_5.8/Engine/Build/Build.version`), and in the engine's `.uplugin` files `ModelContextProtocol`, `AllToolsets` and `Terminal` are marked Experimental and `ModelingToolsEditorMode` is marked Beta.

| Plugin | Scope in the `.uproject` | Modules (from the engine's `.uplugin`) | Purpose |
|---|---|---|---|
| `ModelingToolsEditorMode` | Editor target only (`TargetAllowList: Editor`) | n/a | Editor modeling mode (template default) |
| `ModelContextProtocol` | All targets | `ModelContextProtocol` and `ModelContextProtocolEngine` are `Runtime`, `ModelContextProtocolEditor` is `Editor` | Lets AIs such as Claude connect to the editor through MCP (server on port 8000, see [build-and-run.md](build-and-run.md#9-unreal-mcp-ai-collaboration)) |
| `AllToolsets` | All targets | Declares 21 other toolset plugins as dependencies (for example `EditorToolset`, `AutomationTestToolset`, `PluginToolset`, `ConfigSettingsToolset`) | MCP toolsets |
| `Terminal` | All targets | `Terminal` is `Editor` | In-editor terminal |

`ModelContextProtocol` and `AllToolsets` are development-time tools and are not needed to run the game. They are enabled for all targets, so the packager treats them as dependencies. What the packaged build actually contains from them: the staged package from 2026-11-13 (`Saved/Packages/test-2026-11-13`) lists only `Engine/Plugins/Experimental/ModelContextProtocol/ModelContextProtocol.uplugin` (the descriptor file) in its manifest, and no `AllToolsets` or `Terminal` files. Whether the runtime modules of `ModelContextProtocol` are linked into the monolithic `Game_01.exe` was not checked. Before a release, restricting both plugins to the Editor target (`TargetAllowList`, as `ModelingToolsEditorMode` already does) removes the question.

## 4. Third-Party Dependencies

There are no third-party libraries and no `Plugins/` folder in the project. The game code only depends on the modules that ship with UE and the engine plugins above. There is no package manager, lock file or vendored code.

## 5. Things Deliberately Not Done

- **No Enhanced Input assets**: input uses `BindKey` to bind raw keys. The source comments and the stories both state "no input assets", to move fast during the greybox phase. The `EnhancedInput` module entry in `Game_01.Build.cs` is a leftover from the project template.
- **No GAS**: abilities, damage and status are all self-written components and subsystems. [`.claude/agents`](../../.claude/agents/) has a `ue-gas-specialist`, but the project does not use GAS (no `GameplayAbilities` module, no ability system code).
- **No physics collision for enemies**: enemies have no collision and do not Tick (`AEnemy` sets `NoCollision` and `bCanEverTick = false`), to protect the frame rate with 150 enemies. `UEnemyManagerSubsystem` moves them all in one loop.
- **Legacy stand-in code is still compiled**: `Source/Game_01/Private/StandIn/` (`AStandInCharacter`, `AStandInGameMode`, the story 001/002 greybox) is referenced by no map and no other source file (`L_Camp.umap` and `L_Greybox.umap` name `CampGameMode` and `DirectorGameMode`). It still compiles and still binds the keys E, K and N, but no level uses it.

## 6. Directories in the Repository That Do Not Take Part in the Build

| Directory | Content |
|---|---|
| `.claude/` | Agents, skills, hooks, scripts and rules for AI collaboration |
| `design/`, `docs/`, `production/` | Design documents, plans, QA evidence |
| `Game_01 5.8/Binaries`, `Intermediate`, `Saved`, `DerivedDataCache`, `Build` | Generated output, gitignored (`Saved/Packages/` holds the packaged test builds) |
| `Game_01 5.8/.claude`, `Game_01 5.8/production` | Local leftovers (a local settings file and stray session logs), not part of the game |
| `Game_Jam_01/` | Pre-jam warm-up project, gitignored, not in the repository |
| `.tmp/`, `.screenshots/`, `.obsidian/` | Local scratch, screenshots and notes workspace (ignored or per-user) |
