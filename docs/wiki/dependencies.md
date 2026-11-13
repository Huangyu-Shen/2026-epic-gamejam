# Dependencies

## 1. How Modules Depend on Each Other

For the overall dependency graph see [section 3 of architecture.md](architecture.md#3-layers-and-dependency-graph). Here is a per-directory summary of "who references whom", compiled from header `#include`s and `GetSubsystem<>` calls in the source.

| Directory | References (`#include` count) | Referenced by |
|---|---|---|
| `Camp/` | Extraction, Game, UI | Combat, Editing, Flow, Player, UI |
| `Combat/` | Camp, Enemies, Extraction, Game, Loot, Progression | Game, Player, Progression, UI |
| `Editing/` | Camp, Enemies, Extraction, Game | Player, UI |
| `Enemies/` | Extraction, Game, Loot, Player, Progression | Combat, Editing, Game, Player, Progression |
| `Extraction/` | Flow, Loot | Camp, Combat, Editing, Enemies, Flow, Player, UI |
| `Flow/` | Camp, Extraction, Loot | Extraction |
| `Game/` | Combat, Enemies, Player | Camp, Combat, Editing, Enemies, Progression |
| `Loot/` | UI (only calls `UHudMessageSubsystem::Post`) | Combat, Enemies, Extraction, Flow, Player, UI |
| `Player/` | Camp, Combat, Editing, Enemies, Extraction, Loot, Progression, UI | Enemies, Game, Progression, UI |
| `Progression/` | Combat, Enemies, Game, Player | Combat, Enemies, Player, UI |
| `UI/` | Camp, Combat, Editing, Extraction, Loot, Player, Progression | Camp, Loot, Player |

Counting method: for the headers and implementation files of each directory, count the set of directories it pulls in with `#include "<other directory>/..."` (2026-11-13). In the `Game/` directory, `UDirectorTuningSettings` is read by the various systems, and `ADirectorGameMode` references the hero, enemies and weapons.

**There are cycles**: Enemies<->Player<->Progression, Progression<->Combat, Extraction<->Flow, Camp<->UI, Loot<->UI. The reasons: enemies have to damage the hero, the hero has to read progression, progression has to add to hero health, and settlement has to write the save while the save has to read the stock. All of these cycles are inside the same C++ module `Game_01`, so compilation is fine. No automated check guards the dependency direction, and there is no rules file. When adding a new system, follow the conventions in [section 3 of architecture.md](architecture.md#3-layers-and-dependency-graph): the UI only reads and never modifies, camp state goes in the GameInstance, and per-run state goes in the World.

## 2. UE Module Dependencies

Declared in [Game_01.Build.cs](../../Game_01%205.8/Source/Game_01/Game_01.Build.cs):

| Type | Module | Purpose |
|---|---|---|
| Public | `Core`, `CoreUObject`, `Engine` | Basics |
| Public | `InputCore` | `EKeys`, raw key bindings |
| Public | `EnhancedInput` | **Declared but there is no `#include` of it anywhere in the source**, currently unused |
| Public | `DeveloperSettings` | `UDirectorTuningSettings` (the project settings page) |
| Public | `UMG` | UI widgets |
| Public | `Slate`, `SlateCore` | Font info, `FSlateApplication::SetAllUserFocusToGameViewport` |
| Private | None | - |

`PCHUsage = UseExplicitOrSharedPCHs`. The target files [Game_01.Target.cs](../../Game_01%205.8/Source/Game_01.Target.cs) and [Game_01Editor.Target.cs](../../Game_01%205.8/Source/Game_01Editor.Target.cs) both use `BuildSettingsVersion.V7` and `EngineIncludeOrderVersion.Unreal5_8`.

**Unity build pitfall**: UBT merges several `.cpp` files into one compilation unit. Helper functions with the same name in anonymous namespaces will collide. For example, `RunHudWidget.cpp` and `CampWidget.cpp` each used to have a `RelicName`, which after merging produced C2084. They are now called `HudRelicLabel` and `CampRelicLabel` respectively. When adding new files, give internal functions names that are specific to the file.

## 3. Engine Plugins

[Game_01.uproject](../../Game_01%205.8/Game_01.uproject):

| Plugin | Scope | Purpose |
|---|---|---|
| `ModelingToolsEditorMode` | Editor only | Editor modeling mode (template default) |
| `ModelContextProtocol` | All | Lets AIs such as Claude connect to the editor through MCP |
| `AllToolsets` | All | MCP toolsets (Actor, assets, scene, UMG, etc.) |
| `Terminal` | All | In-editor terminal |

`ModelContextProtocol` and `AllToolsets` are development-time tools and are not needed to run the game. They are marked "All", so they will be included when packaging. Before release, confirm whether to restrict them to Editor.

## 4. Third-Party Dependencies

There are no third-party libraries. The game code only depends on the modules that ship with UE and the engine plugins above.

## 5. Things Deliberately Not Done

- **No Enhanced Input assets**: input uses `BindKey` to bind raw keys. The source comments and the stories both state "no input assets", to move fast during the greybox phase.
- **No GAS**: abilities, damage and status are all self-written components and subsystems. [`.claude/agents`](../../.claude/agents/) has a `ue-gas-specialist`, but the project does not use GAS.
- **No physics collision for enemies**: enemies have no collision and do not Tick, to protect the frame rate with 150 enemies.

## 6. Directories in the Repository That Do Not Take Part in the Build

| Directory | Content |
|---|---|
| `.claude/` | Agents, skills, hooks, scripts and rules for AI collaboration |
| `design/`, `docs/`, `production/` | Design documents, plans, QA evidence |
| `Game_01 5.8/Binaries`, `Intermediate`, `Saved`, `DerivedDataCache` | Generated output, gitignored |
| `Game_Jam_01/` | Pre-jam warm-up project, gitignored, not in the repository |
