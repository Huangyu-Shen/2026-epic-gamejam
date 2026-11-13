# Main Module Responsibilities

The source is split into directories by function. Headers are in `Public/<module>/` and implementations in `Private/<module>/`. Line counts include headers and implementations, from `wc -l` on 2026-11-13. Paths omit the prefix `Game_01 5.8/Source/Game_01/`.

| Module | Lines | Responsibility | Not responsible for |
|---|---|---|---|
| [Loot](#loot-loot-bag-and-heat) | 997 | Searchable containers, bag, relics, heat | Extraction settlement, UI |
| [Combat](#combat) | 384 | Health, auto weapons | Weapon values (in Progression), enemy movement |
| [Enemies](#enemies) | 713 | Enemies, spawning, swarm movement and contact damage | XP after a kill (in Progression) |
| [Progression](#progression-in-run-growth) | 724 | XP, level-ups, pick-one-of-three, weapons and passives | Weapon firing (in Combat) |
| [Extraction](#extraction-extraction-and-camp-stock) | 584 | Extraction zone, end and settlement of a run, camp stock | Save game, buildings |
| [Editing](#editing-pre-run-editing) | 277 | Film card data, slots, applying them to a run | The card-picking UI (in UI) |
| [Camp](#camp) | 670 | Camp scene, building upgrades and bonuses, relic dismantling | The camp panel (in UI) |
| [Flow](#flow-save-game) | 178 | Reading and writing the single-slot save | Deciding when to save (it listens for stock and building changes) |
| [Player](#player-hero) | 421 | Hero movement, dodge, search, using relics, creating the HUD, run-start initialization | Combat, bag rules |
| [Game](#game-game-mode-and-tuning-settings) | 189 | Run game mode, performance probe, `Director Tuning` settings | The camp game mode (in Camp) |
| [UI](#ui) | 955 | HUD, camp panel, on-screen messages, shared widget building blocks | Game rules |
| [StandIn](#standin-legacy-stand-in) | 256 | The throwaway stand-in character from story 001, since superseded | - |

## Loot: Loot, Bag and Heat

| Class | Responsibility |
|---|---|
| [`ULootBagComponent`](../../Game_01%205.8/Source/Game_01/Private/Loot/LootBagComponent.cpp) | Bag: Iron takes `ceil(Iron / IronPerSlot)` slots, each relic takes 1 slot, total capacity `BagCapacitySlots` (default 6). Adding a relic raises heat, and using a relic adds a buff and lowers heat |
| [`ALootContainer`](../../Game_01%205.8/Source/Game_01/Private/Loot/LootContainer.cpp) | Searchable container (supply crate, vault), each searchable only once. When the bag is full nothing is consumed, and a retry does not re-roll |
| [`ULootHeatSubsystem`](../../Game_01%205.8/Source/Game_01/Private/Loot/LootHeatSubsystem.cpp) | Global heat from 0 to `MaxHeat`. `GetSpawnPressureMultiplier()` interpolates linearly between 1.0 and 3.0, and spawning reads it |
| [`ULootConfig`](../../Game_01%205.8/Source/Game_01/Private/Loot/LootConfig.cpp) | Data asset: relic table, container table, tuning table. When missing, the inline defaults in the constructor are used |
| [`LootTypes.h`](../../Game_01%205.8/Source/Game_01/Public/Loot/LootTypes.h) | Enums and data rows: `ERelicType` (Swiftstep, Warbrand, Aegis), `ELootContainerType`, `FLootStatModifier`, `FRelicRow`, `FLootContainerRow`, `FLootTuning` |

## Combat

| Class | Responsibility |
|---|---|
| [`UHealthComponent`](../../Game_01%205.8/Source/Game_01/Private/Combat/HealthComponent.cpp) | Health, invulnerability window after a hit, invulnerable mode, `Kill()`. `BeginPlay` reads values from the `Hero` row of `HealthTuningTable` |
| [`UAutoWeaponComponent`](../../Game_01%205.8/Source/Game_01/Private/Combat/AutoWeaponComponent.cpp) | Each owned weapon fires automatically at its own interval. Blade hits the nearest enemy, Bolt fires along a line and pierces, Nova hits everything within a radius |

## Enemies

| Class | Responsibility |
|---|---|
| [`AEnemy`](../../Game_01%205.8/Source/Game_01/Private/Enemies/Enemy.cpp) | Greybox enemy, does not Tick. When `ApplyDamage` reaches 0 it notifies the manager and destroys itself |
| [`UEnemyManagerSubsystem`](../../Game_01%205.8/Source/Game_01/Private/Enemies/EnemyManagerSubsystem.cpp) | Holds all live enemies and does everything in one Tick: chasing, grid-based pushing apart, contact damage (each enemy has its own cooldown) |
| [`UEnemySpawnSubsystem`](../../Game_01%205.8/Source/Game_01/Private/Enemies/EnemySpawnSubsystem.cpp) | Wave scheduling: spawns Grunts on a ring around the hero at rate = `(BaseRate + RampPerMinute × minutes) × heat pressure`. `SpawnHorde` and `SpawnBoss` are called by film cards |
| [`EnemyTypes.h`](../../Game_01%205.8/Source/Game_01/Public/Enemies/EnemyTypes.h) | `FEnemyTuning`, `FSwarmTuning`, `FSpawnTuning`, `EEnemyKind` |

## Progression: In-Run Growth

| Class | Responsibility |
|---|---|
| [`URunProgressionSubsystem`](../../Game_01%205.8/Source/Game_01/Private/Progression/RunProgressionSubsystem.cpp) | XP and level (requirement = `BaseXp × Growth^(Level-1)`), generating and applying the pick-one-of-three, weapon levels, passive stacks, pickup magnetism. Reset every run by `ResetProgression()`, and the run starts with a level 1 Blade |
| [`AXpPickup`](../../Game_01%205.8/Source/Game_01/Private/Progression/XpPickup.cpp) | XP gem, registered with the subsystem, which attracts and collects it every frame |
| [`ProgressionTypes.h`](../../Game_01%205.8/Source/Game_01/Public/Progression/ProgressionTypes.h) | `EWeaponType` (Blade, Bolt, Nova), `EPassiveType` (Swiftness, Might, Vitality, Greed), `FXpCurveRow`, `FWeaponRow`, `FPassiveRow` |

## Extraction: Extraction and Camp Stock

| Class | Responsibility |
|---|---|
| [`AExtractionZone`](../../Game_01%205.8/Source/Game_01/Private/Extraction/ExtractionZone.cpp) | Extraction point: while the hero stands inside and a run is in progress, the progress bar fills over `ExtractionSeconds` (default 5 seconds). Leaving resets or decays it. It only Ticks when someone is inside or it is decaying |
| [`URunOutcomeSubsystem`](../../Game_01%205.8/Source/Game_01/Private/Extraction/RunOutcomeSubsystem.cpp) | The state machine of a run, which ends only once. It settles into the stock, writes the save, clears the bag, clears heat, and broadcasts `OnRunEnded` |
| [`UCampStockSubsystem`](../../Game_01%205.8/Source/Game_01/Private/Extraction/CampStockSubsystem.cpp) | Iron and warehouse relics, GameInstance level |
| [`FRunSettlement::Compute`](../../Game_01%205.8/Source/Game_01/Private/Extraction/ExtractionTypes.cpp) | Pure-function settlement: extracting yields `floor(bag Iron × multiplier)` and all relics, and dying loses everything |

## Editing: Pre-Run Editing

[`UFilmCardSubsystem`](../../Game_01%205.8/Source/Game_01/Private/Editing/FilmCardSubsystem.cpp): film cards in two slots. Card effects: `ForceHorde` (spawn a batch of Grunts at the start of the run), `ForceBoss` (spawn a Boss at the start of the run), and payout multipliers multiply together. The selection is stored in `UCampMetaSubsystem::PendingFilmCards`, so it survives `OpenLevel`. It is cleared when a run ends.

## Camp

| Class | Responsibility |
|---|---|
| [`UCampMetaSubsystem`](../../Game_01%205.8/Source/Game_01/Private/Camp/CampMetaSubsystem.cpp) | Level and upgrades of the two buildings (Smithy adds weapon damage, Clinic adds max health). Cost `floor(BaseCost × CostGrowth^level)`. Relic dismantling. `PendingFilmCards` |
| [`ACampGameMode`](../../Game_01%205.8/Source/Game_01/Private/Camp/CampGameMode.cpp) | When no buildings are placed in the camp scene, automatically spawns the Smithy, Clinic and warehouse |
| [`ACampPawn`](../../Game_01%205.8/Source/Game_01/Private/Camp/CampPawn.cpp) | Fixed camera, clicking a building with the mouse upgrades it, creates `UCampWidget` |
| [`ACampBuilding`](../../Game_01%205.8/Source/Game_01/Private/Camp/CampBuilding.cpp), [`ACampWarehouse`](../../Game_01%205.8/Source/Game_01/Private/Camp/CampWarehouse.cpp) | Greybox blocks. A building gets taller with each level and is tinted by type |

## Flow: Save Game

[`UCampSaveSubsystem`](../../Game_01%205.8/Source/Game_01/Private/Flow/CampSaveSubsystem.cpp) and [`UCampSaveGame`](../../Game_01%205.8/Source/Game_01/Public/Flow/CampSaveGame.h). See [data-and-storage.md](data-and-storage.md) for details.

## Player: Hero

[`AHeroCharacter`](../../Game_01%205.8/Source/Game_01/Private/Player/HeroCharacter.cpp): WASD to move, Space to dodge, E to search, 1 through 6 to use relics, K for debug suicide. `BeginPlay` calls `StartRun()` for run-start initialization, then `CreateHud()`.

## Game: Game Mode and Tuning Settings

| Class | Responsibility |
|---|---|
| [`ADirectorGameMode`](../../Game_01%205.8/Source/Game_01/Private/Game/DirectorGameMode.cpp) | Run game mode, default Pawn is the hero. With `-PerfProbe` on the command line it runs a 150-enemy frame rate probe |
| [`UDirectorTuningSettings`](../../Game_01%205.8/Source/Game_01/Public/Game/DirectorTuningSettings.h) | "Director Tuning" in the project settings, references the 9 DataTables, values stored in [DefaultGame.ini](../../Game_01%205.8/Config/DefaultGame.ini) |

## UI

See [ui.md](ui.md).

## StandIn: Legacy Stand-In

[`AStandInCharacter`](../../Game_01%205.8/Source/Game_01/Private/StandIn/StandInCharacter.cpp), `AStandInGameMode`: a throwaway stand-in from story 001, with comments saying "replaced by story 003". Nothing in the code uses them any more. Only comments in [`HeroCharacter.h`](../../Game_01%205.8/Source/Game_01/Public/Player/HeroCharacter.h) and [`DirectorGameMode.h`](../../Game_01%205.8/Source/Game_01/Public/Game/DirectorGameMode.h) mention them. See [known-issues.md](known-issues.md).
