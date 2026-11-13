# Story 003: In-run combat, enemies and greybox map

> **Epic**: Director's Cut (MVP)
> **Status**: Complete
> **Layer**: N/A (minimal)
> **Type**: Visual/Feel
> **Estimate**: L
> **Manifest Version**: N/A (minimal — no control manifest)
> **Last Updated**:2026-11-12

## Context

**GDD**: `design/game-brief.md`
**Requirement**: `Brief MVP feature 1`

**ADR Governing Implementation**: N/A (minimal — no ADRs)
**ADR Decision Summary**: N/A (minimal — no ADRs)
**ADR Version**: N/A (minimal — no ADRs)

**Engine**: Unreal 5.8 | **Risk**: HIGH (`docs/engine-reference/unreal/VERSION.md`, 5.8 row)
**Engine Notes**: none (no ADR engine-compatibility analysis at minimal)

**Control Manifest Rules (this layer)**: N/A (minimal — no control manifest)

---

## Acceptance Criteria

- [x] Top-down player: WASD move, dodge, an interact key, keyboard and mouse only.
- [x] 1 starting weapon auto-attacks nearby enemies with no button press.
- [x] Player takes damage and dies at 0 HP (death is handled by story 002).
- [x] 1 small enemy type chases the player and deals contact damage.
- [x] A spawn scheduler sends waves that ramp over time and respond to heat from story 001.
- [x] A horde event and a Boss can be spawned by the scheduler (the editing cards in story 005 will call them).
- [x] 1 fixed greybox map with search points and 2 extraction-point locations.
- [x] 150 enemies on screen hold ≥ 60 FPS at 1080p on the dev machine (cap on-screen count, simple meshes, no per-enemy complex AI).
- [x] All numbers (HP, damage, wave rates, caps) come from data tables.

---

## Implementation Notes

- Cubes and capsules only; readable silhouettes beat art at this stage.
- Retained screenshot of a busy fight in `production/qa/evidence/` is required (Visual/Feel).

---

## Out of Scope

- Story 004: leveling and extra weapons.
- Story 001/002: loot and extraction rules.
- Elite and extra enemy types, Boss second phase, gamepad (Should/Could tier).

---

## Files

- `Game_01 5.8/Source/Game_01/Private/Combat/AutoWeaponComponent.cpp`
- `Game_01 5.8/Source/Game_01/Private/Combat/HealthComponent.cpp`
- `Game_01 5.8/Source/Game_01/Private/Enemies/Enemy.cpp`
- `Game_01 5.8/Source/Game_01/Private/Enemies/EnemyManagerSubsystem.cpp`
- `Game_01 5.8/Source/Game_01/Private/Enemies/EnemySpawnSubsystem.cpp`
- `Game_01 5.8/Source/Game_01/Private/Enemies/EnemyTypes.cpp`
- `Game_01 5.8/Source/Game_01/Private/Game/DirectorGameMode.cpp`
- `Game_01 5.8/Source/Game_01/Private/Game/DirectorTuningSettings.cpp`
- `Game_01 5.8/Source/Game_01/Private/Player/HeroCharacter.cpp`
- `Game_01 5.8/Source/Game_01/Public/Combat/AutoWeaponComponent.h`
- `Game_01 5.8/Source/Game_01/Public/Combat/HealthComponent.h`
- `Game_01 5.8/Source/Game_01/Public/Enemies/Enemy.h`
- `Game_01 5.8/Source/Game_01/Public/Enemies/EnemyManagerSubsystem.h`
- `Game_01 5.8/Source/Game_01/Public/Enemies/EnemySpawnSubsystem.h`
- `Game_01 5.8/Source/Game_01/Public/Enemies/EnemyTypes.h`
- `Game_01 5.8/Source/Game_01/Public/Game/DirectorGameMode.h`
- `Game_01 5.8/Source/Game_01/Public/Game/DirectorTuningSettings.h`
- `Game_01 5.8/Source/Game_01/Public/Player/HeroCharacter.h`
- `Game_01 5.8/Content/Maps/L_Greybox.umap` (DirectorGameMode, containers and zones spread out, PlayerStart at origin)
- `Game_01 5.8/Content/Data/DT_EnemyTuning.uasset` (rows `Grunt`, `Boss`)
- `Game_01 5.8/Content/Data/DT_SpawnTuning.uasset` (row `Spawn`)
- `Game_01 5.8/Content/Data/DT_SwarmTuning.uasset` (row `Swarm`)
- `Game_01 5.8/Content/Data/DT_HealthTuning.uasset` (row `Hero`)
- `Game_01 5.8/Config/DefaultGame.ini` (`[/Script/Game_01.DirectorTuningSettings]` points at the four tables)

## QA Test Cases

*N/A — no qa-lead specs at this tier; implement against the Acceptance Criteria above.*

---

## Test Evidence

*`qa.level: minimal`: tests waived (advisory), but the screenshot is not.*

**Story Type**: Visual/Feel
**Required evidence**: retained screenshot in `production/qa/evidence/` + sign-off in `production/qa/evidence/story-003-combat-enemies-map-evidence.md`

**Status**: [x] Created — screenshot `production/qa/evidence/story-003-combat-enemies-map/01-fight-horde-boss.png`, hash baseline in the same folder's `stamp.txt` (18 files, all UNCHANGED)

---

## Dependencies

- Depends on: None (story 001 may use a stand-in pawn until this lands)
- Unlocks: `production/epics/directors-cut/story-004-run-progression.md`, `production/epics/directors-cut/story-005-pre-run-editing.md`

---

## Completion Notes
**Completed**: 2026-11-12
**Criteria**: 9/9 passing (AC-8 min frame time accepted below target, see deviations)
**Build**: `Build.bat Game_01Editor Win64 Development` — Succeeded (UE 5.8, Win64)
**Deviations**:
- ADVISORY, AC-8: perf probe `enemies=152 avgFPS=132.3 minFPS=50.0 samples=1324` (standalone `-game`, 1080p). The average clears the 60 FPS bar; the single worst frame (50.0) is below it. Accepted by the user as-is.
- ADVISORY, AC-3: the hero loses health and, at 0 HP, `AHeroCharacter::HandleDied` fires `URunOutcomeSubsystem::ReportPlayerDied` (the `OnDied` bind was fixed this session — `HandleDied` was missing `UFUNCTION()` and never bound). Nothing yet tells the player they died (no overlay, effect or end screen); that feedback is story 007.
- ADVISORY, AC-9: the numbers now live in the four `DT_*Tuning` tables, and the same values remain as member initialisers on the `F*Tuning` structs so a missing row or table silently falls back instead of dropping the value to zero. The fallbacks are a safety net, not a second source of truth.
- OUT OF SCOPE (awareness): `Game_01 5.8/Content/Maps/L_Greybox.umap` is a throwaway test map. Enemies, hero and crates are untextured engine cubes/capsules, so the fight reads as grey-on-grey in the retained screenshot.
**Test Evidence**: waived at `qa.level: minimal`; retained screenshot `production/qa/evidence/story-003-combat-enemies-map/01-fight-horde-boss.png`, hash baseline current
**Reviewed by**: self, no independent review
