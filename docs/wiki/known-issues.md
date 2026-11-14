# Known Issues and Technical Debt

Only issues that have been confirmed against the code are listed. Paths omit the prefix `Game_01 5.8/Source/Game_01/`. Sorted by impact.

## 1. Data and Save Game

### 1.1 The save version is checked only against 2 (partly fixed in story 008)
- **Location**: `SaveVersion` in [CampSaveGame.h](../../Game_01%205.8/Source/Game_01/Public/Flow/CampSaveGame.h), `Initialize` in [CampSaveSubsystem.cpp](../../Game_01%205.8/Source/Game_01/Private/Flow/CampSaveSubsystem.cpp).
- **Status**: Story 008 added the check: a save with `SaveVersion < 2` has its `WarehouseRelics` migrated into `UnlockedOptions`, and `SaveNow` writes version 2. See [data-and-storage.md](data-and-storage.md#13-version-compatibility).
- **What is left**: The load code has only that one branch. A file with a version above 2, or a later layout change that does not bump `CurrentSaveVersion`, is still read as if it were version 2. The class default of `SaveVersion` must stay 1 or old files will skip the migration.
- **Suggestion**: When the layout changes again, bump `CurrentSaveVersion` and add the next migration branch.

### 1.2 An upgrade writes the save twice, and the intermediate state is inconsistent
- **Location**: [`UCampMetaSubsystem::TryUpgrade`](../../Game_01%205.8/Source/Game_01/Private/Camp/CampMetaSubsystem.cpp), [`UCampStockSubsystem::SpendIron`](../../Game_01%205.8/Source/Game_01/Private/Extraction/CampStockSubsystem.cpp).
- **Symptom**: `SpendIron` broadcasts `OnStockChanged` first and the save subsystem writes to disk immediately, when the level has not yet gone up. Then the level is incremented, `OnMetaChanged` is broadcast, and it writes again.
- **Impact**: If the process is killed between the two writes, the save holds "Iron deducted, no upgrade". The window is tiny (within the same frame) but it exists.
- **Suggestion**: In the upgrade flow, finish changing both pieces of state first, then save once.

### 1.3 Fixed in story 008: the dismantle Iron and the missing loot config
- The Iron for dismantling a relic (`RelicDismantleIron`, 8) is gone with the dismantle action, so nothing hardcoded is left there.
- The loot numbers (bag capacity, heat, container and reel values) used to come only from C++ defaults. [`ULootConfig::CreateFromSettings`](../../Game_01%205.8/Source/Game_01/Private/Loot/LootConfig.cpp) now builds the config from the three loot tables in `Director Tuning`, so they are tuned like the other tables (see [data-and-storage.md](data-and-storage.md#2-tuning-data-tables)). The `ULootConfig` data asset itself is still optional and the project has none.
- **What is left**: A table that is not set, or a missing `Default` row, still falls back silently to the C++ defaults. `ULootBagComponent::BeginPlay` logs `LOOTCFG source=table` or `source=c++ defaults`, so check that line in a packaged build's log.
- **Not verified**: the row values inside the `.uasset` tables, which this wiki cannot read.

### 1.5 `project.yaml` still points at the warm-up project
- **Location**: `commands.build/test/run/smoke` and `engine.language` in [project.yaml](../../project.yaml).
- **Symptom**: The project path in all four commands is `Game_Jam_01/Game_Jam_01.uproject`, and `engine.language` is `"Blueprint"`, while the actual project is `Game_01 5.8/` and C++.
- **Impact**: Skills (such as `/smoke-check` and `/perf-profile`) that run these commands will point at a project that is not in the repository. Changing `project.yaml` needs user approval.

## 2. Visuals

### 2.1 The hero, enemies and XP gems are gray, and tinting has no effect
- **Location**: [`AHeroCharacter::BeginPlay`](../../Game_01%205.8/Source/Game_01/Private/Player/HeroCharacter.cpp), [`AEnemy::Init`](../../Game_01%205.8/Source/Game_01/Private/Enemies/Enemy.cpp), [`AXpPickup::BeginPlay`](../../Game_01%205.8/Source/Game_01/Private/Progression/XpPickup.cpp).
- **Symptom**: These three places create a dynamic material on a cylinder or cube and set the `Color` parameter (the comments say "hero blue, enemy red, Boss purple, gem green"), but everything in the game is gray.
- **Cause**: The material that comes with the engine's basic shape meshes has no `Color` parameter, so `SetVectorParameterValue` fails silently. The camp's [`ACampBuilding`](../../Game_01%205.8/Source/Game_01/Private/Camp/CampBuilding.cpp) and [`ACampWarehouse`](../../Game_01%205.8/Source/Game_01/Private/Camp/CampWarehouse.cpp) (now only a decorative cube) explicitly `SetMaterial` to `/Engine/BasicShapes/BasicShapeMaterial`, so they are tinted correctly (visible in the screenshots).
- **The old wiki's statement was wrong**: The old document said "`BasicShapeMaterial` has no `Color` parameter". The reverse is true: `BasicShapeMaterial` has it, and what lacks it is the mesh's default material.
- **Suggestion**: In these three places, call `SetMaterial(0, BasicShapeMaterial)` before creating the dynamic material.

### 2.2 The camp message text does not disappear by itself
- **Location**: `Message` in [`UCampWidget`](../../Game_01%205.8/Source/Game_01/Private/UI/CampWidget.cpp).
- **Symptom**: The "Upgrade succeeded / Not enough Iron" line at the top of the panel stays until the next action.
- **Impact**: Cosmetic only. 3D clicks go through the expiring `UHudMessageSubsystem` messages.

## 3. Legacy and Dead Code

| Item | Location | Description |
|---|---|---|
| Stand-in character and game mode | [StandIn/](../../Game_01%205.8/Source/Game_01/Public/StandIn/) | A throwaway stand-in from story 001. The comments say "replaced by story 003". Nobody uses it any more, but it still gets compiled |
| `EnhancedInput` module | [Game_01.Build.cs](../../Game_01%205.8/Source/Game_01/Game_01.Build.cs) | Declared, but the source has no `#include` of it |
| Calls that clear the old upgrade text | [`ResetProgression`, `PickOffer`](../../Game_01%205.8/Source/Game_01/Private/Progression/RunProgressionSubsystem.cpp) | Call `AddOnScreenDebugMessage(..., TEXT(""))` for keys 2200 through 2203. The old debug text they correspond to was deleted long ago, so these calls now have no effect |
| Callers of `StartNewRun` | [HeroCharacter.cpp](../../Game_01%205.8/Source/Game_01/Private/Player/HeroCharacter.cpp) | It used to be triggered by the N key and is now called only once in `BeginPlay`. There is no "another run" in the game, and every run reloads the run map |
| Weapon debug drawing | [AutoWeaponComponent.cpp](../../Game_01%205.8/Source/Game_01/Private/Combat/AutoWeaponComponent.cpp) | Every shot uses `DrawDebug*` to draw a 0.12-second guide line. The engine compiles it out of Shipping builds, so a Shipping package has no visual weapon feedback |
| `DeleteSaveAndReset()` | [CampSaveSubsystem.cpp](../../Game_01%205.8/Source/Game_01/Private/Flow/CampSaveSubsystem.cpp) | Has no callers |

## 4. Architecture and Robustness

- **Few automated tests**: There are 9 (4 star-tuning tests and, since story 008, 5 lost-reel tests under `Private/Tests/`). The comment on [`FRunSettlement::Compute`](../../Game_01%205.8/Source/Game_01/Private/Extraction/ExtractionTypes.cpp) says "pure function, can be tested without opening the editor", but there is still no test for the settlement math itself. Under `qa.level: minimal` tests are optional, so this is a permitted gap and not a violation.
- **No guard against "a world without a GameInstance"**: `GetWorld()->GetGameInstance()->GetSubsystem<...>()` dereferences null in an editor preview world. In story 007, `UFilmCardSubsystem::Initialize` crashed because of this once, and was changed to `UGameInstance::GetSubsystem<T>(GetWorld()->GetGameInstance())` (which returns null for null). The same old pattern remains in [`UAutoWeaponComponent::FireWeapon`](../../Game_01%205.8/Source/Game_01/Private/Combat/AutoWeaponComponent.cpp), [`URunOutcomeSubsystem::EndRun`](../../Game_01%205.8/Source/Game_01/Private/Extraction/RunOutcomeSubsystem.cpp) and [`ACampBuilding`](../../Game_01%205.8/Source/Game_01/Private/Camp/CampBuilding.cpp). They only execute while the game is running, so they are currently safe. When writing a new world subsystem, do not dereference the GameInstance directly in `Initialize`.
- **Hardcoded map names**: `L_Camp` and `L_Greybox` appear as strings in [CampWidget.cpp](../../Game_01%205.8/Source/Game_01/Private/UI/CampWidget.cpp) and [RunHudWidget.cpp](../../Game_01%205.8/Source/Game_01/Private/UI/RunHudWidget.cpp). Renaming them requires updating both.
- **Dependency cycles, unchecked**: see [dependencies.md](dependencies.md#1-how-modules-depend-on-each-other).
- **The `ModelContextProtocol` and `AllToolsets` plugins are enabled for all targets**: development tools will be carried into the package. Restrict them to Editor before release.
- **The camp panel is rebuilt wholesale**: when the state signature changes, it clears and rebuilds, and hover state is lost after a click. With a very long list (many unlocked options) it still relies on the scroll box.

## 5. Version Comparison

The most recent release tag `jam-start` predates the UE project, so save or config-key compatibility cannot be compared. The save format has two versions (1 = relics in a warehouse, 2 = unlocked options, with a migration from 1). See [data-and-storage.md](data-and-storage.md#13-version-compatibility).
