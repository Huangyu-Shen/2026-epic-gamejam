# Story 006: Camp and meta growth

> **Epic**: Director's Cut (MVP)
> **Status**: Complete
> **Layer**: N/A (minimal)
> **Type**: Integration
> **Estimate**: L
> **Manifest Version**: N/A (minimal — no control manifest)
> **Last Updated**: 2026-11-13

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

- [x] A 3D camp scene where the player clicks a building to select it (point-and-click, not walkable).
- [x] 2 buildings: **Smithy** and **Clinic**; each upgrade raises a run stat (e.g. weapon strength, survivability).
- [x] Upgrade cost grows exponentially per level and is paid in Iron from the camp stock.
- [x] An upgrade with too little Iron is refused with a clear message.
- [x] Upgrade levels apply at the start of the next run.
- [x] A warehouse panel lists stored relics and lets the player **dismantle** one into Iron.
- [x] After two successful extractions with only the starting cards, at least 2 building upgrades have been possible.
- [x] Costs, growth rate and per-level effects come from data tables.

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
  (`CampTypes`, `CampMetaSubsystem`, `CampBuilding`, `CampWarehouse`, `CampPawn`, `CampGameMode`)
- `Game_01/Content/Data/DT_Buildings` building table; `Game_01/Content/Maps/L_Camp` camp scene
- Edited: `CampStockSubsystem` (SpendIron, RemoveRelicAt), `HealthComponent` (BaseMaxHealth/SetMaxHealth),
  `AutoWeaponComponent` (camp damage bonus), `HeroCharacter` (`B` to camp, `N` new run applies bonuses),
  `DirectorTuningSettings` (BuildingTable), `Config/DefaultGame.ini`, `Config/DefaultEngine.ini` (Zen fix)

## Implementation Status (2026-11-13)

- Verified: C++ compiles (Live Coding), L_Camp PIE spawns Smithy/Clinic/Warehouse in distinct colours
  (screenshot `production/qa/evidence/story-006-camp-colors.png`).
- Fixed this session: Cube tint was a no-op because `DefaultMaterial` has no `Color` parameter
  (now `BasicShapeMaterial`); Clinic was spawned as Smithy because `BuildingType` was set after
  `BeginPlay` (now `SpawnActorDeferred`).
- NOT yet verified in play: click-to-select, upgrade with/without Iron (refusal message), bonuses
  applying to the next run, relic dismantle (+8 Iron), the "two extractions -> 2 upgrades possible" criterion.
  Acceptance Criteria stay unchecked until those are played.

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

---

## Completion Notes
**Completed**: 2026-11-13
**Criteria**: 8/8 passing (behaviour criteria confirmed by the user's own playtest; only the building colours have a retained screenshot)
**Deviations**: None blocking. ADVISORY: no retained screenshot or playtest record for click/upgrade/dismantle. OUT OF SCOPE for awareness: uncommitted changes in Enemies/, Flow/, Progression/, Editing/ and stories 004/005 belong to other stories.
**Test Evidence**: waived at qa.level minimal (`production/qa/evidence/story-006-camp-colors.png` for the visuals)
**Reviewed by**: self, no independent review
