# Code Wiki — 2026 Epic AI MegaJam

Documentation for anyone who needs to read or modify this game's code. Every page was written against the source; where it conflicts with older documents or comments, the code wins.

> The project in one sentence: a solo entry in the 2026 Epic AI MegaJam, an Unreal Engine 5.8 project themed **Director's Cut**, a fantasy angled-top-down "mow-down x search-and-extract x thin camp" game, built with Claude Code in 4 days.

Corresponds to commit `cd44b7f` on the `main` branch (2026-11-13), with stories 001 to 007 all done. Source root: [`Game_01 5.8/Source/Game_01/`](../../Game_01%205.8/Source/Game_01/).

## At a glance

| Item | Content |
|---|---|
| Form | Single-player UE 5.8 game, one C++ module `Game_01` (about 6350 lines, of which 955 are UI and 256 are legacy placeholders) |
| Language and framework | C++20; the UI is UMG built in C++; no Blueprint classes |
| Two maps | `L_Camp` (camp, the default startup map), `L_Greybox` (the run map) |
| Core loop | Upgrade at camp and pick cards -> loot, fight and level up on the run map -> extract or die -> settle back at camp |
| State storage | Camp state lives in a GameInstance subsystem (persists across levels); per-run state lives in World subsystems (cleared on map change) |
| Data | 12 DataTables (referenced by `Director Tuning`, including the three loot tables added in story 008); when a table is missing, defaults in code are used |
| Save | Single slot `CampSave`, storing Iron, unlocked upgrade options and building levels (save version 2; a version 1 save is migrated on load); located at `Game_01 5.8/Saved/SaveGames/` |
| Automated tests | 9 UE automation tests under `Source/Game_01/Private/Tests/` (4 star-tuning tests, 5 lost-reel tests); evidence for player-visible changes is screenshots |
| Build status | `Game_01Editor Win64 Development` builds successfully (2026-11-13) |

## Contents

| Document | Content |
|---|---|
| [architecture.md](architecture.md) | Tech stack, the two maps and game modes, layering and dependency graph, runtime structure, 3 key data flows, design principles, repository tree |
| [modules.md](modules.md) | Responsibilities and boundaries of the 11 source directories |
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
| Adding a new system | [architecture.md section 3](architecture.md#3-layers-and-dependency-graph) -> [dependencies.md](dependencies.md) -> [known-issues.md](known-issues.md) |

## Glossary

| Term | Meaning | Where |
|---|---|---|
| Iron | The only general-purpose material; found by looting, moved into camp after extraction, used to upgrade buildings | `ULootBagComponent`, `UCampStockSubsystem` |
| Lost film reel (code name: relic) | Fast Cut, Wide Shot and Chain Scene. A reel cannot be used in a run; carry it out alive and the upgrade option it names is unlocked for every later run. Dying loses the reels | `ERelicType`, `FRelicRow` |
| Heat | Rises each time a lost film reel is picked up and is reset only when the run ends; determines spawn pressure | `ULootHeatSubsystem` |
| Extraction | Stand in the extraction zone until the progress bar fills; the bag is settled into camp | `AExtractionZone` |
| Film card | A difficulty card picked before a run; forces a horde or a Boss in exchange for a higher earnings multiplier | `UFilmCardSubsystem` |
| Editing room | The camp area for picking film cards (the Should tier also has "restore a cut segment", not built) | [CampWidget](ui.md#4-camp-ui-ucampwidget) |
| Smithy / Clinic | Two buildings: add weapon damage / add max health | `UCampMetaSubsystem` |
| Soft pause | While the level-up panel is shown, enemies, spawning and weapons stop; engine time keeps running | [architecture.md](architecture.md#4-runtime-structure) |
| Stories 001 to 008 | Implementation task numbers | [production/epics/directors-cut/](../../production/epics/directors-cut/) |
| MVP feature N | Feature numbers in the game brief | [design/game-brief.md](../../design/game-brief.md) |

## Maintenance conventions

| What changed | What to update |
|---|---|
| Added or renamed a class, function or public field | [classes.md](classes.md), and [modules.md](modules.md) if needed |
| Added a source directory or system | The dependency table in [modules.md](modules.md) and [dependencies.md](dependencies.md), and the diagram in [architecture.md](architecture.md) |
| Changed a tuning table, row names or save fields | [data-and-storage.md](data-and-storage.md); changing the save layout also requires adding a version check and updating [known-issues.md](known-issues.md) |
| Changed the HUD or camp panel | [ui.md](ui.md) |
| Changed `Build.cs`, plugins or build commands | [dependencies.md](dependencies.md), [build-and-run.md](build-and-run.md) |
| Fixed a known issue | Delete the matching entry from [known-issues.md](known-issues.md) |

## Differences from the old wiki

The old wiki (from the story 001 to 003 period) has been rewritten in full. All 6 original file names are kept (README, architecture, modules, classes, dependencies, build-and-run), and three pages are new: `ui.md`, `data-and-storage.md` and `known-issues.md`. The errors corrected from the old documents are listed in the handoff notes and in [known-issues.md](known-issues.md#21-the-hero-enemies-and-xp-gems-are-gray-and-tinting-has-no-effect).
