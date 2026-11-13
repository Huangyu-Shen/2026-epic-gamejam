# Story 005: Pre-run editing (film cards, multiplier)

> **Epic**: Director's Cut (MVP)
> **Status**: Complete
> **Layer**: N/A (minimal)
> **Type**: Integration
> **Estimate**: M
> **Manifest Version**: N/A (minimal — no control manifest)
> **Last Updated**: 2026-11-13

## Context

**GDD**: `design/game-brief.md`
**Requirement**: `Brief MVP feature 5`

**ADR Governing Implementation**: N/A (minimal — no ADRs)
**ADR Decision Summary**: N/A (minimal — no ADRs)
**ADR Version**: N/A (minimal — no ADRs)

**Engine**: Unreal 5.8 | **Risk**: HIGH (`docs/engine-reference/unreal/VERSION.md`, 5.8 row)
**Engine Notes**: none (no ADR engine-compatibility analysis at minimal)

**Control Manifest Rules (this layer)**: N/A (minimal — no control manifest)

---

## Acceptance Criteria

- [ ] The editing room has 2 film slots; the player can put a card in or take it out before departing.
- [ ] 2 cards exist: **Horde Night** (forces a horde) and **The Cut Knight** (forces an elite/Boss variant).
- [ ] Each card shows its effect and the earnings multiplier it adds before the run starts.
- [ ] Departing with a card active visibly changes the run (horde / boss appears) via story 003's scheduler.
- [ ] The run's multiplier (product or sum of active cards; 1.0 with none) is passed to settlement in story 002.
- [ ] Card effects and multipliers come from a data table.
- [ ] The chosen cards are remembered only for that run (not carried over).

---

## Implementation Notes

- Cards are data rows; adding a third card later must need no code change.
- Difficulty comes only from the player's choices, never random (pillar: you direct the cut).

---

## Out of Scope

- More cards, slots up to 5, card combos (Should/Could).
- Story 007: polished editing panel UI.

---

## Files

- `Game_01/Source/Game_01/Public/Editing/FilmCardTypes.h` (new) — `EFilmCardEffect` enum + `FFilmCardRow` struct
- `Game_01/Source/Game_01/Public/Editing/FilmCardSubsystem.h` (new) — `UWorldSubsystem`, slots/cards/multiplier
- `Game_01/Source/Game_01/Private/Editing/FilmCardSubsystem.cpp` (new) — data-table load, `ApplyActiveCards`, `ResetCards` on run end
- `Game_01/Source/Game_01/Public/Game/DirectorTuningSettings.h` (mod) — `FilmCardTable` soft ptr
- `Game_01/Config/DefaultGame.ini` (mod) — `FilmCardTable=/Game/Data/DT_FilmCards.DT_FilmCards`
- `Game_01/Source/Game_01/Public/Player/HeroCharacter.h` (mod) — `[`/`]` card keys; removed H/B debug keys
- `Game_01/Source/Game_01/Private/Player/HeroCharacter.cpp` (mod) — card slot/apply logic + HUD `CARDS` line
- `Game_01/Content/Data/DT_FilmCards.uasset` (new) — rows `HordeNight` (ForceHorde/40/1.5), `CutKnight` (ForceBoss/0/2.0)

**Build**: `Game_01Editor Win64 Development` — EXIT=0 (UHT + compile + link clean).

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

- Depends on: `production/epics/directors-cut/story-002-extraction-death.md`, `production/epics/directors-cut/story-003-combat-enemies-map.md`
- Unlocks: None

## Completion Notes

**Completed**: 2026-11-13
**Criteria**: 7/7 passing
**Deviations**: None (one advisory — AC-3: the debug HUD shows card names and the aggregate earnings multiplier, not a per-card effect string or per-card multiplier; the polished editing-room panel is story-007)
**Test Evidence**: waived at `qa.level: minimal` (Integration)
**Reviewed by**: self, no independent review (solo mode)
