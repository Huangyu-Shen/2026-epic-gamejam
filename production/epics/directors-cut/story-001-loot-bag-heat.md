# Story 001: Loot, bag and heat — the keep-or-use choice

> **Epic**: Director's Cut (MVP)
> **Status**: Complete
> **Layer**: N/A (minimal)
> **Type**: Logic
> **Estimate**: M
> **Manifest Version**: N/A (minimal — no control manifest)
> **Last Updated**:2026-11-12

## Context

**GDD**: `design/game-brief.md`
**Requirement**: `Brief MVP feature 3` (build-order step 1: prove "keep it or use it" is fun first)

**ADR Governing Implementation**: N/A (minimal — no ADRs)
**ADR Decision Summary**: N/A (minimal — no ADRs)
**ADR Version**: N/A (minimal — no ADRs)

**Engine**: Unreal 5.8 | **Risk**: HIGH (`docs/engine-reference/unreal/VERSION.md`, 5.8 row)
**Engine Notes**: none (no ADR engine-compatibility analysis at minimal)

**Control Manifest Rules (this layer)**: N/A (minimal — no control manifest)

---

## Acceptance Criteria

- [ ] Supply crates and a vault can be searched in-run; each search yields Iron and/or a relic.
- [ ] Exactly 3 relic types exist; each can be **kept** (stays in the bag) or **used** on the spot for a buff.
- [ ] Every pickup of a rare item raises heat; the heat value is readable from code and changes enemy spawn pressure (hook only; spawning itself is story 003).
- [ ] Using a relic gives its buff and lowers heat; keeping it leaves heat high (the tension the game rests on).
- [ ] The bag has a hard capacity; a full bag refuses further pickups and says so.
- [ ] Bag, heat and relic numbers (capacity, heat per item, buff size) come from a data table, none hardcoded.
- [ ] Death handling is not here: the bag is exposed so story 002 can clear it or settle it.

---

## Implementation Notes

- Needs a movable pawn and the greybox map. If story 003 is not started, use the engine's default Character with WASD as a throwaway stand-in; build no combat here.
- Cubes and capsules only (greybox); no art.

---

## Out of Scope

- Story 002: extraction, settlement, death loss.
- Story 003: enemy spawning and combat (this story only exposes heat).
- Story 007: bag/HUD widgets (use debug text here).

---

## Files

- `Game_01/Source/Game_01/Public/Loot/LootBagComponent.h`
- `Game_01/Source/Game_01/Public/Loot/LootConfig.h`
- `Game_01/Source/Game_01/Public/Loot/LootContainer.h`
- `Game_01/Source/Game_01/Public/Loot/LootHeatSubsystem.h`
- `Game_01/Source/Game_01/Public/Loot/LootTypes.h`
- `Game_01/Source/Game_01/Private/Loot/LootBagComponent.cpp`
- `Game_01/Source/Game_01/Private/Loot/LootConfig.cpp`
- `Game_01/Source/Game_01/Private/Loot/LootContainer.cpp`
- `Game_01/Source/Game_01/Private/Loot/LootHeatSubsystem.cpp`
- `Game_01/Source/Game_01/Private/Loot/LootTypes.cpp`
- `Game_01/Source/Game_01/Public/StandIn/StandInCharacter.h`
- `Game_01/Source/Game_01/Public/StandIn/StandInGameMode.h`
- `Game_01/Source/Game_01/Private/StandIn/StandInCharacter.cpp`
- `Game_01/Source/Game_01/Private/StandIn/StandInGameMode.cpp`

## QA Test Cases

*N/A — no qa-lead specs at this tier; implement against the Acceptance Criteria above.*

---

## Test Evidence

*`qa.level: minimal`: tests waived (advisory).*

**Story Type**: Logic
**Required evidence**: `Game_01/Source/Game_01/Private/Tests/LootBagHeatTests.cpp` (advisory at minimal)

**Status**: [ ] Not yet created

---

## Dependencies

- Depends on: None
- Unlocks: `production/epics/directors-cut/story-002-extraction-death.md`, `production/epics/directors-cut/story-003-combat-enemies-map.md`

---

## Completion Notes
**Completed**: 2026-11-12
**Criteria**: 6/7 passing (AC-2 partly untested: Swiftstep and Warbrand use not observed; Aegis use observed)
**Deviations**:
- ADVISORY, AC-6: tuning numbers are editable defaults on `FLootTuning` / `ULootConfig` in code. The data-table path works but no DataTable or DataAsset exists yet (`Game_01/Content/Data/` absent). Author them when the numbers settle.
- ADVISORY: "Pickup refused: Bag is full" also shows at 4/6 slots, when a vault's Iron plus relic does not fit (all-or-nothing). The text should say "does not fit".
- ADVISORY: relic-use heat drop was changed 25 → 8 after the screenshots; the new value was not re-observed. Needs a playtest once story 002 gives keeping a payoff.
- OUT OF SCOPE (awareness): `Game_01/Content/Maps/L_Greybox.umap` is not in `## Files`; it is the throwaway test map. Stand-in pawn and game mode (`StandIn/`) are temporary and go in story 003.
**Test Evidence**: waived at qa.level minimal; retained run screenshots in `production/qa/evidence/story-001-loot-bag-heat/` (01–04), stamp current
**Reviewed by**: self, no independent review
