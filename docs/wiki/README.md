# Code Wiki — 2026 Epic AI MegaJam

Documentation for anyone who needs to read or modify this game's code. Every page was written against the source; where it conflicts with older documents or comments, the code wins.

> The project in one sentence: a solo entry in the 2026 Epic AI MegaJam, an Unreal Engine 5.8 project themed **Director's Cut**, a fantasy angled-top-down "mow-down x search-and-extract x thin camp" game, built with Claude Code in 4 days.

Corresponds to commit `3177f1d` on the `main` branch (2026-11-14) plus the story 009 changes that were in the working tree, not yet committed, when this page was updated: stories 001 to 009 are done (010 to 013 are open in [the epic](../../production/epics/directors-cut/EPIC.md)). Source root: [`Game_01 5.8/Source/Game_01/`](../../Game_01%205.8/Source/Game_01/).

## At a glance

| Item | Content |
|---|---|
| Form | Single-player UE 5.8 game, one C++ module `Game_01` (about 7000 lines in 12 source directories, of which 962 are UI and 235 are the legacy StandIn placeholders; plus 302 lines of tests) |
| Language and framework | C++20; the UI is UMG built in C++; no Blueprint classes |
| Two maps | `L_Camp` (camp, the default startup map), `L_Greybox` (the run map) |
| Core loop | Upgrade at camp and pick film cards -> loot, fight and level up (cut one of three options, keep two; three copies merge into a star) on the run map -> extract or die -> settle back at camp |
| State storage | Camp state lives in GameInstance subsystems (persists across levels); per-run state lives in World subsystems (cleared on map change) |
| Data | 12 DataTables (referenced by `Director Tuning`, including the three loot tables added in story 008) plus the `StarTuning` struct on the same page; when a table is missing, defaults in code are used silently |
| Save | Single slot `CampSave`, storing Iron, unlocked upgrade options (by name) and building levels (save version 2; a version 1 save is migrated on load; the class default of the version is 1 on purpose); located at `Game_01 5.8/Saved/SaveGames/` when run from the editor |
| Loot numbers | Bag size, heat, container yields and reel rows come from three tables (`DT_LootTuning`, `DT_LootContainers`, `DT_Reels`) through `ULootConfig::CreateFromSettings`; a lost film reel cannot be used or dismantled |
| Command-line switches | `-PerfProbe`, `-OfferProbe`, `-ReelProbe` / `-ReelProbe=die` with `-ReelCount=N` (run map), `-CampShot` (camp map), `-SaveSlot=Name` (any map); see [build-and-run.md](build-and-run.md#6-performance-probe--perfprobe) |
| Automated tests | 12 UE automation tests under `Source/Game_01/Private/Tests/` (4 star-tuning tests, 5 lost-reel tests, 3 unlockable-option tests), pure helpers only; evidence for player-visible changes is screenshots and probe logs in `production/qa/evidence/` |
| Build status | `Game_01Editor Win64 Development` was recorded as building successfully on 2026-11-13; it was not rebuilt while checking this wiki |

## Contents

| Document | Content |
|---|---|
| [architecture.md](architecture.md) | Tech stack, the two maps and game modes, layering and dependency graph, runtime structure and what survives a level change, 4 key data flows (start of a run, kill to level-up, settlement and save, camp upgrade), design principles, repository tree |
| [modules.md](modules.md) | Responsibilities and boundaries of the 12 source directories |
| [classes.md](classes.md) | Key classes and functions: signatures, behavior, edge cases, default values, formulas |
| [ui.md](ui.md) | HUD, camp panel, on-screen messages, shared widget building blocks |
| [data-and-storage.md](data-and-storage.md) | Save file, the 12 tuning tables, config files, unit conventions, version compatibility |
| [dependencies.md](dependencies.md) | Dependencies between directories, UE modules and plugins, third-party dependencies, Unity build pitfalls |
| [build-and-run.md](build-and-run.md) | Environment, building, running, controls, performance probe, testing status, pre-release checks, FAQ |
| [known-issues.md](known-issues.md) | Problems confirmed by reading the code: save, data, visuals, legacy code, robustness |

## Reading paths

| Who you are | Suggested order |
|---|---|
| First time here | README -> [architecture.md](architecture.md) -> [build-and-run.md](build-and-run.md) |
| Changing gameplay rules | [modules.md](modules.md) -> [classes.md](classes.md) -> [data-and-storage.md](data-and-storage.md) |
| Changing the UI | [ui.md](ui.md) -> [architecture.md section 4](architecture.md#4-runtime-structure) |
| Changing loot, reels or the save | [architecture.md section 5.3](architecture.md#53-extraction-or-death-settlement-and-save) -> [data-and-storage.md](data-and-storage.md) -> [known-issues.md section 1](known-issues.md#1-data-and-save-game) |
| Adding a new system | [architecture.md section 3](architecture.md#3-layers-and-dependency-graph) -> [dependencies.md](dependencies.md) -> [known-issues.md](known-issues.md) |
| Checking a change in the running game | [build-and-run.md](build-and-run.md) (probes and testing) -> `production/qa/evidence/` |

## Glossary

| Term | Meaning | Where |
|---|---|---|
| Iron | The only general-purpose material; found by looting, moved into camp after extraction, used to upgrade buildings | `ULootBagComponent`, `UCampStockSubsystem` |
| Lost film reel (code name: relic) | Fast Cut, Wide Shot and Chain Scene (`Swiftstep`, `Warbrand`, `Aegis` in code; the enum names are kept for save compatibility). A reel takes one bag slot and cannot be used in a run; carry it out alive and the upgrade option its row names (`FRelicRow::UnlocksOption`) is unlocked for every later run. Dying loses the reels. Fast Cut unlocks `Haste`, Wide Shot unlocks `Magnet`, Chain Scene unlocks `Chain` | `ERelicType`, `FRelicRow`, [known-issues 1.4](known-issues.md#14-the-unlock-loop-has-three-leftovers) |
| Unlocked option | An upgrade-option name saved in the camp stock (save version 2): the table row name of a weapon or passive that starts locked (`Chain`, `Haste`, `Magnet`). A locked option is in the level-up pool only once it is unlocked; the first offer of the next run shows it first, marked `NEW!` | `UCampStockSubsystem::UnlockOption`, `TakeNewOptions`, `FOptionUnlock::IsAvailable` |
| Heat | Rises each time a lost film reel is picked up and is reset when a run starts and when it ends; raises spawn pressure | `ULootHeatSubsystem` |
| Extraction | Stand in the extraction zone until the progress bar fills; the bag is settled into camp | `AExtractionZone` |
| Cut one, keep two | At each level-up the player cuts one of three options and keeps the other two; while permanent cuts remain (pool size minus 4) the cut option leaves the pool for the run | `URunProgressionSubsystem::CutOffer` |
| Star | Copies of a weapon or passive are its level; 3 copies make two stars, 9 make three, which multiplies its effect (`FStarTuning`) | `FStarTuning`, `ApplyOption` |
| Film card | A difficulty card picked before a run; forces a horde or a Boss in exchange for a higher earnings multiplier | `UFilmCardSubsystem` |
| Editing room | The section of the camp panel where film cards are slotted (2 slots) | [CampWidget](ui.md#4-camp-ui-ucampwidget) |
| Smithy / Clinic | Two buildings: add weapon damage / add max health | `UCampMetaSubsystem` |
| Director Tuning | The project settings page (`UDirectorTuningSettings`) that points at the 12 tables and holds `StarTuning` | [data-and-storage.md](data-and-storage.md#2-tuning-data-tables) |
| Soft pause | While the level-up panel is shown, enemies, spawning and weapons stop; engine time keeps running | [architecture.md](architecture.md#4-runtime-structure) |
| Probe | A command-line switch that makes a game mode drive the game, log `PERFPROBE` / `OFFERPROBE` / `REELPROBE` / `CAMPSHOT` lines, save screenshots and exit | [build-and-run.md](build-and-run.md#6-performance-probe--perfprobe) |
| Stories 001 to 013 | Implementation task numbers (001 to 009 done) | [production/epics/directors-cut/](../../production/epics/directors-cut/) |
| MVP feature N | Feature numbers in the game brief | [design/game-brief.md](../../design/game-brief.md) |

## Maintenance conventions

| What changed | What to update |
|---|---|
| Added or renamed a class, function or public field | [classes.md](classes.md), and [modules.md](modules.md) if needed |
| Added a source directory or system | The dependency table in [modules.md](modules.md) and [dependencies.md](dependencies.md), and the graph in [architecture.md](architecture.md) |
| Changed a tuning table, row names or save fields | [data-and-storage.md](data-and-storage.md); changing the save layout also requires bumping `CurrentSaveVersion`, adding a migration branch and updating [known-issues.md](known-issues.md) |
| Changed a flow (run start, settlement, level-up, camp upgrade) | The matching diagram in [architecture.md](architecture.md) section 5 |
| Added or changed a command-line probe | [build-and-run.md](build-and-run.md) and the switches row above |
| Changed the HUD or camp panel | [ui.md](ui.md) |
| Changed `Build.cs`, plugins or build commands | [dependencies.md](dependencies.md), [build-and-run.md](build-and-run.md) |
| Fixed a known issue | Delete the matching entry from [known-issues.md](known-issues.md) |

## Differences from the old wiki

The old wiki (from the story 001 to 003 period) has been rewritten in full. All 6 original file names are kept (README, architecture, modules, classes, dependencies, build-and-run), and three pages are new: `ui.md`, `data-and-storage.md` and `known-issues.md`. A statement the old pages got wrong about the gray hero and enemies is corrected in [known-issues.md](known-issues.md#21-the-hero-enemies-and-xp-gems-are-gray-and-tinting-has-no-effect).
