# Story 007: Game flow, save and must-have UI

> **Epic**: Director's Cut (MVP)
> **Status**: Ready
> **Layer**: N/A (minimal)
> **Type**: UI
> **Estimate**: L
> **Manifest Version**: N/A (minimal — no control manifest)
> **Last Updated**:

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

- [ ] The flow runs camp → editing room → run → result → camp with no dead ends.
- [ ] A new save starts at the camp; one save slot auto-saves after each run and after each upgrade.
- [ ] Reloading the game restores camp stock, building levels and stored relics.
- [ ] In-run HUD shows HP, XP bar, heat, bag usage and the extraction bar.
- [ ] Level-up 3-choice panel, bag view, search/extraction progress bar and result screen exist.
- [ ] Camp panel and editing-room panel (from stories 006 and 005) are usable with mouse.
- [ ] A fresh player can complete two full runs from the camp without developer help.
- [ ] A retained screenshot of every screen touched is saved in `production/qa/evidence/`.
- [ ] All in-game text is English.

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

- `Game_01/Source/Game_01/Public/Flow/*.h`, `Public/UI/*.h` and matching `Private/`
- `Game_01/Content/UI/` widget blueprints

---

## QA Test Cases

*N/A — no qa-lead specs at this tier; implement against the Acceptance Criteria above.*

---

## Test Evidence

*`qa.level: minimal`: tests waived (advisory), but the screenshots are not.*

**Story Type**: UI
**Required evidence**: a retained screenshot of each screen touched, in `production/qa/evidence/`

**Status**: [ ] Not yet created

---

## Dependencies

- Depends on: `production/epics/directors-cut/story-001-loot-bag-heat.md`, `production/epics/directors-cut/story-002-extraction-death.md`, `production/epics/directors-cut/story-004-run-progression.md`, `production/epics/directors-cut/story-005-pre-run-editing.md`, `production/epics/directors-cut/story-006-camp-meta.md`
- Unlocks: None
