# Story 004: Run progression (level-up, weapons, passives)

> **Epic**: Director's Cut (MVP)
> **Status**: Ready
> **Layer**: N/A (minimal)
> **Type**: Logic
> **Estimate**: M
> **Manifest Version**: N/A (minimal — no control manifest)
> **Last Updated**:

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

- [ ] Killing enemies drops experience; collecting it fills an XP bar.
- [ ] Reaching the threshold pauses the game and offers a choice of 3 upgrades; picking one resumes play.
- [ ] 3 weapons exist, each with a distinct attack pattern; the pool offers them as new or upgrade choices.
- [ ] 4 passives exist and apply their effect immediately on pick.
- [ ] The offer never repeats a maxed-out option, and always has 3 distinct entries while enough remain.
- [ ] XP curve, weapon stats and passive values come from data tables.
- [ ] Run progression resets at the start of each run (nothing carries over to the camp).

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

- `Game_01/Source/Game_01/Public/Progression/*.h`, `Private/Progression/*.cpp`
- `Game_01/Content/Data/` XP, weapon and passive tables
- `Game_01/Source/Game_01/Private/Tests/RunProgressionTests.cpp`

---

## QA Test Cases

*N/A — no qa-lead specs at this tier; implement against the Acceptance Criteria above.*

---

## Test Evidence

*`qa.level: minimal`: tests waived (advisory).*

**Story Type**: Logic
**Required evidence**: `Game_01/Source/Game_01/Private/Tests/RunProgressionTests.cpp` (advisory at minimal)

**Status**: [ ] Not yet created

---

## Dependencies

- Depends on: `production/epics/directors-cut/story-003-combat-enemies-map.md`
- Unlocks: None
