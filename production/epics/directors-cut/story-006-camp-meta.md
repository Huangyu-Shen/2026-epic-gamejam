# Story 006: Camp and meta growth

> **Epic**: Director's Cut (MVP)
> **Status**: Ready
> **Layer**: N/A (minimal)
> **Type**: Integration
> **Estimate**: L
> **Manifest Version**: N/A (minimal — no control manifest)
> **Last Updated**:

## Context

**GDD**: `design/game-brief.md`
**Requirement**: `Brief MVP feature 6`

**ADR Governing Implementation**: N/A (minimal — no ADRs)
**ADR Decision Summary**: N/A (minimal — no ADRs)
**ADR Version**: N/A (minimal — no ADRs)

**Engine**: Unreal 5.8 | **Risk**: HIGH (`docs/engine-reference/unreal/VERSION.md`, 5.8 row)
**Engine Notes**: none (no ADR engine-compatibility analysis at minimal)

**Control Manifest Rules (this layer)**: N/A (minimal — no control manifest)

---

## Acceptance Criteria

- [ ] A 3D camp scene where the player clicks a building to select it (point-and-click, not walkable).
- [ ] 2 buildings: **Smithy** and **Clinic**; each upgrade raises a run stat (e.g. weapon strength, survivability).
- [ ] Upgrade cost grows exponentially per level and is paid in Iron from the camp stock.
- [ ] An upgrade with too little Iron is refused with a clear message.
- [ ] Upgrade levels apply at the start of the next run.
- [ ] A warehouse panel lists stored relics and lets the player **dismantle** one into Iron.
- [ ] After two successful extractions with only the starting cards, at least 2 building upgrades have been possible.
- [ ] Costs, growth rate and per-level effects come from data tables.

---

## Implementation Notes

- Building visuals are greybox; visible growth means a readable level number/size change at minimum.
- Persistence of camp state is story 007.

---

## Out of Scope

- More buildings, building appearance stages, editing-room unlocks, extra materials (Should/Could).
- Story 007: save/load and the final camp panels.

---

## Files

- `Game_01/Source/Game_01/Public/Camp/*.h`, `Private/Camp/*.cpp`
- `Game_01/Content/Data/` building table; `Game_01/Content/Maps/` camp scene

---

## QA Test Cases

*N/A — no qa-lead specs at this tier; implement against the Acceptance Criteria above.*

---

## Test Evidence

*`qa.level: minimal`: tests waived (advisory).*

**Story Type**: Integration
**Required evidence**: integration test OR a documented playtest (advisory at minimal)

**Status**: [ ] Not yet created

---

## Dependencies

- Depends on: `production/epics/directors-cut/story-002-extraction-death.md`
- Unlocks: None
