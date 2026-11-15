# Main Module Responsibilities

The source is split into directories by function. Headers are in `Public/<module>/` and implementations in `Private/<module>/`. Line counts are `wc -l` of every `.h` and `.cpp` in `Public/<module>` and `Private/<module>`, recounted on 2026-11-14; `Private/Tests` (three automation test files, 302 lines) is not counted. Total 7020. Paths omit the prefix `Game_01 5.8/Source/Game_01/`.

| Module | Lines | Responsibility | Not responsible for |
|---|---|---|---|
| [Loot](#loot-loot-bag-and-heat) | 925 | Searchable containers, bag, lost film reels, heat | Extraction settlement, UI |
| [Combat](#combat) | 451 | Health, auto weapons | Weapon values (in Progression), enemy movement |
| [Enemies](#enemies) | 713 | Enemies, spawning, swarm movement and contact damage | XP after a kill (in Progression) |
| [Progression](#progression-in-run-growth) | 1086 | XP, level-ups, cut-one-keep-two offers, star merging, weapons and passives | Weapon firing (in Combat) |
| [Extraction](#extraction-extraction-and-camp-stock) | 618 | Extraction zone, end and settlement of a run, camp stock (Iron and unlocked options) | The save file format, buildings |
| [Editing](#editing-pre-run-editing) | 277 | Film card data, slots, applying them to a run | The card-picking UI (in UI) |
| [Camp](#camp) | 683 | Camp scene, building upgrades and bonuses | The camp panel (in UI) |
| [Flow](#flow-save-game) | 222 | Reading the single-slot save at startup, writing it whenever the stock or a building changes | What the run does with the stock |
| [Player](#player-hero) | 398 | Hero movement, dodge, search, creating the HUD, run-start initialization | Combat, bag rules |
| [Game](#game-game-mode-and-tuning-settings) | 450 | Run game mode, probes (`-PerfProbe`, `-OfferProbe`, `-ReelProbe`), `Director Tuning` settings | The camp game mode (in Camp, which owns `-CampShot`) |
| [UI](#ui) | 962 | HUD, camp panel, on-screen messages, shared widget building blocks | Game rules |
| [StandIn](#standin-legacy-stand-in) | 235 | The throwaway stand-in character from story 001, since superseded | - |

## Loot: Loot, Bag and Heat

| Class | Responsibility |
|---|---|
| [`ULootBagComponent`](../../Game_01%205.8/Source/Game_01/Private/Loot/LootBagComponent.cpp) | Bag: Iron takes `ceil(Iron / IronPerSlot)` slots, each lost film reel takes 1 slot, total capacity `BagCapacitySlots` (default 6, `IronPerSlot` default 10). `TryAddLoot` is atomic: when it would not fit, nothing is added and a "Pickup refused: Bag is full" toast is posted. Picking up a reel raises heat. A reel cannot be used: it only does something when it is carried out alive (see `URunOutcomeSubsystem`). `BeginPlay` pushes the tuning into `ULootHeatSubsystem` and logs a `LOOTCFG` line saying whether the numbers came from the tables or the C++ defaults, because a missing table falls back silently |
| [`ALootContainer`](../../Game_01%205.8/Source/Game_01/Private/Loot/LootContainer.cpp) | Searchable container (supply crate, vault), each searchable only once; searching completes instantly. The roll is made on the first try and remembered: when the bag is full nothing is consumed and a retry does not re-roll (so a full bag cannot be used to re-roll the loot). If another container filled the same reel type into the bag meanwhile, the remembered reel is dropped and only the Iron is given. A reel is only rolled when `ULootConfig::PickRandomRelic` finds one that can still drop. Default rows: Supply Crate 3-6 Iron with a 20% reel chance, Vault 10-20 Iron with a 100% reel chance |
| [`ULootHeatSubsystem`](../../Game_01%205.8/Source/Game_01/Private/Loot/LootHeatSubsystem.cpp) | Global heat from 0 to `MaxHeat` (default 100, +15 per reel). `GetSpawnPressureMultiplier()` interpolates linearly between `SpawnPressureAtZeroHeat` (1.0) and `SpawnPressureAtMaxHeat` (3.0) by normalized heat, and spawning reads it |
| [`ULootConfig`](../../Game_01%205.8/Source/Game_01/Private/Loot/LootConfig.cpp) | Loot config: reel table, container table, tuning table. `CreateFromSettings` builds one from the three loot tables in `Director Tuning` (`LootTuningTable`, `LootContainerTable`, `ReelTable`). The `ULootConfig` data asset itself is optional (`ULootBagComponent::Config`). A missing or empty table falls back to the inline defaults in the constructor. In those defaults Fast Cut names `Haste`, Wide Shot `Magnet` and Chain Scene `Chain` (the shipped `DT_Reels` maps the same); a reel whose `UnlocksOption` is `None` never drops. `PickRandomRelic` rolls only reels that can still drop, weighted by `DropWeight` (all weights zero = uniform) |
| [`LootTypes.h`](../../Game_01%205.8/Source/Game_01/Public/Loot/LootTypes.h) | Enums and data rows: `ERelicType` (display names Fast Cut, Wide Shot, Chain Scene; the C++ names `Swiftstep`, `Warbrand`, `Aegis` predate the rename and are kept for save compatibility), `ELootContainerType`, `EBagAddResult`, `FRelicRow` (`UnlocksOption`, `DropWeight`, `IsDroppable`), `FLootContainerRow`, `FLootTuning`, plus the pure helpers `FReelUnlocks` (`AddOption`, `OptionsForReels`), which touch no UObjects so tests need no world |

## Combat

| Class | Responsibility |
|---|---|
| [`UHealthComponent`](../../Game_01%205.8/Source/Game_01/Private/Combat/HealthComponent.cpp) | Health, invulnerability window after a hit that lands (default 0.25 s), god mode (`bGodMode`), `Kill()` (ignores god mode and invulnerability). `BeginPlay` reads values from the `Hero` row of `HealthTuningTable` (default 100 health). Damage-taken multipliers are not applied here, the owner does that |
| [`UAutoWeaponComponent`](../../Game_01%205.8/Source/Game_01/Private/Combat/AutoWeaponComponent.cpp) | Each owned weapon fires automatically at its own interval, only while a run is active, no level-up offer is pending and the hero is alive. Blade hits the nearest enemy within its range, Bolt fires a line (toward the nearest enemy within range, else the facing direction) and pierces, Nova hits everything within a radius, Chain (unlocked by a lost film reel) hits the nearest enemy and jumps up to `ChainJumps` times to the nearest enemy not yet hit within `ChainRange`. Damage = `(Damage + DamagePerLevel × (Level-1) + Smithy bonus) × Might multiplier × star damage multiplier`; the star also scales reach (range, line half-width, radius, chain range). The Haste passive divides every weapon's interval. `OnWeaponFired(weapon, hits)` is broadcast after every fire. Effects are debug lines and circles |

## Enemies

| Class | Responsibility |
|---|---|
| [`AEnemy`](../../Game_01%205.8/Source/Game_01/Private/Enemies/Enemy.cpp) | Greybox enemy (engine cube, no collision), does not Tick. Grunts are red, the Boss is purple. When `ApplyDamage` reaches 0 it notifies the manager and destroys itself |
| [`UEnemyManagerSubsystem`](../../Game_01%205.8/Source/Game_01/Private/Enemies/EnemyManagerSubsystem.cpp) | Holds all live enemies and does everything in one Tick: chasing, grid-based pushing apart (each enemy only looks at the 3x3 cells around it), contact damage (each enemy has its own cooldown; it only hurts the hero while a run is active). It does nothing while a level-up offer is pending. Broadcasts `OnEnemyKilled(Kind, Location)`, which Progression uses to drop XP |
| [`UEnemySpawnSubsystem`](../../Game_01%205.8/Source/Game_01/Private/Enemies/EnemySpawnSubsystem.cpp) | Wave scheduling: while a run is active and no offer is pending, spawns Grunts on a ring around the hero (1500-1900 cm by default) at rate = `(BaseRate + RampPerMinute × minutes) × heat pressure`, up to `MaxAlive` (default 200; at the cap the banked spawns are limited so they do not arrive as a burst). `SpawnHorde` and `SpawnBoss` are called by film cards (a card horde ignores the cap). Holds the Grunt and Boss rows (Boss: 1500 health, speed 180, scale 2.5) |
| [`EnemyTypes.h`](../../Game_01%205.8/Source/Game_01/Public/Enemies/EnemyTypes.h) | `FEnemyTuning`, `FSwarmTuning`, `FSpawnTuning`, `EEnemyKind`, the `LogEnemies` log category (also used by the probes) |

## Progression: In-Run Growth

Brief v2: the level-up is "cut one, keep two". Three options are offered, the player cuts one, and the other two each add one copy of a weapon or passive. Copies are also the item's level.

| Class | Responsibility |
|---|---|
| [`URunProgressionSubsystem`](../../Game_01%205.8/Source/Game_01/Private/Progression/RunProgressionSubsystem.cpp) | XP and level (requirement to leave level `L` = `BaseXp × Growth^(L-1)`, defaults 30 and 1.3). On a level-up `GenerateOffer` builds a pool of everything that is available (an option that starts locked is available only once a lost film reel has unlocked the id equal to its table row name: `Chain`, `Haste`, `Magnet`), not cut for good and not at the copy cap, shuffles it, moves options unlocked since the player last saw an offer to the front (marked `bIsNewUnlock`, shown as `NEW! `; the list comes from `UCampStockSubsystem::TakeNewOptions`) and offers 3. `CutOffer(Index)` cuts one and applies the other two, then checks for another level-up. While `PermanentCutsLeft` > 0 a cut option also leaves the pool for the rest of the run (`PermanentCutsForPool(pool) = max(pool - MinPoolSize, 0)`, so the 7 options of a new save and `MinPoolSize` 4 give 3 permanent cuts, and each unlocked option adds one option and one cut, up to 10 and 6); after that a cut only skips the option this time. With fewer than 3 options in the pool there is nothing to cut, the player simply gets what is left. Every cut goes to `GetCuttingRoomFloor()` (reserved for a later boss, nothing but the probe reads it yet). Also: weapon levels and stars, passive stacks, the Haste (`GetAttackSpeedMultiplier`) and Magnet (`GetMagnetRadius`) results, `GetLoadoutSummary()` and `GetOfferDescriptions()` for the HUD, pickup magnetism. An enemy dies without dropping XP while an offer is pending, and a Boss drops 10x XP. Reset every run by `ResetProgression()`, and the run starts with a level 1 Blade |
| [`AXpPickup`](../../Game_01%205.8/Source/Game_01/Private/Progression/XpPickup.cpp) | XP gem, does not tick or collide, registered with the subsystem, which attracts (radius `GetMagnetRadius()`: `BaseMagnetRadius` 600 from the XP curve row plus the Magnet passive; speed 900) and collects it (radius 80) every frame, scaled by the Greed multiplier |
| [`ProgressionTypes.h`](../../Game_01%205.8/Source/Game_01/Public/Progression/ProgressionTypes.h) | `EWeaponType` (Blade, Bolt, Nova, Chain), `EPassiveType` (Swiftness, Might, Vitality, Greed, Haste, Magnet), `FXpCurveRow` (including `BaseMagnetRadius`), `FWeaponRow` (`ChainJumps`, `ChainRange`, `bStartsLocked`), `FPassiveRow` (`AttackSpeedAdd`, `MagnetRadiusAdd`, `bStartsLocked`), `FOptionUnlock::IsAvailable` (the pure unlock rule) and `FStarTuning` (pure functions, unit tested): 3 copies make 2 stars and 9 make 3 stars (also the most copies one item can hold); weapon damage x2 / x4 and reach x1.25 / x1.6; passive total effect x1.5 / x2.5; `MinPoolSize` 4. A passive's total = `stacks × per-stack amount × star multiplier`, and Vitality heals by however much its bonus grew, including a star-up jump |

## Extraction: Extraction and Camp Stock

| Class | Responsibility |
|---|---|
| [`AExtractionZone`](../../Game_01%205.8/Source/Game_01/Private/Extraction/ExtractionZone.cpp) | Extraction point: while a pawn carrying a `ULootBagComponent` stands inside and a run is in progress, the progress bar fills over `ExtractionSeconds` (default 5 seconds). Leaving empties the bar at once (`bResetProgressOnLeave`, default true) or drains it by `ProgressDecayPerSecond`. It only Ticks when someone is inside or the bar is draining. A pawn already standing in the zone when play begins is picked up on the next tick, because overlaps that exist at level start raise no events |
| [`URunOutcomeSubsystem`](../../Game_01%205.8/Source/Game_01/Private/Extraction/RunOutcomeSubsystem.cpp) | The state machine of a run, which ends only once. In order it settles into the stock (Iron, and on extraction the options unlocked by the reels carried out), writes the save (`SaveNow`), clears the bag, clears heat, and broadcasts `OnRunEnded`. Holds the earnings multiplier (set by film cards) |
| [`UCampStockSubsystem`](../../Game_01%205.8/Source/Game_01/Private/Extraction/CampStockSubsystem.cpp) | Iron and the set of unlocked upgrade options, GameInstance level. Reels are not stored: carrying one out unlocks its option at once. Also keeps `NewOptions`, the options unlocked since the player last saw an offer (`TakeNewOptions()` hands them to the next run's first offer). `OnStockChanged` fires on every change |
| [`FRunSettlement::Compute`](../../Game_01%205.8/Source/Game_01/Private/Extraction/ExtractionTypes.cpp) | Pure-function settlement: extracting yields `floor(bag Iron × multiplier)` and all lost film reels, and dying loses everything. Turning reels into unlocked options is done afterwards by `URunOutcomeSubsystem::EndRun`, so `FRunResult::OptionsUnlocked` is not filled by `Compute` |

## Editing: Pre-Run Editing

[`UFilmCardSubsystem`](../../Game_01%205.8/Source/Game_01/Private/Editing/FilmCardSubsystem.cpp) (a world subsystem, rebuilt for every level): film cards in two slots (`MaxSlots`). The card rows come from `FilmCardTable` (`FFilmCardRow` and `EFilmCardEffect` are in [`FilmCardTypes.h`](../../Game_01%205.8/Source/Game_01/Public/Editing/FilmCardTypes.h)). Card effects: `ForceHorde` (spawn a batch of Grunts at the start of the run, default 40, ignoring the alive cap), `ForceBoss` (spawn a Boss at the start of the run), and payout multipliers multiply together (default 1.5 each). The selection is stored in `UCampMetaSubsystem::PendingFilmCards`, so it survives `OpenLevel`, and is restored in `Initialize`. It is cleared when a run ends, so a choice never leaks into the next run.

## Camp

| Class | Responsibility |
|---|---|
| [`UCampMetaSubsystem`](../../Game_01%205.8/Source/Game_01/Private/Camp/CampMetaSubsystem.cpp) | Level and upgrades of the two buildings (Smithy adds flat weapon damage, default +5 per level; Clinic adds flat max health, default +25 per level; max level 5). Cost `floor(BaseCost × CostGrowth^level)` (defaults 10 and 2.0). `PendingFilmCards` |
| [`ACampGameMode`](../../Game_01%205.8/Source/Game_01/Private/Camp/CampGameMode.cpp) | When no `ACampBuilding` is placed in the camp scene, automatically spawns the Smithy and the Clinic; the warehouse is spawned separately when no `ACampWarehouse` is placed. With `-CampShot` it waits 4 seconds, logs a `CAMPSHOT` line (Iron and unlocked options), saves a screenshot to `Saved/ReelProbe/camp-storage.png` and exits |
| [`ACampPawn`](../../Game_01%205.8/Source/Game_01/Private/Camp/CampPawn.cpp) | Fixed camera (not walkable), clicking a building with the left mouse button upgrades it, creates `UCampWidget` |
| [`ACampBuilding`](../../Game_01%205.8/Source/Game_01/Private/Camp/CampBuilding.cpp), [`ACampWarehouse`](../../Game_01%205.8/Source/Game_01/Private/Camp/CampWarehouse.cpp) | Greybox blocks. A building gets taller with each level and is tinted by type (Smithy orange, Clinic green). The warehouse is only a decorative blue cube now: relics can no longer be stored or dismantled (the unlocked options are listed in the STORAGE list of the camp panel) |

## Flow: Save Game

[`UCampSaveSubsystem`](../../Game_01%205.8/Source/Game_01/Private/Flow/CampSaveSubsystem.cpp) and [`UCampSaveGame`](../../Game_01%205.8/Source/Game_01/Public/Flow/CampSaveGame.h): the single slot `CampSave` (`-SaveSlot=Name` picks another, so a probe never touches the real save) holds Iron, unlocked options, the not-yet-seen NEW option markers and building levels, with `SaveVersion` 2. A version 1 save (relics in a warehouse) is migrated at load: each relic becomes the option its reel row names. See [data-and-storage.md](data-and-storage.md) for details.

## Player: Hero

[`AHeroCharacter`](../../Game_01%205.8/Source/Game_01/Private/Player/HeroCharacter.cpp): WASD to move (walk speed 600 times the Swiftness multiplier), Space to dodge (0.18 s dash at 1800 cm/s with 0.25 s invulnerability, cooldown 0.8 s), E to search the nearest unsearched container within 250 cm, 1 through 3 to cut a level-up option while the level-up panel is shown, K for debug suicide. Movement, dodge and search are all blocked while no run is active, the hero is dead or an offer is pending (`CanAct`). Lost film reels cannot be used. `BeginPlay` calls `StartRun()` for run-start initialization (clear enemies and the kill count, reset the spawner, heat, bag and progression, set max health to base plus the Clinic bonus, start a new run, apply the slotted film cards), then `CreateHud()`.

## Game: Game Mode and Tuning Settings

| Class | Responsibility |
|---|---|
| [`ADirectorGameMode`](../../Game_01%205.8/Source/Game_01/Private/Game/DirectorGameMode.cpp) | Run game mode, default Pawn is the hero. With `-PerfProbe` on the command line it runs a 150-enemy frame rate probe (logs one `PERFPROBE` line), with `-OfferProbe` a level-up probe (forces level-ups, cuts the first option that is not the Chain each time, saves three screenshots to `Saved/OfferProbe/`, then spawns 15 enemies and saves a fourth screenshot of the first Chain attack that hits two or more of them; logs `OFFERPROBE` lines), and with `-ReelProbe` (or `-ReelProbe=die`, and `-ReelCount=N` for 1 to 3 reel types) a lost-reel extraction (or death) probe (saves screenshots to `Saved/ReelProbe/`, logs `REELPROBE` lines). Each probe puts the hero in god mode, turns the wave scheduler off and exits when done; without the flags nothing probe-related runs |
| [`UDirectorTuningSettings`](../../Game_01%205.8/Source/Game_01/Public/Game/DirectorTuningSettings.h) | "Director Tuning" in the project settings, references the 12 DataTables, values stored in [DefaultGame.ini](../../Game_01%205.8/Config/DefaultGame.ini). Also holds `StarTuning` (a struct, not a table): `DefaultGame.ini` has no entry for it, so the C++ defaults apply |

## UI

See [ui.md](ui.md).

## StandIn: Legacy Stand-In

[`AStandInCharacter`](../../Game_01%205.8/Source/Game_01/Private/StandIn/StandInCharacter.cpp), `AStandInGameMode`: a throwaway stand-in from story 001, with comments saying "replaced by story 003". Nothing in the code uses them any more. Only comments in [`HeroCharacter.h`](../../Game_01%205.8/Source/Game_01/Public/Player/HeroCharacter.h) and [`DirectorGameMode.h`](../../Game_01%205.8/Source/Game_01/Public/Game/DirectorGameMode.h) mention them. See [known-issues.md](known-issues.md).
