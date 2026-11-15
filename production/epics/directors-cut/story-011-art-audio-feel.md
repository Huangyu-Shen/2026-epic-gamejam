# Story 011: Art, audio and feel

> **Epic**: Director's Cut (MVP)
> **Status**: In Progress
> **Layer**: N/A (minimal)
> **Type**: Visual/Feel
> **Estimate**: L
> **Tier**: Should (`design/prd/master-prd.md` §6 row 11), except replacing the DrawDebug weapon visuals, which is Must for a Shipping package. Code parts must be in before the Day 3 23:00 freeze; swapping assets is allowed after it (PRD §8 rule 4)
> **Manifest Version**: N/A (minimal — no control manifest)
> **Last Updated**: 2026-11-14

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
- [x] Weapon attacks are drawn with real visuals. No gameplay visual depends on DrawDebug (today `AutoWeaponComponent.cpp` does), and the attacks show in the packaged build. (checked 2026-11-15 by code: `AWeaponEffect` uses meshes; DrawDebug remains only in `StandInCharacter`; screenshot 3-packaged-bolt-beam)
- [x] On star-up, a weapon's look (projectile, area or size) grows together with its numbers. (checked 2026-11-15 by code: `AutoWeaponComponent.cpp:54-62` multiplies the weapon's reach by the star range multiplier, and that reach sets beam thickness, ring radius, bullet size and orb size; `StarTuningTest` checks that 3 stars reach further than 2. Size only: there is no colour or shape change. Screenshot 12 shows Gun and Orbit at two stars, but there is no before/after pair)
- [x] One audio entry point exists and gameplay code calls it for: attack, enemy hit, enemy death, pickup, level-up, cut, star-up, extraction, player death. (checked by code: `GameAudioSubsystem`, called from 11 files; 11 sounds assigned in `DefaultGame.ini`)
- [x] Hits give feedback the player can see and hear (at least a flash or knock, plus a sound). (checked 2026-11-15 in `Enemy.cpp:102-125` and by the author's play: EnemyHit sound plus a 0.1 s scale pop to 1.2x that carries the model along. The white colour flash only changes the hidden `Body` material, so it shows on cube enemies but not on modelled ones)
- [x] Music plays in the camp and in the run: 2 tracks (PRD §6 row 11). (Review 2) Each track loops, and travelling camp to run and back never leaves two tracks playing at once or restarts the same track on top of itself. (confirmed by the author's play, 2026-11-15; code has `StartMusic` with same-track check and fade-out)
- [x] (Review 2) Sound stays readable in a crowd: the number of hit and death sounds playing at once is capped (a concurrency limit), so 150 enemies do not produce a wall of noise or a frame drop. (code: `MaxCrowdSoundsPerSecond = 16`; the author heard no problem in play, 2026-11-15)
- [x] (Review 2) Volumes are set once in data, since there is no settings screen: music sits under the effects, and nothing clips at the default Windows volume. (data: master 0.8, music 0.35, sfx 0.7; the author heard no problem in play, 2026-11-15. Note: since #6 the Esc menu has volume settings)
- [x] (Review 2) Sounds and music are referenced so that the cook includes them. Assets reached only through a soft pointer in settings are skipped by the cooker without an error (this already happened with the run map), so either reference them hard or put their folder in the always-cook list, then confirm in the packaged build by ear. (config done: `/Game/Free_Sounds_Pack/cue` and `/Game/PathOfAdventure/Loop/CUE` are in `DirectoriesToAlwaysCook`; confirmed by the author's play of the packaged build, 2026-11-15)

Assets (can continue after the freeze):
- [x] Hero, enemies, boss, pickups, vault, extraction point and map read as one style: a unified material and palette replaces the greybox look. The hero's colour is correct. (checked 2026-11-15 on screenshots 12 and 19: the hero is a modelled knight, not grey)
- [x] Hero, grunts and boss stay easy to tell apart at a glance against the floor, with 150 enemies on screen. (screenshot 12-forest-floor-and-horde; a 150-enemy shot with weapons firing does not exist)

Constraints (PRD §5):
- [ ] The frame rate still holds 60 FPS or more at 1080p with 150 enemies on screen after the asset swap. (PARTLY: packaged probe 151 enemies, avg 130 FPS, min 96, but with weapons off, taken at story-011 time; not re-run after later assets)
- [ ] The packaged build is under 1 GB without the `.pdb` file. It was about 0.75 GB before any art. (STALE: 679 MB at story-011 time; many packs were imported since, re-measure in story 013)
- [ ] No trademarks or logos; content suits ages 13+; all text and audio are English. (NOT CHECKED systematically)
- [x] Every external asset and every AI tool used is listed in `docs/devlog/README.md` (Assets and Tools sections) with its source and licence. (checked 2026-11-15: audio and model packs listed with licences, CC-BY credit in the Esc menu; two imported packs are marked "not used so far")
- [x] (Review 2) Before any WeLoveIndies file is imported, the user confirms that a jam with cash prizes counts as non-commercial under that licence. (NOT NEEDED: no WeLoveIndies audio was used; the sounds and music come from Fab packs, see README)

Evidence:
- [ ] Retained screenshots in `production/qa/evidence/`: camp, a run in combat, a star-up, all taken from the packaged build. (camp and combat exist in `story-011-art-audio-feel/`; no star-up screenshot found; whether each was taken from the packaged build is not stated except 3-packaged-bolt-beam)

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

- `Game_01 5.8/Source/Game_01/Public/Audio/GameAudioSubsystem.h`, `Private/Audio/GameAudioSubsystem.cpp` (new: audio entry point, music, crowd cap)
- `Game_01 5.8/Source/Game_01/Public/Combat/WeaponEffect.h`, `Private/Combat/WeaponEffect.cpp` (new: weapon visuals, replace DrawDebug)
- `Game_01 5.8/Source/Game_01/Private/Combat/AutoWeaponComponent.cpp`, `Public/Combat/AutoWeaponComponent.h` (visuals, attack sound)
- `Game_01 5.8/Source/Game_01/Private/Enemies/Enemy.cpp`, `Public/Enemies/Enemy.h`, `Private/Enemies/EnemyManagerSubsystem.cpp` (hit flash, hit and death sound)
- `Game_01 5.8/Source/Game_01/Public/Game/DirectorTuningSettings.h` (audio settings)
- `Game_01 5.8/Source/Game_01/Private/Progression/RunProgressionSubsystem.cpp`, `Private/Extraction/RunOutcomeSubsystem.cpp`, `Private/Loot/LootContainer.cpp`, `Private/Camp/CampGameMode.cpp`, `Private/Game/DirectorGameMode.cpp` (sound calls, music start, weapon-visual probe shots)

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
