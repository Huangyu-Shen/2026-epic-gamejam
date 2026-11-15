# Known Issues and Technical Debt

Only issues confirmed by reading the current code are listed (checked at commit `bfe9e16`, 2026-11-15, with no tracked source file modified). Paths omit the prefix `Game_01 5.8/Source/Game_01/` (the source folder is [here](../../Game_01%205.8/Source/Game_01/)); `file:line` numbers are for that commit. Nothing was run to confirm a symptom unless the entry says so: each entry says what the code does. When you fix one, delete its entry.

## 1. Data and Save Game

### 1.1 The save version is checked only against 2
- **Location**: `Public/Flow/CampSaveGame.h:18-26` (`CurrentSaveVersion`, `SaveVersion`), `Private/Flow/CampSaveSubsystem.cpp:43` (the only migration branch), `:65-68` (cast failure), `:103-124` (`SaveNow`).
- **What happens**: A save with `SaveVersion < 2` has its `WarehouseRelics` migrated into `UnlockedOptions`; `SaveNow` writes version 2. See [data-and-storage.md](data-and-storage.md#13-version-compatibility).
- **What is left**:
  - A file with a version above 2, or a later layout change that does not bump `CurrentSaveVersion`, is read as version 2 and the next change rewrites it as version 2. `NewOptions` was already added to version 2 without a bump (`CampSaveGame.h:44`).
  - The class default of `SaveVersion` must stay 1 or old files skip the migration (comment at `CampSaveGame.h:20-24`).
  - The migration is one-way: `WarehouseRelics` is never written back, and `FReelUnlocks::OptionsForReels` (`Public/Loot/LootTypes.h:97`) skips a reel whose row has no `UnlocksOption`.
  - If the slot exists but cannot be cast to `UCampSaveGame`, `Initialize` logs a warning and starts a fresh camp; the next change overwrites the slot.
  - Enums are stored by name (`ERelicType`, `EBuildingType` keys), so renaming an enumerator silently drops old data.
- **Suggestion**: When the layout changes again, bump `CurrentSaveVersion`, add a migration branch, and refuse or back up a file with a higher version.

### 1.2 A save is written several times per event, and an early write can be an intermediate state
- **Location**: `Private/Camp/CampMetaSubsystem.cpp:97` and `:107` (`TryUpgrade`), `Private/Extraction/RunOutcomeSubsystem.cpp:46-66` (`EndRun`), `Private/Flow/CampSaveSubsystem.cpp:75-76,95-101` (listeners).
- **Symptom**: Every `OnStockChanged` / `OnMetaChanged` triggers `SaveNow`. An upgrade writes once after the cost is spent (resource gone, level not raised) and again after the level rises. A run settlement writes after `AddIron` (if wood > 0), after `AddStone` (if stone > 0), after each newly unlocked option, and once more explicitly (1 to 3+N writes for N new options); the first write holds the new wood without the unlocks.
- **Impact**: A kill between two writes leaves an intermediate state. The window is within one frame, but the file is rewritten needlessly.
- **Suggestion**: Finish changing all state, then save once (suppress the listener during `EndRun` and `TryUpgrade`).

### 1.3 A missing data table or row falls back to C++ defaults silently
- **Location**: `FindRow(..., false)` lookups at `Private/Enemies/EnemySpawnSubsystem.cpp:73-94`, `Private/Enemies/EnemyManagerSubsystem.cpp:18-24`, `Private/Combat/HealthComponent.cpp:17-23`, `Private/Progression/RunProgressionSubsystem.cpp:116-152`, `Private/Camp/CampMetaSubsystem.cpp:49-58`, `Private/Editing/FilmCardSubsystem.cpp:18-22`, `Private/Loot/LootConfig.cpp` (whole file).
- **Symptom**: An unset table, or a misspelled row name, keeps the C++ default with no log line; an unset film-card table means no cards at all. Only `ULootBagComponent::BeginPlay` (`Private/Loot/LootBagComponent.cpp:26`) logs where its numbers came from (`LOOTCFG source=table` or `c++ defaults`). In a package the tables are soft references, which is why `DefaultGame.ini` lists `DirectoriesToAlwaysCook=/Game/Data`.
- **Loot variant**: `ULootConfig::FindContainerRow` (`LootConfig.cpp:103-131`) uses the table rows if it has any, with no per-type fallback. If `DT_LootContainers` has rows but none whose `ContainerType` field matches, `ALootContainer::Interact` logs a warning and returns false (`Private/Loot/LootContainer.cpp:97-100`) and that container can never be searched. Lookups match the row field, not the row name, and take the first match. `FindRelicRow` has no caller at all.
- **Duplicated numbers**: the C++ fallbacks (`RunProgressionSubsystem.cpp:18-72`, `LootConfig.cpp:5-41`, `CampMetaSubsystem.cpp` `DefaultBuilding`) are a second copy of what the tables hold, and `Private/Tests/UnlockableOptionsTest.cpp:16-23` is a third copy of the option list. They can drift apart without any check.
- **Suggestion**: Log a warning when a table is set but a row is missing, or unset in a non-editor build.
- Not verified: the row values inside the `.uasset` tables (binary, not read here).

### 1.4 The unlock loop has three leftovers
- **Location**: `Public/Extraction/CampStockSubsystem.h:39` (`IsOptionUnlocked`, no caller), `Public/Loot/LootTypes.h:76-79` (`IsDroppable`), `Private/Progression/RunProgressionSubsystem.cpp:336-415` (`GenerateOffer`).
- **What is left**:
  - `IsOptionUnlocked` has no caller; the progression subsystem copies the whole list through `GetUnlockedOptions` (`RunProgressionSubsystem.cpp:261`), also once per `GetPoolSize` call.
  - Once all three options are unlocked, `PickRandomRelic` finds nothing droppable, so no container drops a reel, even a vault with `RelicChance` 1.0. Carrying reels out earns nothing after that.
  - The NEW markers are taken (`TakeNewOptions`, `:392`) only by an offer that shows three options. When the pool has fewer than three options the auto-apply path (`:378-385`) returns before that.
  - `TakeNewOptions` clears all markers although only three can be shown; with more than three new unlocks the extra ones lose their NEW mark (today there are only three lockable options).

### 1.5 A table `MaxLevel` below 9 is ignored
- **Location**: `Public/Progression/RunProgressionSubsystem.h:203` (`MaxCopies`), used at `RunProgressionSubsystem.cpp:315,347,361,426,437`.
- **Symptom**: The copy cap is `max(row MaxLevel, CopiesForThreeStar)`. The defaults have `MaxLevel` 5 but the cap is 9, so a `MaxLevel` below 9 in `DT_Weapons` / `DT_Passives` changes nothing.
- **Impact**: Misleading field; a designer cannot shorten an item's ladder.

### 1.6 Gameplay numbers still hardcoded in C++
The project rule is that values live in data ([master PRD section 5](../../design/prd/master-prd.md)). Still in code:
- **Economy rules**: the Smithy costs stone and the Clinic costs wood (`Public/Camp/CampMetaSubsystem.h:32`, not a field of `FBuildingRow`); crates give wood and vaults give stone (`Private/Loot/LootContainer.cpp:135`); the bag counts wood and stone slots with the same `IronPerSlot`.
- **Progression**: Boss XP is 10 times a gem (`RunProgressionSubsystem.cpp:309`); XP collect radius 80 and gem pull speed 900 (`RunProgressionSubsystem.h:228,231`).
- **Hero**: walk speed, interact range, camera distance, dodge numbers (`Public/Player/HeroCharacter.h:20-67`) are `UPROPERTY` defaults, not table rows; the Hero capsule/cylinder scales are in the constructor (`HeroCharacter.cpp:49-56`).
- **Weapons**: Gun bullet cap 5 and spread 9 degrees, Orbit ball cap 6, hit radii 70 and 65, beam thicknesses and all weapon colors (`Private/Combat/AutoWeaponComponent.cpp:116-229,272,305`); boss ball colors and size (`BossRejectsSubsystem.cpp:15-16,149`).
- **Camp**: layout radius 700 and building positions/rotations (`Private/Camp/CampGameMode.cpp:18,66-67,80`), model scales 0.45 / 0.6, click cube sizes 4 / 7, growth 0.08 per level, offset 60 (`Private/Camp/CampBuilding.cpp:74,101-105`), warehouse scales 0.55 / 6 (`CampWarehouse.cpp:38-39`), camp camera arm 2300 and pitch -75 (`CampPawn.cpp`). The greybox growth 0.5 per level and tint colors are at `CampBuilding.cpp:110-114`, enemy colors at `Private/Enemies/Enemy.cpp:74-75`.
- **Enemies and look**: the list of five grunt models, their scale 1.2 and the boss scale 0.85 (`EnemySpawnSubsystem.cpp:49-62`); forest counts 160 / 40 / 10 / 500 and rim sizes (`Private/Game/ForestDressing.cpp:15-16,129-147`); play-area thresholds (`Private/Game/PlayAreaSubsystem.cpp:15-16`).
- **Film cards**: slot count `MaxSlots = 2` (`Public/Editing/FilmCardSubsystem.h:22`); extraction time and reset rule are properties of the placed zone (`Public/Extraction/ExtractionTypes.h:115-124`), so they live in the map.
- **Never read**: `FRelicRow::DisplayName`, `FLootContainerRow::DisplayName`, `FBuildingRow::DisplayName` (`LootTypes.h:59,125`, `CampTypes.h:29`) are only assigned in `LootConfig.cpp`. The HUD and toasts use the enum display names, so editing the name in a table does nothing.

### 1.7 `project.yaml` still points at the warm-up project
- **Location**: `commands.build/test/run/smoke` (lines 59-62) and `engine.language` (line 35) in [project.yaml](../../project.yaml).
- **Symptom**: All four commands use `Game_Jam_01/Game_Jam_01.uproject` (not in the repository), and `engine.language` is `"Blueprint"` for a C++ project. The build command has no staging or packaging options.
- **Impact**: Skills that run these commands (`/smoke-check`, `/perf-profile`) point at the wrong project. Changing `project.yaml` needs user approval.

### 1.8 Fields named Iron now hold wood (and vault yields hold stone)
- **Location**: `Public/Extraction/CampStockSubsystem.h:68,79` (`Iron`, `GetIron`, `AddIron`, `SpendIron`, `GetWood` returns `Iron`), `Public/Loot/LootBagComponent.h:54,98,104` (`IronCount`, `TryAddIron`, `SlotsForIron`), `Public/Flow/CampSaveGame.h:30` (`Iron`), `Public/Extraction/ExtractionTypes.h:50-58` (`BagIron`, `IronGained`, `IronLost`), `Public/Loot/LootTypes.h:128,131,153` (`IronMin`, `IronMax`, `IronPerSlot`), log text `CAMPSHOT iron=`, `LOOTCFG ironPerSlot=` (`CampGameMode.cpp:101`, `LootBagComponent.cpp:26`).
- **Symptom**: Wood kept its old name. `IronMin` / `IronMax` are the yield range of a vault in stone too, and `TryAddLoot` passes the resource separately. Several comments still say Iron (`CampMetaSubsystem.h:41`, `CampTypes.h:31`, `RunOutcomeSubsystem.h:39,46`).
- **Impact**: Readers and table editors are misled; the save field name cannot change without a migration.
- **Suggestion**: Rename in code (`Wood`) behind a `UPROPERTY` redirect / save migration, or at least keep the aliases documented in one place.

## 2. Visuals and Assets

### 2.1 The hero, enemies and XP gems are gray, and tinting has no effect
- **Status**: **fixed for the cubes and cylinder** (the heading is kept so older links work). `HeroCharacter.cpp:55-58`, `Enemy.cpp:29-33`, `XpPickup.cpp:24-28` now set `/Game/Materials/M_FlatColor` (tracked in git) before creating the dynamic material, so the `Color` parameter works.
- **Still true with the Fab models installed**: `AEnemy::SetVisual` (`Enemy.cpp:52`) removes the cube mesh, and `Init` (`:77`) / `ApplyDamage` (`:105-112`) tint the material of that hidden cube. The white hit flash and the grunt / boss color therefore do nothing on a model; only the 1.2x scale punch of the whole body shows. The hero's `BodyMesh` is also tinted in `BeginPlay` (`HeroCharacter.cpp:132`) with no mesh when the pack is present.
- **Suggestion**: Tint the skeletal mesh's material instances, or drop the dead calls.

### 2.2 The camp message text does not disappear by itself
- **Location**: `Message` in `Private/UI/CampWidget.cpp:62,118-120,190-218`.
- **Symptom**: The "Upgrade succeeded / Not enough wood / Card added" line at the top of the panel stays until the next action. It is part of the rebuild signature. Cosmetic; 3D clicks use the expiring toasts.

### 2.3 Asset packs from Fab are not in git; the code reaches them by hard path
- **Location**: 33 pack asset paths in 7 classes, all `ConstructorHelpers::FObjectFinder`: hero (`HeroCharacter.cpp:64-68`), enemies (`EnemySpawnSubsystem.cpp:39-62`, 6 meshes + 6 animations), containers (`LootContainer.cpp:27-28`), extraction portal (`ExtractionZone.cpp:62`), camp buildings (`CampBuilding.cpp:45-48`), warehouse (`CampWarehouse.cpp:34`), forest (`ForestDressing.cpp:37-42`). Sounds and music are soft references in `Config/DefaultGame.ini` (`Sounds`, `CampMusic`, `RunMusic`).
- **What happens without them** (a fresh clone has only `Content/Data`, `Content/Maps`, `Content/Materials`): each failed finder is handled in code (cube, cylinder, no portal, no forest, no sound), but the engine also logs `CDO Constructor (<class>): Failed to find <path>` at Error level (`UObjectGlobals.cpp:4932` in the engine source). The finders in `EnemySpawnSubsystem`, `CampBuilding` and `ForestDressing` are not `static`, so they run again every time such an object is constructed. The hero model needs all three of mesh, run and walk (`HeroCharacter.cpp:70`); the clinic house needs all five parts (`CampBuilding.cpp:49-53`). The forest is skipped with `FOREST dressing skipped (pack missing)` (`PlayAreaSubsystem.cpp:44`) and the grey template floor stays.
- **Impact**: Works, but a log full of errors can hide real ones, and nobody has cooked a package without the packs; `DefaultGame.ini` also lists two pack folders in `DirectoriesToAlwaysCook` (`/Game/Free_Sounds_Pack/cue`, `/Game/PathOfAdventure/Loop/CUE`) whose behaviour when absent is not verified.
- **Static finders cache a failure**: the `static` finders (hero, containers, portal, ...) are built once per process, so importing a pack into an already running editor needs an editor restart before the code sees it.
- **Suggestion**: Load by soft path at runtime with one log line per missing pack, and keep the `CDO` constructors free of pack paths.

### 2.4 The camp map seems to be a copy of the run map
- **Location**: `Content/Maps/L_Camp.umap` (binary). It contains the same names as `L_Greybox.umap`: `ExtractionZone_0/_1/_A/_B`, `LootContainer_0` to `_4` and `PlayerStart_0`, and no placed `ACampBuilding` (the code layout is used, `CampGameMode.cpp:53-58`).
- **Impact**: Not verified in the editor: if these actors are really in the camp, the camp holds an extraction portal, beams and stumps nobody can use (the camp pawn has no bag, so `TryEnter` never fires).

## 3. Legacy and Dead Code

| Item | Location | Description |
|---|---|---|
| Stand-in character and game mode | `Public/StandIn/`, `Private/StandIn/` (4 files, 235 lines) | Throwaway from story 001 ("replaced by story 003"). No map references it (checked by searching both `.umap` files for the name), but it is compiled. Its `N` key and debug text are the only users of `GetIronCount` and `StartNewRun` from a key |
| `EnhancedInput` module | `Game_01.Build.cs:10` | Declared, but no source file includes it; input is raw keys |
| Clearing of old debug text | `RunProgressionSubsystem.cpp:179-186,504-511` | `AddOnScreenDebugMessage(..., TEXT(""))` for keys 2200 to 2203; the text they clear was removed long ago |
| Debug kill key | `HeroCharacter.cpp:194,255` | `K` kills the hero (`Health->Kill()`) in every build, including a package; nothing strips it |
| Probes and test hooks in all builds | `Private/Game/DirectorGameMode.cpp:46-268`, `RunProgressionSubsystem.h:168-171` (`DebugGrantWeapon`, `DebugCut`) | No `UE_BUILD_SHIPPING` / `WITH_EDITOR` guard anywhere in the source (searched); the nine probes (god mode, time dilation, forced XP) work in any build if the flag is passed |
| `DeleteSaveAndReset()` | `Private/Flow/CampSaveSubsystem.cpp:126` | No caller, so `UCampMetaSubsystem::ResetCamp` (`CampMetaSubsystem.cpp:134`) has none either |
| Written but never read | `AutoWeaponComponent.h:59` `OwnerBag` (assigned at `.cpp:23`); `CampBuilding.h:61` `CachedLevel` (`.cpp:96`); `GameAudioSubsystem.cpp:126` `Played[]` | Dead state |
| No caller | `GetPassiveStars` (`RunProgressionSubsystem.h:124`), `IsOptionUnlocked`, `FindRelicRow`, `GetKillCount` (`EnemyManagerSubsystem.h:54`), `GetRunTime` (`EnemySpawnSubsystem.h:71`), `GetRequestCount`, `GetCappedCount` (`GameAudioSubsystem.h:78,81`), `GetAttackCount` (`BossRejectsSubsystem.h:125`), `ShowOffer` (only logs, `RunProgressionSubsystem.cpp:516`) | |
| Delegates without a C++ listener | `OnBagChanged`, `OnPickupRefused`, `OnHeatChanged`, `OnSearched`, `OnDamaged`, `OnExtractionProgress`, `OnExtractionInterrupted` | Broadcast, never bound; the HUD polls |
| The warehouse | `CampWarehouse.cpp`, `CampGameMode.cpp:76-81` | Decorative only (relics cannot be dismantled), but still spawned and it takes clicks (`BlockAll`, 6 m cube) |
| Fixed since the last check | | `DrawDebugLine` weapon feedback is gone (replaced by `AWeaponEffect`, drawn in Shipping too); `DirectorUi::TextColor` is now used (`RunHudWidget.cpp:109,117`); `GetCuttingRoomFloor` is now read by `UBossRejectsSubsystem` (`BossRejectsSubsystem.cpp:212`) |

### 3.1 Stale comments
- `Public/Player/HeroCharacter.h:44` ("1-3 cut a level-up option": 1-3 take, 4-6 cut) and `:97` (`ReceiveEnemyDamage` "bag's damage-taken multiplier": it applies the Armor passive, `HeroCharacter.cpp:288`).
- `Public/Game/DirectorGameMode.h:51-63`: the `-OfferProbe` "cuts the first option each time" (it keeps the option containing "Chain" and cuts another, `DirectorGameMode.cpp:435-455`); the list of probes omits `-WeaponShowcase`, `-PauseProbe`, `-EdgeProbe`, `-BossProbe`, `-ZoneShot`.
- `Public/Game/DirectorTuningSettings.h:13` ("every gameplay number", see 1.6) and `:92` ("There is no settings screen": the Esc menu has one).
- `Public/Extraction/CampStockSubsystem.h:12,22,26,50,59-60` (Iron, "or the warehouse") and `CampSaveSubsystem.h:10-11`, `CampWidget.h:1,12,36`, `CampPawn.cpp:54` ("warehouse dismantle").
- `Public/Camp/CampTypes.h:31` and `CampMetaSubsystem.h:41` ("Iron cost").
- `Public/Progression/RunProgressionSubsystem.h:43-46` ("the player CUTS one option and keeps the other two": the rule is now take one of three, with an optional cut).
- `Public/Editing/FilmCardSubsystem.h:14` ("driven by debug keys": there are none); `Public/Enemies/EnemyTypes.h:37` ("hero's damage-taken multiplier": none); `Public/Loot/LootConfig.h:11` ("Create a Data Asset": the tables are used); `Public/Enemies/BossRejectsSubsystem.h:121` ("Starts with no cuts applied": `Processed = 0` applies every earlier cut on the next tick).

## 4. Architecture and Robustness

- **Few automated tests**: 12 (4 star-tuning, 5 lost-reel, 3 unlockable-option) in `Private/Tests/`. They cover pure helpers only. Nothing tests `GenerateOffer`, the NEW marker, settlement math (`FRunSettlement::Compute`), save load, building costs or the boss subsystem. `UnlockableOptionsTest` uses its own 16-option table, so a wrong `bStartsLocked` in the real data is not caught. Under `modes.rigor: minimal` tests are optional; this is a permitted gap.
- **No guard against "a world without a GameInstance"**: `GetWorld()->GetGameInstance()->GetSubsystem<...>()` dereferences null in an editor preview world. `UFilmCardSubsystem::Initialize` was fixed after crashing once; the old pattern remains at `AutoWeaponComponent.cpp:98,326`, `RunOutcomeSubsystem.cpp:46,64`, `LootContainer.cpp:108`, `CampBuilding.cpp:79,88`, `CampPawn.cpp:114`, `HeroCharacter.cpp:273` and in the probes (`DirectorGameMode.cpp:276,349`). Safe while the game runs.
- **`EndRun` does not null-check the bag**: `RunOutcomeSubsystem.cpp:44` calls `Bag->GetWoodCount()` first.
- **Progression binds to the enemy manager without declaring the dependency**: `RunProgressionSubsystem.cpp:154` uses `GetSubsystem<>` and no `Collection.InitializeDependency`, unlike `UFilmCardSubsystem` and the camp subsystems. If the manager initialized later, the `OnEnemyKilled` binding would be skipped silently and no XP gem would drop. It works today.
- **The auto-apply path of `GenerateOffer` does not re-check the level**: `RunProgressionSubsystem.cpp:378-385` returns without `CheckLevelUp`; `ChooseOption` does call it (`:513`). Only reachable late in a run.
- **Soft pause is not complete**: while a level-up offer is shown (`IsOfferPending`), `AExtractionZone::Tick` (`ExtractionZone.cpp:175-217`) keeps filling the bar, so a hero standing in the zone can extract behind the panel (the HUD hides the offer once the run is inactive). The dodge branch of `AHeroCharacter::Tick` (`HeroCharacter.cpp:365-370`) does not test `CanAct()`: a dash keeps moving the hero for up to 0.18 s after the panel opens, after death or after the run ended. `AWeaponEffect` (a boss slam warning) keeps ticking while `UBossRejectsSubsystem` is frozen, so the red ring can vanish before its slam lands (`BossRejectsSubsystem.cpp:178-181` against `:217,270`). Engine time is not stopped, so absolute-time timers run on: hit and dodge invulnerability expire, enemy `NextContactTime` and the boss `NextUse` times pass, and after the offer closes overdue attacks fire at once.
- **The Esc menu is a hard pause, not the soft pause**: `UPauseMenuWidget::Open` (`PauseMenuWidget.cpp:115`) calls `SetGamePaused(true)` in a run. That stops all world timers, so probes cannot use `FTimerManager` while it is open (`DirectorGameMode.cpp:99-112` works around it with `FTSTicker`), and `Close` / `PressReturnToCamp` call `SetGamePaused(false)` (`:127,175`), also in the camp where `Open` never paused. Esc is blocked while an offer is pending (`HeroCharacter.cpp` `QuitKey`). Quitting from the menu (`:202`) ends the process without ending the run. Whether the music keeps playing while paused was not checked.
- **`UPlayAreaSubsystem` finds the floor by shape**: `PlayAreaSubsystem.cpp:15-36` takes every `AStaticMeshActor` whose bounds (all components, colliding or not) are at least 1000 cm half-size and thinner than 20 percent. The play area is the bounding box of all of them, so two separate floor pieces make the gap between them walkable, and only the last piece is hidden by the forest dressing and gives its top height to it (`:33-34`, `ForestDressing.cpp:87-91`). With no match it only logs a warning and nothing keeps the hero or enemies on the map. One `PLAYAREA candidate` log line is written for every static mesh actor. The clamp is not used in the camp (the camp pawn does not move) and the camp does not need it.
- **Cuts feed a boss only when the Cut Knight card is slotted**: `SpawnBoss` has callers only in `FilmCardSubsystem.cpp:131` (the `ForceBoss` card) and probes. Without that card there is no boss, yet the level-up text says the cut "gives it to the Boss" (`RunHudWidget.cpp:109`) and a toast says "CUT: X now belongs to the Boss" (`RunProgressionSubsystem.cpp:488`). A boss registered later receives all earlier cuts at once and a burst of toasts. `SetBoss` replaces the previous boss.
- **`UBossRejectsSubsystem::Tick` returns early when the run is inactive** (`BossRejectsSubsystem.cpp:206`) without `ClearAttacks`; balls in flight stay frozen on the result screen (cleared only when the world ends or when the boss dies).
- **Weapon oddities**: Gun reports `Hits = 1` for any shot (`AutoWeaponComponent.cpp:231`), so the attack sound plays and the showcase hook fires even if the bullet later misses; the Orbit balls stay frozen in place after the hero dies or the run ends until `EndPlay` (`TickComponent` returns early, `:32-41`, `EndPlay` at `:250`).
- **Contact cooldown is spent while the hero is invulnerable**: `UEnemyManagerSubsystem::Tick` sets `NextContactTime` (`EnemyManagerSubsystem.cpp:124-125`) even when `ReceiveEnemyDamage` returned 0 (dodge, hit invulnerability, god mode), so an enemy waits 0.8 s after a hit that did nothing.
- **XP gems are never capped**: each kill spawns an actor (`RunProgressionSubsystem.cpp:307`) that lives until collected or the next run; gems left outside the magnet radius accumulate. Not measured.
- **`CampBuilding` binds and never unbinds**: `CampBuilding.cpp:80` adds `OnMetaChanged` and there is no `EndPlay` removal; safe today because the subsystem outlives the building only within one session (dynamic delegates drop dead objects).
- **`HeroModelScale` and `HeroModelYaw` are read only in the constructor** (`HeroCharacter.cpp:61-63`), so an override on a placed instance or in `Config` has no effect on the model.
- **Per-frame work in the UI**: `URunHudWidget::NativeTick` rebuilds the loadout string and iterates every actor for the extraction zones each frame (`RunHudWidget.cpp:172,195`); `UCampWidget::MakeSignature` builds a string every frame (`CampWidget.cpp:60-94`). Not measured; the 150-enemy probe does not include the HUD cost separately.
- **The probes touch the real save and share timing**: `-ReelProbe` settles a real run (unlocks, wood, `CampSave`) unless `-SaveSlot=Name` is passed (`CampSaveSubsystem.cpp:12-21`). It reads its delay and step from `OfferProbe` (`DirectorGameMode.cpp:267`), so changing those changes both. `-PaceProbe` sets global time dilation and does not reset it (`:309`).
- **Hardcoded map names**: `L_Camp` and `L_Greybox` are strings in `CampWidget.cpp:233`, `PauseMenuWidget.cpp:176` and `RunHudWidget.cpp:285`.
- **Dependency cycles, unchecked**: see [dependencies.md](dependencies.md#1-how-modules-depend-on-each-other).
- **Development plugins are enabled for all targets**: `ModelContextProtocol`, `AllToolsets` and `Terminal` have no `TargetAllowList` in `Game_01.uproject`, so they go into the package. Restrict them to Editor before release.
- **The camp panel is rebuilt wholesale** when its signature changes, so hover state is lost after a click.
- **Checked and fine**: every handler bound with `AddDynamic` (`HandleDied`, `HandleMetaChanged`, `HandleRunEnded`, `HandleEnemyKilled`, `HandleCampChanged`, zone overlaps, `HandleClicked`, the three `HandleAction`) is a `UFUNCTION`.

## 5. Version Comparison

The only release tag, `jam-start`, predates the UE project, so save or config-key compatibility cannot be compared. The save format has two versions (1 = relics in a warehouse, 2 = unlocked options, wood in the `Iron` field, `Stone` and `NewOptions` added without a bump). See [data-and-storage.md](data-and-storage.md#13-version-compatibility).
