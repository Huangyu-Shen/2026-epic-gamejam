# Code Wiki — 2026 Epic AI MegaJam

Documentation for anyone who needs to read or modify this game's code. Every page was written against the source; where it conflicts with older documents or comments, the code wins.

> The project in one sentence: a solo entry in the 2026 Epic AI MegaJam, an Unreal Engine 5.8 project themed **Director's Cut**, a fantasy angled-top-down "mow-down x search-and-extract x thin camp" game, built with Claude Code in 4 days.

Corresponds to commit `bfe9e16` on `main` (2026-11-15; no tracked source file modified). Stories 001 to 010 and 012 are Complete, 011 is In Progress and 013 is Ready (status in the header of each story file in [the epic folder](../../production/epics/directors-cut/EPIC.md); the table in `EPIC.md` itself is out of date). Source root: [`Game_01 5.8/Source/Game_01/`](../../Game_01%205.8/Source/Game_01/).

## At a glance

| Item | Content |
|---|---|
| Form | Single-player UE 5.8 game, one C++ module `Game_01`: 86 source files (45 headers, 41 `.cpp`) and 10,298 lines in 13 source directories, plus 3 test files (303 lines) and 3 build files. The biggest directories are UI (1,387 lines), Enemies (1,363), Progression (1,268) and Game (1,117); 235 lines are the unused `StandIn` placeholders |
| Language and framework | C++20; the UI is UMG built in C++; no Blueprint classes |
| Two maps | `L_Camp` (camp, default startup map), `L_Greybox` (the run map) |
| Core loop | Upgrade at camp (Smithy costs stone, Clinic costs wood) and slot film cards -> loot wood, stone and lost film reels, fight, level up (take one of three options, optionally cutting one first; three copies merge into a star) -> extract or die -> settle back at camp. A cut goes to the Cut Knight boss when the Cut Knight card is slotted |
| State storage | Camp state in GameInstance subsystems (survives level changes); per-run state in World subsystems (cleared on map change) |
| Data | 12 DataTables referenced by `Director Tuning` plus the `StarTuning`, `BossRejects` and audio settings on the same page; a missing table or row silently falls back to C++ defaults |
| Save | Single slot `CampSave` (wood in the field named `Iron`, stone, unlocked options by name, NEW markers, building levels; save version 2, a version 1 file is migrated on load); `Game_01 5.8/Saved/SaveGames/` when run from the editor |
| Assets | 3D models and audio come from Fab packs that are **not in git**; a fresh clone runs as greybox cubes with no sound. Credits: the assets table in [docs/devlog/README.md](../devlog/README.md#assets-and-credits) and the bottom of the Esc menu |
| Command-line switches | Run map: `-PerfProbe`, `-OfferProbe`, `-ReelProbe[=die]` with `-ReelCount=N`, `-PaceProbe` (with `-PaceSpeed` and overrides), `-WeaponShowcase`, `-PauseProbe`, `-EdgeProbe`, `-BossProbe`, `-ZoneShot`; camp: `-CampShot`; any map: `-SaveSlot=Name`. See [build-and-run.md](build-and-run.md#6-performance-probe--perfprobe-and-the-other-test-probes) |
| Automated tests | 12 UE automation tests in `Private/Tests/` (4 star-tuning, 5 lost-reel, 3 unlockable-option), pure helpers only; reported 12 of 12 passing on 2026-11-15. Player-visible changes are checked with screenshots in `production/qa/evidence/` |
| Packaging | Windows Development package via `RunUAT BuildCookRun`; about 1.1 GB (0.7 GB without the `.pdb`) |
| Build status | `Game_01Editor Win64 Development` builds on this machine (the newest package in `Saved/Packages` is dated 2026-11-15 02:48; the last commits came later); the wiki pages were checked by reading, not by rebuilding |

## Contents

| Document | Content |
|---|---|
| [architecture.md](architecture.md) | Tech stack, maps and game modes, layering and dependency graph, runtime structure and what survives a level change, key data flows, design principles, repository tree |
| [modules.md](modules.md) | Responsibilities and boundaries of the source directories |
| [classes.md](classes.md) | Key classes and functions: signatures, behavior, edge cases, defaults, formulas |
| [ui.md](ui.md) | HUD, camp panel, Esc menu, on-screen messages, shared widget building blocks |
| [data-and-storage.md](data-and-storage.md) | Save file, the tuning tables, config files, unit conventions, version compatibility |
| [dependencies.md](dependencies.md) | Dependencies between directories, UE modules and plugins, third-party assets, Unity build pitfalls |
| [build-and-run.md](build-and-run.md) | Environment, fresh-clone behavior, building, controls, every probe flag, tests, packaging, MCP, workflow, FAQ |
| [known-issues.md](known-issues.md) | Defects confirmed by reading the code: location, symptom, impact, direction (no fixes made) |

## Reading paths

| Who you are | Suggested order |
|---|---|
| First time here | README -> [architecture.md](architecture.md) -> [build-and-run.md](build-and-run.md) |
| Changing gameplay rules | [modules.md](modules.md) -> [classes.md](classes.md) -> [data-and-storage.md](data-and-storage.md) |
| Changing the UI | [ui.md](ui.md) -> [architecture.md section 4](architecture.md#4-runtime-structure) |
| Changing loot, reels or the save | [architecture.md section 5.3](architecture.md#53-extraction-or-death-settlement-and-save) -> [data-and-storage.md](data-and-storage.md) -> [known-issues.md section 1](known-issues.md#1-data-and-save-game) |
| Adding a new system | [architecture.md section 3](architecture.md#3-layers-and-dependency-graph) -> [dependencies.md](dependencies.md) -> [known-issues.md](known-issues.md) |
| Checking a change in the running game | [build-and-run.md](build-and-run.md) (probes, tests, packaging) -> `production/qa/evidence/` |
| Cloning the repository | [build-and-run.md section 1.1](build-and-run.md#11-the-fab-asset-packs-are-not-in-git), then [known-issues.md 2.3](known-issues.md#23-asset-packs-from-fab-are-not-in-git-the-code-reaches-them-by-hard-path) |

## Glossary

| Term | Meaning | Where |
|---|---|---|
| Wood / stone | The two loot resources. Crates (tree stumps) give wood, vaults (boulders) give stone. The Clinic costs wood, the Smithy costs stone. Wood still carries the old name Iron in many fields (`Iron`, `IronCount`, `BagIron`, `IronMin`) | `ELootResource`, `UCampStockSubsystem`, [known-issues 1.8](known-issues.md#18-fields-named-iron-now-hold-wood-and-vault-yields-hold-stone) |
| Lost film reel (code name: relic) | Fast Cut, Wide Shot and Chain Scene (`Swiftstep`, `Warbrand`, `Aegis` in code; names kept for save compatibility). A reel takes one bag slot and cannot be used in a run; carry it out alive and the option its row names (`FRelicRow::UnlocksOption`) is unlocked for every later run. Dying loses the reels. Fast Cut unlocks `Haste`, Wide Shot `Magnet`, Chain Scene `Chain` | `ERelicType`, `FRelicRow`, [known-issues 1.4](known-issues.md#14-the-unlock-loop-has-three-leftovers) |
| Unlocked option | An option name saved in the camp stock: the table row name of a weapon or passive that starts locked (`Chain`, `Haste`, `Magnet`). It enters the level-up pool once unlocked; the first offer of the next run shows it first, marked `NEW!` | `UCampStockSubsystem::UnlockOption`, `TakeNewOptions`, `FOptionUnlock::IsAvailable` |
| Heat | Rises when a lost film reel is picked up, resets when a run starts and ends; raises spawn pressure | `ULootHeatSubsystem` |
| Extraction | Stand in the extraction zone (a portal and a green beam) until the bar fills; the bag is settled into camp | `AExtractionZone` |
| Pick one, optional cut (a "cut") | At each level-up the player takes one of three options and may first cut one; a cut option leaves the pool while permanent cuts remain (pool size minus 4) and is put on the cutting-room floor | `URunProgressionSubsystem::ChooseOption`, `CutFromOffer` |
| Cutting-room floor | The run's list of cut options, oldest first | `GetCuttingRoomFloor` |
| Boss rejects | What the Cut Knight boss does with the cuts: a cut weapon becomes a boss attack (shot, slam or ring), a cut passive raises a boss stat or health. Only active when a boss was spawned (the Cut Knight film card) | `UBossRejectsSubsystem`, `FBossRejectTuning` |
| Star | Copies of a weapon or passive are its level; 3 copies make two stars, 9 make three, multiplying its effect | `FStarTuning`, `ApplyOption` |
| Film card | A difficulty card slotted before a run; forces a horde or a boss in exchange for an earnings multiplier. Two slots; cleared when a run ends | `UFilmCardSubsystem` |
| Editing room | The section of the camp panel where film cards are slotted | [ui.md](ui.md#4-camp-ui-ucampwidget) |
| Smithy / Clinic / warehouse | Two upgradable buildings (weapon damage / max health) and a decorative third; they stand on a triangle in the camp | `ACampBuilding`, `ACampWarehouse`, `ACampGameMode` |
| Play area | The walkable floor, found at world start by its shape; hero and enemies are clamped to it | `UPlayAreaSubsystem`, [known-issues 4](known-issues.md#4-architecture-and-robustness) |
| Forest dressing | The grass land mass, tree rim and ground detail built over the floor when the forest pack exists | `AForestDressing` |
| Director Tuning | The project settings page (`UDirectorTuningSettings`) that points at the 12 tables and holds stars, boss rejects and audio settings | [data-and-storage.md](data-and-storage.md#2-tuning-data-tables) |
| Soft pause | While a level-up offer shows, enemies, spawning, weapons and gems stop but engine time keeps running; incomplete, see known issues | [architecture.md](architecture.md#4-runtime-structure) |
| Esc menu | Pause menu in camp and run (Resume, Settings with volume, Return to camp, Quit); in a run it pauses the whole world | `UPauseMenuWidget` |
| Fab packs | The third-party model and audio packs, not in git | [build-and-run.md](build-and-run.md#11-the-fab-asset-packs-are-not-in-git) |
| Probe | A command-line switch that makes a game mode drive the game, log `PERFPROBE` / `OFFERPROBE` / `REELPROBE` / `PACEPROBE` / `CAMPSHOT` ... lines, save screenshots and exit | [build-and-run.md](build-and-run.md#6-performance-probe--perfprobe-and-the-other-test-probes) |
| Stories 001 to 013 | Implementation task numbers | [production/epics/directors-cut/](../../production/epics/directors-cut/) |
| MVP feature N | Feature numbers in the game brief | [design/game-brief.md](../../design/game-brief.md) |

## Maintenance conventions

| What changed | What to update |
|---|---|
| Added or renamed a class, function or public field | [classes.md](classes.md), and [modules.md](modules.md) if needed |
| Added a source directory or system | The dependency table in [modules.md](modules.md) and [dependencies.md](dependencies.md), the graph in [architecture.md](architecture.md), and the counts in the table above (`wc -l` over `Public` and `Private`, tests counted apart) |
| Changed a tuning table, row names or save fields | [data-and-storage.md](data-and-storage.md); a save layout change also needs `CurrentSaveVersion` bumped, a migration branch and [known-issues.md](known-issues.md) |
| Changed a flow (run start, settlement, level-up, camp upgrade) | The matching diagram in [architecture.md](architecture.md) section 5 |
| Added or changed a probe flag | [build-and-run.md](build-and-run.md) section 6 (re-list with `grep -rn "FParse::" "Game_01 5.8/Source/Game_01/Private"`) and the switches row above |
| Added or removed a test | The table in [build-and-run.md](build-and-run.md) section 7 and the count above |
| Changed the HUD, camp panel or Esc menu | [ui.md](ui.md) |
| Changed `Build.cs`, plugins, packaging settings or build commands | [dependencies.md](dependencies.md), [build-and-run.md](build-and-run.md) |
| Used a new asset pack | The assets table in [docs/devlog/README.md](../devlog/README.md#assets-and-credits), the credits text in `PauseMenuWidget.cpp`, and [known-issues.md 2.3](known-issues.md#23-asset-packs-from-fab-are-not-in-git-the-code-reaches-them-by-hard-path) |
| Fixed a known issue | Delete the matching entry from [known-issues.md](known-issues.md) |
| Any change | Re-check the commit hash at the top of this page, [build-and-run.md](build-and-run.md) and [known-issues.md](known-issues.md); line numbers move |
