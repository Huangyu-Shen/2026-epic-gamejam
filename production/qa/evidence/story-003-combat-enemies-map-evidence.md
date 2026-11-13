# Test Evidence: In-run combat, enemies and greybox map

> **Story**: `production/epics/directors-cut/story-003-combat-enemies-map.md`
> **Story Type**: Visual/Feel
> **Date**: 2026-11-12
> **Tester**: self (solo developer)
> **Build / Commit**: `UnrealEditor-Game_01.dll`, Development Win64 (UE 5.8) — `Build.bat Game_01Editor Win64 Development` succeeded

---

## What Was Tested

Top-down movement, an auto-attacking starting weapon, a chasing contact-damage
enemy, a heat-responsive spawn scheduler with horde and Boss events, a greybox
map with search points and two extraction zones, the 150-enemy frame-rate target,
and the data-driven source of all gameplay numbers.

**Acceptance criteria covered**: AC-1, AC-2, AC-3, AC-4, AC-5, AC-6, AC-7, AC-8, AC-9

---

## Acceptance Criteria Results

| # | Criterion (from story) | Result | Notes |
|---|----------------------|--------|-------|
| AC-1 | Top-down player: WASD move, dodge, an interact key, keyboard and mouse only. | PASS | Confirmed in PIE by the user. |
| AC-2 | 1 starting weapon auto-attacks nearby enemies with no button press. | PASS | Confirmed in PIE by the user. |
| AC-3 | Player takes damage and dies at 0 HP (death is handled by story 002). | PASS | `HandleDied` binds `OnDied` (fixed a missing `UFUNCTION()` this session) and reports death to `URunOutcomeSubsystem`. No player-facing death feedback yet — that is story 007. |
| AC-4 | 1 small enemy type chases the player and deals contact damage. | PASS | Confirmed in PIE by the user. |
| AC-5 | A spawn scheduler sends waves that ramp over time and respond to heat from story 001. | PASS | Confirmed in PIE by the user. |
| AC-6 | A horde event and a Boss can be spawned by the scheduler. | PASS | Confirmed in PIE by the user. |
| AC-7 | 1 fixed greybox map with search points and 2 extraction-point locations. | PASS | `L_Greybox.umap` binary scan: `ExtractionZone` x8, `LootContainer` x14, `PlayerStart` x5, `DirectorGameMode` x1. |
| AC-8 | 150 enemies on screen hold ≥ 60 FPS at 1080p. | PASS (advisory) | `enemies=152 avgFPS=132.3 minFPS=50.0 samples=1324`. Average clears 60; single worst frame below. Accepted by the user. |
| AC-9 | All numbers (HP, damage, wave rates, caps) come from data tables. | PASS | Four `DT_*Tuning` tables under `/Game/Data` wired through `UDirectorTuningSettings` in `Config/DefaultGame.ini`. |

---

## Screenshots / Video

| # | Filename | What It Shows | Acceptance Criterion |
|---|----------|--------------|----------------------|
| 1 | `story-003-combat-enemies-map/01-fight-horde-boss.png` | A busy fight with a horde of enemies around the hero | AC-4, AC-6 |

---

## Test Conditions

- **Game state at start**: fresh run on `L_Greybox`, hero at full health
- **Platform / hardware**: Windows, dev machine, 1080p
- **Framerate during test**: perf probe `enemies=152 avgFPS=132.3 minFPS=50.0`
- **Any special setup required**: H key spawned a horde; debug keys used for state setup

---

## Observations

- Enemies, hero and crates render as untextured engine cubes/capsules, so the
  fight reads grey-on-grey. The hero is tinted blue and enemies red in code, but
  the shared `BasicShapeMaterial` has no colour parameter, so the tint does not
  show. Cosmetic only; real materials land with art.
- The greybox map is a throwaway test map; the final map is a later story.

---

## Sign-Off

**Solo developer**: signed as lead.

| Role | Name | Date | Signature |
|------|------|------|-----------|
| Lead (art-director / designer) | self | 2026-11-12 | [x] Approved |

---

*Template: `.claude/docs/templates/test-evidence.md`*
*Used for: Visual/Feel and UI story type evidence records*
*Location: `production/qa/evidence/[story-slug]-evidence.md`*
