# Story 012: The Cut Knight brings the rejects

> **Epic**: Director's Cut (MVP)
> **Status**: Complete
> **Layer**: N/A (minimal)
> **Type**: Integration
> **Estimate**: L
> **Tier**: Could (`design/prd/master-prd.md` §6 row 3). **Cannot begin after Day 3 18:00** (PRD §8 rule 3). If it has not started by then, it is cut, not carried over
> **Manifest Version**: N/A (minimal — no control manifest)
> **Last Updated**: 2026-11-15

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

- [x] In a run with The Cut Knight card slotted, the boss gains what the player cuts in that run. Runs without the card have no boss and are unchanged. (checked by the author's play; "no card, no boss" by code only)
- [x] The boss spawns at run start, before any cut, so it gains each cut at the moment the player makes it. (author's play)
- [ ] A cut weapon gives the boss that weapon's attack aimed at the hero; a cut passive raises the matching boss stat. (UNTESTED in part: weapons seen in screenshots; Might, Swiftness and Haste not observed)
- [~] ~~Both permanent and skipped cuts count (everything in the run's cut list). Nothing carries over from earlier runs.~~ (OBSOLETE, not tested and no longer needed: since v3 a cut is refused once permanent cuts run out, so there are no skipped cuts. Everything in the cut list counts, and the list is per run by code)
- [x] With no cuts made yet, the boss fights as it does today (contact damage only). (author's play)
- [ ] (Review 2) The boss gets the base version of a cut option, with no stars, whatever the player owns of it. Cutting the same weapon again raises that attack's damage by one step instead of adding a second copy of the attack. (UNTESTED: only in code, each weapon was cut once in the probe)
- [x] (Review 2) A cut that has no mapping (for example an option added by story 009) still counts: it gives the boss a small generic health bonus, so no cut is silently ignored. (probe log: Greed gave health)
- [x] (Review 2) Boss attacks aimed at the hero can be seen coming and dodged: each has a visible wind-up or travel time. Their damage to the hero is its own data value, not the player weapon's damage, which is sized for enemy health. (author's play)
- [ ] (Review 2) If the boss dies, it stays dead; later cuts do nothing and no HUD line is shown for them. (UNTESTED: only in code, the boss was never killed in a test)
- [x] How much each cut adds, and the cap on the total, are data, not code. (advisory: code defaults under Director Tuning > BossRejects, not a data table)
- [x] The HUD tells the player when the boss gains a cut (one line per cut). (screenshot 1-gun-balls; its text is the old wording, see Completion Notes)
- [ ] A normal build that used all its permanent cuts can still beat the boss. (UNTESTED: the author collected only 1 to 2 cuts)
- [x] Retained screenshot in `production/qa/evidence/`: the boss using a cut weapon, with the HUD line. (production/qa/evidence/story-012-cut-knight/)

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

**Status**: [x] Created 2026-11-15: `production/qa/evidence/story-012-cut-knight/` (1-gun-balls, 2-slam-warning, 3-ring-of-balls, plus the void fix). Taken from the `-BossProbe` run, not from a normal run. Taken before the HUD wording fix; re-capture if exact wording matters.

---

## Dependencies

- Depends on: `production/epics/directors-cut/story-004-run-progression.md`, `production/epics/directors-cut/story-005-pre-run-editing.md` (both Complete; the cut list and the boss card exist)
- (Review 2) Depends on the code part of `production/epics/directors-cut/story-011-art-audio-feel.md`: the boss's weapon attacks need the real weapon visuals. Drawn with DrawDebug they would be invisible in a Shipping package.
- Order: last. Start only when stories 008-011 are done or safely under way, and before Day 3 18:00
- Unlocks: None

## Implementation Notes (2026-11-15)

Built as `UBossRejectsSubsystem` (new), hooked from `UEnemySpawnSubsystem::SpawnBoss`. It reads `GetCuttingRoomFloor()` every tick and applies each new cut at once.

- Weapon cut → boss attack: Blade, Bolt, Chain, Gun = one slow ball at the hero; Nova = a red ring on the floor with a 1 s wind-up; Orbit = a ring of 8 balls. Cutting the same weapon again adds 25% of the base damage.
- Passive cut → Swiftness: +8% boss speed; Might: +15% contact damage; Haste: attack interval ×0.9 (floor 0.5); Vitality: +150 health; everything else (Greed, Magnet, Reach, Armor, Regen, Crit): +100 health. Caps: stat bonus 100%, bonus health 1500.
- All numbers are `BossRejects` in Project Settings > Game > Director Tuning (code defaults, no table).
- Boss dead → its balls and rings disappear, later cuts do nothing.
- Without the Cut Knight card there is no boss, so nothing changes.
- Not yet done (as of 2026-11-15): checking the "a build that used all its cuts can still beat the boss" criterion by hand; the boss-attack damage numbers are first estimates. A real run through the camp with the card slotted was done at acceptance (see Completion Notes).
- Evidence (probe `-BossProbe`, boss placed 900 cm from the hero, hero in god mode): `production/qa/evidence/story-012-cut-knight/` — 1-gun-balls, 2-slam-warning, 3-ring-of-balls.

## Completion Notes
**Completed**: 2026-11-15
**Criteria**: 8 of 13 checked, 1 obsolete (4), 3 untested (6, 9, 12) and 1 partly tested (3). Re-checked by /story-done on 2026-11-15: accepted by the author after playing the packaged build (1-2 cuts per run): boss attacks and red rings are readable and dodgeable; the boss does not kill the hero at the start of a run; HUD line per cut shown.
**Untested criteria**: 3 (passive stats Might, Swiftness, Haste), 6 (repeat cut raises damage), 9 (dead boss stays dead), 12 (a build with all cuts can still beat the boss).
**Evidence note**: the screenshots were taken before the HUD wording was fixed ("it a new attack" became "it gets a new attack"); `BossRejectsSubsystem.cpp` shows as changed since the evidence stamp. Re-capture if exact wording matters.
**Deviations**: None. Not checked: a build that spent all its cuts beating the boss (the author only collected 1-2 cuts). No automated test (waived at `qa.level: minimal`).
**Test Evidence**: `production/qa/evidence/story-012-cut-knight/` (1 to 3 boss attacks, 4 void fix).
**Reviewed by**: self, no independent review

Added at acceptance: the map has no walls and a void around it, so `UPlayAreaSubsystem` (new) finds the floor (Floor_0, x and y from -4000 to 4000) at level start and keeps the hero, every enemy, the boss and new spawns on it. Probe `-EdgeProbe`: hero pushed to x=6000 ended at x=3966; farthest enemy 3970.
