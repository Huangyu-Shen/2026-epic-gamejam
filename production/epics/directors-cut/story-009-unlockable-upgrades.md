# Story 009: Three unlockable upgrade options

> **Epic**: Director's Cut (MVP)
> **Status**: Ready
> **Layer**: N/A (minimal)
> **Type**: Logic
> **Estimate**: M
> **Tier**: Should (`design/prd/master-prd.md` §6 row 3) — must be started before Day 3 18:00 (PRD §8 rule 3). The first option is effectively Must, see "Minimum" below
> **Manifest Version**: N/A (minimal — no control manifest)
> **Last Updated**: 2026-11-14 (reviewed against the PRD and the source tree; second review filled the gaps marked "Review 2")

## Context

**GDD**: `design/game-brief.md`
**Requirement**: `Brief MVP feature 4` (v2 list)

**ADR Governing Implementation**: N/A (minimal — no ADRs)
**ADR Decision Summary**: N/A (minimal — no ADRs)
**ADR Version**: N/A (minimal — no ADRs)

**Engine**: Unreal 5.8 | **Risk**: HIGH (`docs/engine-reference/unreal/VERSION.md`, 5.8 row)
**Engine Notes**: none (no ADR engine-compatibility analysis at minimal)

**Control Manifest Rules (this layer)**: N/A (minimal — no control manifest)

---

## Acceptance Criteria

- [ ] Three new upgrade options exist: 2 passives and 1 weapon, each with a name and an offer description in English.
- [ ] On a new save none of the three appears in any level-up offer, and the pool is the current 7 options with 3 permanent cuts.
- [ ] An option enters the pool only after its reel has been carried out (story 008). It then shows in a level-up offer in the next run. This closes the judging experience "carry out a film, see the new item next run" (PRD §4 item 3).
- [ ] Each unlocked option raises the pool size by 1 and the permanent cuts by 1 (cuts = pool size − 4; PRD §11).
- [ ] (Review 2) In the first run after an unlock, the new option is guaranteed to be one of the three in the first level-up offer, and it is marked "NEW". Offers are a random shuffle today (`GenerateOffer`), so without this a judge can carry a reel out and still not see the option in a short second run.
- [ ] (Review 2) The offer text of the longest new option fits the offer panel, and the HUD loadout line still fits on screen with all 10 options owned.
- [ ] An unlocked option behaves like any other in a run: offered, kept, cut permanently, and merged at 3 and 9 copies into two and three stars.
- [ ] The new weapon has its own attack pattern, deals damage, and grows in reach with its stars like the other weapons.
- [ ] Each new passive changes one stat that no existing passive changes, and the HUD loadout line shows it.
- [ ] All values of the three options are in the weapon and passive data tables under `/Game/Data`; the C++ defaults are fallbacks only.
- [ ] A unit test covers: locked options are never offered; pool size and permanent cuts with 0, 1 and 3 unlocks.
- [ ] Retained screenshots in `production/qa/evidence/`: each new option in a level-up offer, and the new weapon firing.

---

## Implementation Notes

- **This is code, not only data.** Weapons and passives are enums (`EWeaponType`: Blade, Bolt, Nova; `EPassiveType`: Swiftness, Might, Vitality, Greed). Each new option needs a new enum value, a table row, an entry in `URunProgressionSubsystem`'s option naming and description, and (for the weapon) a new branch in `UAutoWeaponComponent`.
- **The effects are not fixed by the brief.** Decide them when the story starts and write them here. Each new passive needs a stat hook that does not exist yet (the four current ones cover move speed, damage, max health and XP gain); candidates the code can reach cheaply are attack interval and XP magnet radius.
- (Review 2) **Default choices, so the story is not blocked.** They are defaults, not decisions; change them before coding if you prefer others.
  - Passive "Haste": shortens every weapon's attack interval. The hook is `FWeaponRow::Interval`, read in `UAutoWeaponComponent`.
  - Passive "Magnet": widens the XP pickup radius. The radius is a hardcoded member today (`MagnetRadius = 600` in `RunProgressionSubsystem.h`); move it into the progression table first.
  - Weapon "Chain": hits the nearest enemy, then jumps to further enemies nearby. The three existing patterns are nearest single target (Blade), piercing line (Bolt) and radial burst (Nova), so a jumping hit is the one shape not yet used; stars add reach, as for the others.
  - Reel to option: one reel type each, in the order the minimum ships: the first reel type unlocks Haste, the second Magnet, the third Chain. The reels need film-style display names; "Swiftstep", "Warbrand" and "Aegis" are the names of the removed buffs.
- (Review 2) Keep the pool filter a pure function (all rows in, unlocked set in, pool out) so the unit test needs no world and no save file.
- (Review 2) `PermanentCutsLeft` is computed from `Weapons.Num() + Passives.Num()` at run start. Once the tables hold locked rows, that count must use the filtered pool, or a new save would get 6 permanent cuts.
- **Minimum:** if time runs short, ship one passive first, wired end to end with its reel, before starting the other two. One unlockable option is enough for the judging experience; three is the Should target.
- `-OfferProbe` on `ADirectorGameMode` forces level-ups and takes screenshots; use it, with an unlocked save, for the evidence.
- After the feature works in the editor, re-run the packaged build probe.

---

## Out of Scope

- More than three options; any new enemy, card, material or building.
- Final art and sound for the new weapon (story 011).

---

## Files

All paths are under `Game_01 5.8/`.

- `Source/Game_01/Public/Progression/ProgressionTypes.h` (new enum values, new passive fields)
- `Source/Game_01/Public/Progression/RunProgressionSubsystem.h` + `Private/Progression/RunProgressionSubsystem.cpp` (pool filtered by the unlocked set, names, descriptions, new stat getters)
- `Source/Game_01/Private/Combat/AutoWeaponComponent.cpp` + header (new weapon pattern)
- `Source/Game_01/Private/Player/HeroCharacter.cpp` (read the new passive stats)
- `Content/Data/DT_Weapons`, `DT_Passives` (three new rows), the reel-to-option mapping table from story 008
- `Source/Game_01/Private/Tests/` (pool filter and cut count test, beside `StarTuningTest.cpp`)

## QA Test Cases

*N/A — no qa-lead specs at this tier; implement against the Acceptance Criteria above.*

---

## Test Evidence

*`qa.level: minimal`: tests waived (advisory), but the screenshots are not.*

**Story Type**: Logic
**Required evidence**: automated unit test in `Game_01 5.8/Source/Game_01/Private/Tests/` (advisory at this tier); retained screenshots in `production/qa/evidence/`

**Status**: [ ] Not yet created

---

## Dependencies

- Depends on: `production/epics/directors-cut/story-008-lost-film-reels.md`
- Unlocks: None (stories 010 and 011 can proceed with or without it)
