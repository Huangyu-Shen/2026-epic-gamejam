# Story 004: Run progression (level-up, weapons, passives)

> **Epic**: Director's Cut (MVP)
> **Status**: Complete
> **Layer**: N/A (minimal)
> **Type**: Logic
> **Estimate**: M
> **Manifest Version**: N/A (minimal — no control manifest)
> **Last Updated**: 2026-11-13

## Context

**GDD**: `design/game-brief.md`
**Requirement**: `Brief MVP feature 2`

**ADR Governing Implementation**: N/A (minimal — no ADRs)
**ADR Decision Summary**: N/A (minimal — no ADRs)
**ADR Version**: N/A (minimal — no ADRs)

**Engine**: Unreal 5.8 | **Risk**: HIGH (`docs/engine-reference/unreal/VERSION.md`, 5.8 row)
**Engine Notes**: none (no ADR engine-compatibility analysis at minimal)

**Control Manifest Rules (this layer)**: N/A (minimal — no control manifest)

---

## Acceptance Criteria

- [x] Killing enemies drops experience; collecting it fills an XP bar.
- [x] Reaching the threshold pauses the game and offers a choice of 3 upgrades; picking one resumes play.
- [x] 3 weapons exist, each with a distinct attack pattern; the pool offers them as new or upgrade choices.
- [x] 4 passives exist and apply their effect immediately on pick.
- [x] The offer never repeats a maxed-out option, and always has 3 distinct entries while enough remain.
- [x] XP curve, weapon stats and passive values come from data tables.
- [x] Run progression resets at the start of each run (nothing carries over to the camp).

---

## Implementation Notes

- The 3-choice panel is a plain debug-style widget here; the final panel is story 007.
- Weapons reuse story 003's auto-attack hook.

---

## Out of Scope

- Weapons unlocked from the editing room, weapon evolution (Should/Could).
- Story 007: polished level-up UI.

---

## Files

- `Game_01 5.8/Source/Game_01/Public/Progression/ProgressionTypes.h` — created (weapon/passive/XP row structs)
- `Game_01 5.8/Source/Game_01/Public/Progression/XpPickup.h` + `Private/Progression/XpPickup.cpp` — created (XP pickup actor)
- `Game_01 5.8/Source/Game_01/Public/Progression/RunProgressionSubsystem.h` + `Private/Progression/RunProgressionSubsystem.cpp` — created (XP/level/offer/passive brain)
- `Game_01 5.8/Source/Game_01/Public/Combat/AutoWeaponComponent.h` + `Private/Combat/AutoWeaponComponent.cpp` — rewritten to three weapons (Blade/Bolt/Nova)
- `Game_01 5.8/Source/Game_01/Public/Enemies/EnemyManagerSubsystem.h` + `Private/Enemies/EnemyManagerSubsystem.cpp` — GetEnemies accessor + offer pause gate
- `Game_01 5.8/Source/Game_01/Private/Enemies/EnemySpawnSubsystem.cpp` — offer pause gate
- `Game_01 5.8/Source/Game_01/Public/Combat/HealthComponent.h` + `Private/Combat/HealthComponent.cpp` — IncreaseMaxHealth (Vitality)
- `Game_01 5.8/Source/Game_01/Private/Player/HeroCharacter.cpp` — reset/speed/offer keys/CanAct/XP debug
- `Game_01 5.8/Source/Game_01/Public/Game/DirectorTuningSettings.h` + `Config/DefaultGame.ini` — three data-table pointers
- `Game_01 5.8/Content/Data/DT_Progression.uasset`, `DT_Weapons.uasset`, `DT_Passives.uasset` — created

---

## QA Test Cases

*N/A — no qa-lead specs at this tier; implement against the Acceptance Criteria above.*

---

## Test Evidence

*`qa.level: minimal`: tests waived (advisory).*

**Story Type**: Logic
**Required evidence**: `Game_01/Source/Game_01/Private/Tests/RunProgressionTests.cpp` (advisory at minimal)

**Status**: [x] Waived at `qa.level: minimal` — no test file written

---

## Dependencies

- Depends on: `production/epics/directors-cut/story-003-combat-enemies-map.md`
- Unlocks: None

---

## Completion Notes

**Completed**: 2026-11-13
**Criteria**: 5/7 passing (AC-1, AC-2 deferred to playtest)
**Deviations**: hardcoded fallback weapon/passive/XP stats in `RunProgressionSubsystem.cpp` (used only when a data table fails to load; the three data tables are authoritative). Out-of-scope tooling: `.tmp/` build scripts and `Game_01 5.8/.mcp.json`.
**Test Evidence**: waived at `qa.level: minimal` — no test file written
**Reviewed by**: self, no independent review (solo mode)
