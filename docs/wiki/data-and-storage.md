# Data and Storage

Paths omit the prefix `Game_01 5.8/`.

## 1. Save Game

A single save slot named `CampSave`, user slot 0. Implementation: [CampSaveSubsystem.cpp](../../Game_01%205.8/Source/Game_01/Private/Flow/CampSaveSubsystem.cpp). Data class: [CampSaveGame.h](../../Game_01%205.8/Source/Game_01/Public/Flow/CampSaveGame.h).

File location: `Saved/SaveGames/CampSave.sav` (confirmed to exist, about 2.5 KB). `Saved/` is listed in [.gitignore](../../.gitignore), so the save does not go into the repository.

### 1.1 Contents

| Field | Type | Source | How it is restored |
|---|---|---|---|
| `SaveVersion` | `int32`, currently 1 | Constant | **Not checked on load**, see [known-issues.md](known-issues.md) |
| `Iron` | `int32` | `UCampStockSubsystem::GetIron()` | `RestoreState`, negative values clamped to 0 |
| `WarehouseRelics` | `TArray<ERelicType>` | `GetWarehouseRelics()` | `RestoreState` |
| `BuildingLevels` | `TMap<EBuildingType, int32>` | `UCampMetaSubsystem::GetLevels()` | `RestoreLevels`, each entry clamped to 0 through `MaxLevel`, unknown types discarded |

**Not saved**: the film cards chosen before a run (`PendingFilmCards`, which only carry across levels within one session), and any in-run state (bag, heat, XP, weapons). In-run state is reset every run anyway, so quitting the game is the same as abandoning the current run.

### 1.2 When It Is Read and Written

```mermaid
sequenceDiagram
  participant G as GameInstance startup
  participant V as UCampSaveSubsystem
  participant S as CampStock and CampMeta
  G->>V: Initialize
  V->>S: InitializeDependency ensures they are created first
  V->>V: DoesSaveGameExist then LoadGameFromSlot
  V->>S: bApplyingLoad = true then RestoreState RestoreLevels
  V->>S: only afterwards bind OnStockChanged OnMetaChanged
  Note over V,S: Load first and bind later so restoring does not trigger a save
  S-->>V: stock or building changed
  V->>V: HandleCampChanged then SaveNow if not restoring
  Note over V: URunOutcomeSubsystem::EndRun also calls SaveNow explicitly
```

- Events that trigger a save: Iron changes, warehouse changes (`OnStockChanged`), building level changes (`OnMetaChanged`). An upgrade fires twice in a row: `SpendIron` broadcasts `OnStockChanged` first (Iron has been deducted but the level has not yet gone up), then the level is incremented and `OnMetaChanged` is broadcast. So one upgrade writes the file twice. The first write captures the intermediate state "paid but not upgraded", and only the second write is the complete state. If the process is killed between the two writes, the save will have less Iron but no upgrade (see [known-issues.md](known-issues.md)).
- When a run ends, `EndRun` writes once more explicitly, because a death with no Iron in the bag leaves the stock unchanged and no event fires.
- If the save is corrupt or cannot be read: only a warning is logged and the game starts from a fresh camp. The bad file is not deleted, and the next change overwrites it.
- `DeleteSaveAndReset()`: deletes the slot and zeroes the stock and buildings. **Currently no UI or key calls it** (the "New Game" button is Should-tier and out of the current scope).

### 1.3 Version Compatibility

The most recent release tag is `jam-start` (commit `44589d2`, 2026-11-12). It predates the UE project itself (that commit contains no `Game_01` files), so there is no "released version" to compare save formats against. The repository currently has only one save format, `SaveVersion = 1`, and no migration code.

| Data | Released version | Current code | Consequence |
|---|---|---|---|
| Save format | No release | `SaveVersion = 1` | When the field layout changes later, a version check has to be added by hand. Loading currently ignores this field |

## 2. Tuning Data Tables

All value tables live in `Content/Data/` (LFS) and are referenced by the project setting "Director Tuning". The reference paths are recorded under `[/Script/Game_01.DirectorTuningSettings]` in [DefaultGame.ini](../../Game_01%205.8/Config/DefaultGame.ini). If a table or row is missing, the member initial values in C++ work as usual and no error is raised.

| Setting | Table | Row names | Row struct | Used by |
|---|---|---|---|---|
| `EnemyTuningTable` | `DT_EnemyTuning` | `Grunt`, `Boss` | `FEnemyTuning` | `UEnemySpawnSubsystem` |
| `SpawnTuningTable` | `DT_SpawnTuning` | `Spawn` | `FSpawnTuning` | `UEnemySpawnSubsystem` |
| `SwarmTuningTable` | `DT_SwarmTuning` | `Swarm` | `FSwarmTuning` | `UEnemyManagerSubsystem` |
| `HealthTuningTable` | `DT_HealthTuning` | `Hero` | `FHealthTuning` | `UHealthComponent` |
| `ProgressionTable` | `DT_Progression` | `Progression` | `FXpCurveRow` | `URunProgressionSubsystem` |
| `WeaponTable` | `DT_Weapons` | `Blade`, `Bolt`, `Nova` | `FWeaponRow` | `URunProgressionSubsystem` |
| `PassiveTable` | `DT_Passives` | `Swiftness`, `Might`, `Vitality`, `Greed` | `FPassiveRow` | `URunProgressionSubsystem` |
| `FilmCardTable` | `DT_FilmCards` | `HordeNight`, `CutKnight` | `FFilmCardRow` | `UFilmCardSubsystem` (the row key is the card id, and all rows are optional) |
| `BuildingTable` | `DT_Buildings` | `Smithy`, `Clinic` | `FBuildingRow` | `UCampMetaSubsystem` |

Notes:

- The `Kind`, `BuildingType`, `WeaponType` and `PassiveType` fields in the tables are overwritten by the row name (the row name is the identity).
- Heat parameters (bag capacity, Iron per slot, heat gain and loss, spawn pressure) and relic and container values are **not** in these 9 tables. They come from the `ULootConfig` data asset, and when there is no asset the inline defaults in the constructor of [LootConfig.cpp](../../Game_01%205.8/Source/Game_01/Private/Loot/LootConfig.cpp) are used. The current project has no `ULootConfig` asset, so the inline defaults are what actually runs.
- The Iron gained from dismantling a relic (8) is hardcoded in [`UCampMetaSubsystem`](../../Game_01%205.8/Source/Game_01/Public/Camp/CampMetaSubsystem.h) and is not in any table.
- These `.uasset` files are binary. This wiki only checked row names and fields (taken from the code) and did not read out the specific values in each table. The values in the editor's tables are authoritative.

## 3. Config Files

| File | Project-relevant content |
|---|---|
| [DefaultEngine.ini](../../Game_01%205.8/Config/DefaultEngine.ini) | `GameDefaultMap` and `EditorStartupMap` are both `/Game/Maps/L_Camp.L_Camp`. Rendering: Lumen (dynamic GI method 1), reflection method 1, Substrate on, ray tracing on, DX12 SM6 |
| [DefaultGame.ini](../../Game_01%205.8/Config/DefaultGame.ini) | The 9 table references of `Director Tuning` |
| `DefaultEditorPerProjectUserSettings.ini` | Listed in .gitignore, stores each developer's local editor settings (such as the MCP port) |

`EditorStartupMap` makes the editor open the camp directly every time. When it is unset, the engine default is `/Engine/Maps/Templates/OpenWorld`, which opens an unnamed template level every time.

## 4. Cross-Module Units and Conventions

| Item | Convention |
|---|---|
| Length | Centimeters. `ContactRange`, `RingMinRadius` and weapon `Range` are all in centimeters |
| Planar math | Chasing, pushing apart and weapon hits are all done in the XY plane, ignoring Z |
| Time | Game logic uses `World->GetTimeSeconds()`. `UHudMessageSubsystem` uses real time |
| Iron and slots | In the bag, every `IronPerSlot` (10) Iron takes 1 slot, rounded up |
| Indexing | Relic indices are 0-based in both the bag and the warehouse, and the UI shows them starting from `[1]` |
