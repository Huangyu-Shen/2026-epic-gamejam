# Story 011: Art, audio and feel

> **Epic**: Director's Cut (MVP)
> **Status**: Ready
> **Layer**: N/A (minimal)
> **Type**: Visual/Feel
> **Estimate**: L
> **Tier**: Should (`design/prd/master-prd.md` §6 row 11), except replacing the DrawDebug weapon visuals, which is Must for a Shipping package. Code parts must be in before the Day 3 23:00 freeze; swapping assets is allowed after it (PRD §8 rule 4)
> **Manifest Version**: N/A (minimal — no control manifest)
> **Last Updated**: 2026-11-14 (reviewed against the PRD and the source tree; second review filled the gaps marked "Review 2")

## Context

**GDD**: `design/game-brief.md`
**Requirement**: `Brief MVP feature 6` (v2 list)

**ADR Governing Implementation**: N/A (minimal — no ADRs)
**ADR Decision Summary**: N/A (minimal — no ADRs)
**ADR Version**: N/A (minimal — no ADRs)

**Engine**: Unreal 5.8 | **Risk**: HIGH (`docs/engine-reference/unreal/VERSION.md`, 5.8 row)
**Engine Notes**: none (no ADR engine-compatibility analysis at minimal)

**Control Manifest Rules (this layer)**: N/A (minimal — no control manifest)

---

## Acceptance Criteria

Code, before the freeze (do these first):
- [ ] Weapon attacks are drawn with real visuals. No gameplay visual depends on DrawDebug (today `AutoWeaponComponent.cpp` does), and the attacks show in the packaged build.
- [ ] On star-up, a weapon's look (projectile, area or size) grows together with its numbers.
- [ ] One audio entry point exists and gameplay code calls it for: attack, enemy hit, enemy death, pickup, level-up, cut, star-up, extraction, player death. No sound code exists today.
- [ ] Hits give feedback the player can see and hear (at least a flash or knock, plus a sound).
- [ ] Music plays in the camp and in the run: 2 tracks (PRD §6 row 11). (Review 2) Each track loops, and travelling camp to run and back never leaves two tracks playing at once or restarts the same track on top of itself.
- [ ] (Review 2) Sound stays readable in a crowd: the number of hit and death sounds playing at once is capped (a concurrency limit), so 150 enemies do not produce a wall of noise or a frame drop.
- [ ] (Review 2) Volumes are set once in data, since there is no settings screen: music sits under the effects, and nothing clips at the default Windows volume.
- [ ] (Review 2) Sounds and music are referenced so that the cook includes them. Assets reached only through a soft pointer in settings are skipped by the cooker without an error (this already happened with the run map), so either reference them hard or put their folder in the always-cook list, then confirm in the packaged build by ear.

Assets (can continue after the freeze):
- [ ] Hero, enemies, boss, pickups, vault, extraction point and map read as one style: a unified material and palette replaces the greybox look. The hero's colour is correct (it is grey today; the Color parameter does nothing).
- [ ] Hero, grunts and boss stay easy to tell apart at a glance against the floor, with 150 enemies on screen.

Constraints (PRD §5):
- [ ] The frame rate still holds 60 FPS or more at 1080p with 150 enemies on screen after the asset swap.
- [ ] The packaged build is under 1 GB without the `.pdb` file. It was about 0.75 GB before any art.
- [ ] No trademarks or logos; content suits ages 13+; all text and audio are English.
- [ ] Every external asset and every AI tool used is listed in `docs/devlog/README.md` (Assets and Tools sections) with its source and licence. WeLoveIndies audio is non-commercial only.
- [ ] (Review 2) Before any WeLoveIndies file is imported, the user confirms that a jam with cash prizes counts as non-commercial under that licence. This has not been checked. If it is unclear, use engine built-ins or another source with a licence that plainly allows it.

Evidence:
- [ ] Retained screenshots in `production/qa/evidence/`: camp, a run in combat, a star-up, all taken from the packaged build.

---

## Implementation Notes

- Order of work: DrawDebug replacement, audio entry point and hit feedback first (they are code and freeze at 23:00), then asset swaps.
- If the whole story reaches twice its estimate, cut in this order (PRD §8 rule 5): music layering and extra sounds, then the map dressing, then the pickup and vault models. Never cut the DrawDebug replacement.
- Check free-pack availability and licence before importing; nothing has been verified yet.
- If story 009 ships, its new weapon needs a look and a sound too; if it is cut, ignore this.
- Decide Shipping or Development for the final package once the DrawDebug visuals are gone, and re-run the package command after the first batch of assets, not only at the end.
- (Review 2) Shipping has a second cost: the `-OfferProbe` and `-PerfProbe` checks report through the log, and a Shipping build writes no log by default. If the final package is Shipping, take the probe and frame-rate evidence from a Development package of the same content first.
- (Review 2) `AStandInCharacter` also calls DrawDebug. It is the greybox stand-in, not part of the shipped game; leave it, and do not count it against the first criterion.
- (Review 2) Hit feedback runs on up to 150 enemies. Use something cheap (a material parameter flash or a short scale punch), not a spawned effect per hit.

---

## Out of Scope

- UI animation polish, title screen, settings, tutorial hints (PRD Should and Could items not in the brief's v2 list).
- CUT screen wipe and gray-to-color restore (rejected; brief Out of scope).
- Music that layers with heat (PRD Could tier).

---

## Files

All paths are under `Game_01 5.8/`.

- `Content/` (materials, meshes, VFX, sounds, music)
- `Source/Game_01/Private/Combat/AutoWeaponComponent.cpp` (replace DrawDebug)
- `Source/Game_01/Private/Player/HeroCharacter.cpp`, `Private/Enemies/Enemy.cpp` (materials, hit feedback)
- New audio subsystem under `Source/Game_01/Public/Audio/` and `Private/Audio/`
- Call sites: `RunProgressionSubsystem.cpp`, `LootContainer.cpp`, `ExtractionZone.cpp`, `HealthComponent.cpp`, `CampGameMode.cpp`, `DirectorGameMode.cpp`
- `docs/devlog/README.md` (Assets and Tools sections)

## QA Test Cases

*N/A — no qa-lead specs at this tier; implement against the Acceptance Criteria above.*

---

## Test Evidence

*`qa.level: minimal`: tests waived (advisory), but the screenshots are not.*

**Story Type**: Visual/Feel
**Required evidence**: retained screenshots + sign-off in `production/qa/evidence/`. Sound cannot be shown in a screenshot: the user confirms it by ear in the packaged build

**Status**: [ ] Not yet created

---

## Dependencies

- Depends on: none of the v2 stories; can run alongside 008-010
- Soft: `production/epics/directors-cut/story-009-unlockable-upgrades.md` (its weapon needs a look if it ships)
- Unlocks: None
