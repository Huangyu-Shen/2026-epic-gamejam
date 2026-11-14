# Story 012: The Cut Knight brings the rejects

> **Epic**: Director's Cut (MVP)
> **Status**: Ready
> **Layer**: N/A (minimal)
> **Type**: Integration
> **Estimate**: L
> **Tier**: Could (`design/prd/master-prd.md` §6 row 3). **Cannot begin after Day 3 18:00** (PRD §8 rule 3). If it has not started by then, it is cut, not carried over
> **Manifest Version**: N/A (minimal — no control manifest)
> **Last Updated**: 2026-11-14 (reviewed against the PRD and the source tree; second review filled the gaps marked "Review 2")

## Context

**GDD**: `design/game-brief.md`
**Requirement**: `Brief MVP feature 7` (v2 list) — only if time allows

**ADR Governing Implementation**: N/A (minimal — no ADRs)
**ADR Decision Summary**: N/A (minimal — no ADRs)
**ADR Version**: N/A (minimal — no ADRs)

**Engine**: Unreal 5.8 | **Risk**: HIGH (`docs/engine-reference/unreal/VERSION.md`, 5.8 row)
**Engine Notes**: none (no ADR engine-compatibility analysis at minimal)

**Control Manifest Rules (this layer)**: N/A (minimal — no control manifest)

---

## Acceptance Criteria

- [ ] In a run with The Cut Knight card slotted, the boss gains what the player cuts in that run. Runs without the card have no boss and are unchanged.
- [ ] The boss spawns at run start, before any cut, so it gains each cut at the moment the player makes it.
- [ ] A cut weapon gives the boss that weapon's attack aimed at the hero; a cut passive raises the matching boss stat.
- [ ] Both permanent and skipped cuts count (everything in the run's cut list). Nothing carries over from earlier runs.
- [ ] With no cuts made yet, the boss fights as it does today (contact damage only).
- [ ] (Review 2) The boss gets the base version of a cut option, with no stars, whatever the player owns of it. Cutting the same weapon again raises that attack's damage by one step instead of adding a second copy of the attack.
- [ ] (Review 2) A cut that has no mapping (for example an option added by story 009) still counts: it gives the boss a small generic health bonus, so no cut is silently ignored.
- [ ] (Review 2) Boss attacks aimed at the hero can be seen coming and dodged: each has a visible wind-up or travel time. Their damage to the hero is its own data value, not the player weapon's damage, which is sized for enemy health.
- [ ] (Review 2) If the boss dies, it stays dead; later cuts do nothing and no HUD line is shown for them.
- [ ] How much each cut adds, and the cap on the total, are data, not code.
- [ ] The HUD tells the player when the boss gains a cut (one line per cut).
- [ ] A normal build that used all its permanent cuts can still beat the boss.
- [ ] Retained screenshot in `production/qa/evidence/`: the boss using a cut weapon, with the HUD line.

---

## Implementation Notes

- **Bigger than it looks.** The boss is a plain `AEnemy` with contact damage only, spawned by `UEnemySpawnSubsystem::SpawnBoss` from the `ForceBoss` film card at run start. `UAutoWeaponComponent` aims at enemies; aiming it at the hero is new work. That is why the estimate is L.
- The cut list already exists: `URunProgressionSubsystem::GetCuttingRoomFloor()` (every cut this run, oldest first). No change needed there except a notification when it grows.
- Decide the passive mapping at the start and write it here: Swiftness to move speed, Might to contact damage and Vitality to health are direct; Greed (XP gain) has no boss equivalent and needs a choice.
- (Review 2) Default for Greed, so the story is not blocked: treat it as the generic health bonus. Change it before coding if a better idea comes up.
- (Review 2) The boss spawns at second zero and the player's first cut comes at level 2. If the boss is killed or kills the player before the first level-up, the feature is never seen. Check this by playing before building anything; if it is so, the boss's health or the timing of the card has to change, and that is story 010's data, not new logic here.
- (Review 2) After the permanent cuts run out, every level-up still adds a skipped cut, so the boss grows for the whole run. The cap in the criteria is what stops this; set it low enough that the last criterion ("can still beat the boss") holds at level 15.
- Twice the estimate means it is cut (PRD §8 rule 5).

---

## Out of Scope

- New boss attacks beyond reusing the player's weapons and passives.
- A boss second phase (PRD Could tier, separate item).

---

## Files

All paths are under `Game_01 5.8/Source/Game_01/`.

- `Private/Enemies/Enemy.cpp` + `Public/Enemies/Enemy.h`, `Public/Enemies/EnemyTypes.h` (boss gains attacks and stat bonuses)
- `Private/Combat/AutoWeaponComponent.cpp` + header (attack a hero target)
- `Private/Progression/RunProgressionSubsystem.cpp` + header (notify when a cut is made)
- `Private/Enemies/EnemySpawnSubsystem.cpp` (hook the boss to the cut list)
- `Private/UI/RunHudWidget.cpp` or `HudMessageSubsystem` (boss line)
- Boss bonus values in `Content/Data/DT_EnemyTuning` or a new table

## QA Test Cases

*N/A — no qa-lead specs at this tier; implement against the Acceptance Criteria above.*

---

## Test Evidence

*`qa.level: minimal`: tests waived (advisory), but the screenshot is not.*

**Story Type**: Integration
**Required evidence**: integration test or documented playtest; retained screenshot in `production/qa/evidence/`

**Status**: [ ] Not yet created

---

## Dependencies

- Depends on: `production/epics/directors-cut/story-004-run-progression.md`, `production/epics/directors-cut/story-005-pre-run-editing.md` (both Complete; the cut list and the boss card exist)
- (Review 2) Depends on the code part of `production/epics/directors-cut/story-011-art-audio-feel.md`: the boss's weapon attacks need the real weapon visuals. Drawn with DrawDebug they would be invisible in a Shipping package.
- Order: last. Start only when stories 008-011 are done or safely under way, and before Day 3 18:00
- Unlocks: None
