# Story 010: Number retune — XP curve, monster strength, heat

> **Epic**: Director's Cut (MVP)
> **Status**: Ready
> **Layer**: N/A (minimal)
> **Type**: Config/Data
> **Estimate**: S
> **Tier**: Must (the PRD §10 success criteria depend on it). Value tuning is allowed after the Day 3 23:00 freeze (PRD §8 rule 4)
> **Manifest Version**: N/A (minimal — no control manifest)
> **Last Updated**: 2026-11-14 (reviewed against the PRD and the source tree; second review filled the gaps marked "Review 2")

## Context

**GDD**: `design/game-brief.md`
**Requirement**: `Brief MVP feature 5` (v2 list)

**ADR Governing Implementation**: N/A (minimal — no ADRs)
**ADR Decision Summary**: N/A (minimal — no ADRs)
**ADR Version**: N/A (minimal — no ADRs)

**Engine**: Unreal 5.8 | **Risk**: HIGH (`docs/engine-reference/unreal/VERSION.md`, 5.8 row)
**Engine Notes**: none (no ADR engine-compatibility analysis at minimal)

**Control Manifest Rules (this layer)**: N/A (minimal — no control manifest)

---

## Acceptance Criteria

Measured in one timed playthrough of the packaged build, from a deleted save, by someone playing normally:

- [ ] Two runs finish within 15 minutes, and each run lasts between 3 and 10 minutes (PRD §3).
- [ ] The player sees at least one star-up and makes at least one permanent cut.
- [ ] One reel is carried out, and its unlocked option is seen in the second run.
- [ ] With both runs extracted on starting cards, the player can pay for at least 2 building upgrades (PRD §10; stricter than the brief's "one building change", and the PRD wins).
- [ ] Slotting The Cut Knight card visibly changes the run, and the boss can be beaten by a normal build (PRD §4 item 2).
- [ ] Carrying a reel makes the trip to extraction visibly harder through heat, and a careful player can still extract.
- [ ] Growth does not run away: by the end of a 10-minute run enemies still threaten the player (PRD §11, first risk).
- [ ] The frame rate holds 60 FPS or more at 1080p with 150 enemies on screen (PRD §5). (Review 2) Measure it with the existing `-PerfProbe` flag on the packaged build, after the retune, and record the PERFPROBE line. The Day 1 result (average 132 FPS, worst frame 50) was taken in the editor before any v2 work.
- [ ] (Review 2) The first run of a new save, on starting cards, can be extracted by a player who has not seen the game: one outside tester, or failing that the developer playing without the dodge. Every other criterion here assumes extraction, and a judge who dies in both runs sees no unlock and no camp change.
- [ ] (Review 2) A full star-up path is reachable in one run: at least one option can reach two stars by level 6 if the player keeps taking it.
- [ ] Only data values change: the tables in `/Game/Data` and Director Tuning. No new logic.
- [ ] The packaged build reads the same values as the editor on a deleted save. The earlier mismatch (hero HP 100 in the package, 150 in the editor) is explained or fixed.

---

## Implementation Notes

- Retune XP curve, enemy health and heat together, not one at a time: cut-one-keep-two doubles growth and stars multiply it again.
- Where each number lives: XP curve in `DT_Progression`; enemies and boss in `DT_EnemyTuning`; spawn rate and cap in `DT_SpawnTuning`; hero health in `DT_HealthTuning`; star multipliers in Director Tuning `StarTuning`; building costs in `DT_Buildings`; heat and reel drop chance in the loot table that story 008 adds. (Review 2) Card payout multipliers and horde size are in `DT_FilmCards`; they decide whether two runs pay for two building upgrades.
- There is no "new game" button (PRD Should tier), so "from a deleted save" means deleting the save file by hand before the timed run. (Review 2) The slot is named `CampSave`: the file is `CampSave.sav` under the `Saved/SaveGames` folder of whichever build is being run. The editor and the packaged build keep separate saves, which is the likely cause of the 100 versus 150 hero HP reading (the editor save had Clinic upgrades); confirm it by comparing both on deleted saves.
- (Review 2) If story 008's loot table slips, heat stays a C++ default and cannot be tuned under the "only data values change" rule. In that case tune XP and enemy health only, and say in the completion notes that heat was not retuned.
- If the package mismatch turns out to be a code bug, fixing it is a bug fix, allowed after the freeze; note it in the completion notes.
- Record the before and after values and the reason for each change in the completion notes.
- Run a first pass as soon as story 008 is in, and a final pass after story 009 (or after it is cut): the pool size changes the pace.

---

## Out of Scope

- New enemies, cards, materials or buildings.
- A reset-save button.

---

## Files

All paths are under `Game_01 5.8/`.

- `Content/Data/DT_Progression`, `DT_EnemyTuning`, `DT_SpawnTuning`, `DT_HealthTuning`, `DT_Buildings`, `DT_Weapons`, `DT_Passives`, the loot table from story 008
- `Config/DefaultGame.ini` (Director Tuning `StarTuning`)

## QA Test Cases

*N/A — no qa-lead specs at this tier; implement against the Acceptance Criteria above.*

---

## Test Evidence

*`qa.level: minimal`: tests waived (advisory).*

**Story Type**: Config/Data
**Required evidence**: smoke check pass — the timed playthrough above, written up in `production/qa/smoke-2026-11-15.md` (or the date it is run) with run times and the value changes

**Status**: [ ] Not yet created

---

## Dependencies

- Depends on: `production/epics/directors-cut/story-008-lost-film-reels.md` (heat becomes tunable, reels unlock)
- Soft: `production/epics/directors-cut/story-009-unlockable-upgrades.md` — final pass waits for it only if it ships
- Unlocks: None
