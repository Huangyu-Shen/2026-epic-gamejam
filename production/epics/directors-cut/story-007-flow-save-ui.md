# Story 007: Game flow, save and must-have UI

> **Epic**: Director's Cut (MVP)
> **Status**: Complete
> **Layer**: N/A (minimal)
> **Type**: UI
> **Estimate**: L
> **Manifest Version**: N/A (minimal — no control manifest)
> **Last Updated**:2026-11-13

## Context

**GDD**: `design/game-brief.md`
**Requirement**: `Brief MVP feature 7`

**ADR Governing Implementation**: N/A (minimal — no ADRs)
**ADR Decision Summary**: N/A (minimal — no ADRs)
**ADR Version**: N/A (minimal — no ADRs)

**Engine**: Unreal 5.8 | **Risk**: HIGH (`docs/engine-reference/unreal/VERSION.md`, 5.8 row)
**Engine Notes**: none (no ADR engine-compatibility analysis at minimal)

**Control Manifest Rules (this layer)**: N/A (minimal — no control manifest)

---

## Acceptance Criteria

- [x] The flow runs camp → editing room → run → result → camp with no dead ends.
- [x] A new save starts at the camp; one save slot auto-saves after each run and after each upgrade.
- [x] Reloading the game restores camp stock, building levels and stored relics.
- [x] In-run HUD shows HP, XP bar, heat, bag usage and the extraction bar.
- [x] Level-up 3-choice panel, bag view, search/extraction progress bar and result screen exist.
- [x] Camp panel and editing-room panel (from stories 006 and 005) are usable with mouse.
- [x] A fresh player can complete two full runs from the camp without developer help.
- [x] A retained screenshot of every screen touched is saved in `production/qa/evidence/`.
- [x] All in-game text is English.

---

## Implementation Notes

- Use UMG/CommonUI; keep one readable style so the Should-tier art pass can restyle it later.
- Replace the debug text from stories 001, 002, 004 and 005 with these widgets.

---

## Out of Scope

- Title screen, pause menu, settings, tutorial hints, new-game reset (Should tier).
- UI animation polish (Could tier).

---

## Files

- `Game_01 5.8/Source/Game_01/Public/UI/*.h` and `Private/UI/*.cpp` (`DirectorUi`, `RunHudWidget`, `CampWidget`); built in C++, no widget blueprints
- Edited: `HeroCharacter` (StartRun, HUD), `CampPawn` (camp screen), `RunOutcomeSubsystem` (save after each run), `FilmCardSubsystem` and `CampMetaSubsystem` (cards survive the level change), `RunProgressionSubsystem` (offer text), `Game_01.Build.cs` (UMG, Slate), `Config/DefaultEngine.ini` (start maps)
- Story 007 flow save files from the first pass: `Public/Flow/CampSaveGame.h`, `CampSaveSubsystem.h` and matching `Private/Flow/`

## QA Test Cases

*N/A — no qa-lead specs at this tier; implement against the Acceptance Criteria above.*

---

## Test Evidence

*`qa.level: minimal`: tests waived (advisory), but the screenshots are not.*

**Story Type**: UI
**Required evidence**: a retained screenshot of each screen touched, in `production/qa/evidence/`

**Status**: [x] Created: `production/qa/evidence/story-007-flow-save-ui/` (camp x3, run HUD, level-up panel, extraction bar with bag, result screen)

---

## Dependencies

- Depends on: `production/epics/directors-cut/story-001-loot-bag-heat.md`, `production/epics/directors-cut/story-002-extraction-death.md`, `production/epics/directors-cut/story-004-run-progression.md`, `production/epics/directors-cut/story-005-pre-run-editing.md`, `production/epics/directors-cut/story-006-camp-meta.md`
- Unlocks: None

---

## Completion Notes
**Completed**: 2026-11-13
**Criteria**: 9/9 passing (full loop, save restore and two runs confirmed by the user's own playthrough; every screen has a retained screenshot)
**Deviations**: ADVISORY: no separate search progress bar, because searching a crate is instant (the extraction bar exists). FIXED after the first close: the old debug text ("EXTRACTING 31%", "Searched: ...") that overlapped the HP bar now goes through `UHudMessageSubsystem` and shows in a top-centre message area (`09-`, `10-`); the camp panel now fills the free height and START RUN is pinned at the bottom (`08-`). Final screens: `08-` to `11-`. OUT OF SCOPE: `DefaultEngine.ini` start-map settings.
**Test Evidence**: waived at qa.level minimal; screenshots in `production/qa/evidence/story-007-flow-save-ui/`
**Reviewed by**: self, no independent review
