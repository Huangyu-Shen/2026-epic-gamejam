# Story 013: Package and submission materials

> **Epic**: Director's Cut (MVP)
> **Status**: In Progress
> **Layer**: N/A (minimal)
> **Type**: Config/Data
> **Estimate**: M
> **Tier**: Must (`design/prd/master-prd.md` §7, Day 4 row, and §10). Not a feature: it adds nothing to the game, so the feature freeze does not apply
> **Manifest Version**: N/A (minimal — no control manifest)
> **Last Updated**: 2026-11-14 (added by the second review: stories 008-012 covered no Day 4 work)

## Context

**GDD**: `design/game-brief.md`
**Requirement**: Brief "Build order" step 1 (package test) and PRD §7 Day 4: "Build runs in a clean environment, materials ready"

**ADR Governing Implementation**: N/A (minimal — no ADRs)
**ADR Decision Summary**: N/A (minimal — no ADRs)
**ADR Version**: N/A (minimal — no ADRs)

**Engine**: Unreal 5.8 | **Risk**: HIGH (`docs/engine-reference/unreal/VERSION.md`, 5.8 row)
**Engine Notes**: none (no ADR engine-compatibility analysis at minimal)

**Control Manifest Rules (this layer)**: N/A (minimal — no control manifest)

---

## Acceptance Criteria

The build:
- [ ] The final package is built with the command in `production/session-state/active.md`, from a clean working tree at a named commit.
- [ ] Both maps load in the package: camp, start a run, extract or die, back to camp, twice in a row. (PARTLY, 2026-11-15, `story-013c`: the camp map loads as the default map with no ensure and no error; the run map loaded and settled an extraction twice in a row by probe. The walk-through by hand, camp to run to camp twice, is the author's)
- [x] The package starts from a deleted save and from an existing save. (checked 2026-11-15 in `story-013c`: a new slot gives "No camp save found", the next start gives "Loaded camp save", on the run map twice and on the camp map; a damaged file now gives a backup and a new camp)
- [ ] The zip leaves out `Game_01.pdb` and is under 1 GB (PRD §5). Record its size. (PARTLY: the folder is 703 MB without the pdb, measured on `story-013c`; no zip has been made yet)
- [ ] The zip is unpacked to a new folder (ideally on another Windows machine or user account) and the game starts from there. A build that only runs inside the project folder is not checked.
- [ ] No debug text, probe output or greybox stand-in is visible in normal play.
- [x] All in-game text is English (PRD §5). (checked 2026-11-15: no Chinese characters in the source or config)

The materials:
- [ ] A gameplay video of 30 to 60 seconds (PRD §4) that shows a level-up cut, a star-up and an extraction.
- [ ] The dev log has entries for Day 3 and Day 4, and `docs/devlog/README.md` has current numbers, tools and assets.
- [ ] The dev log is reachable at a public URL: the repository is public (confirmed 11/14), so the address is `https://github.com/Huangyu-Shen/2026-epic-gamejam/blob/main/docs/devlog/README.md`. Open it logged out and check the images load.
- [ ] The list of tools and of external assets with their licences is ready to paste into the submission form.
- [ ] The root `README.md` matches the game as shipped.

Hand-over:
- [ ] The user submits in person before 11/16 01:59 UK time. This story ends when the zip, the video and the texts are ready; uploading and submitting are not done by Claude.

---

## Progress (2026-11-15)

- Packages `story-013a` to `story-013d` were built with the command now in `project.yaml` (about 1 to 2 minutes each when the cook is warm).
- Packaging found a regression from the plugin limit: with the three dev plugins limited to the Editor, the packaged game logged an ensure ("Failed to load class GameFeatureData"), because they had been pulling in the GameFeatures plugin. Removing the `GameFeatureData` entry from `DefaultGame.ini` made the cook fail. Enabling the GameFeatures plugin in `Game_01.uproject` fixed it: no ensure, and the three dev plugins are not in the package.
- Still to do: the final package from a clean tree at a named commit, the hand walk-through, the unpack test in a new folder, the zip, the video, and the author's submission.

## Implementation Notes

- Package after every feature on Day 3, not only here; this story is the last check, not the first.
- The earlier silent failures were all found only in the package: a map missing from the cook, and tuning tables that fall back to C++ defaults without an error. Re-check both after story 008 adds a table and story 011 adds sounds.
- A Shipping build writes no log by default, so run the probes on a Development package first if the final one is Shipping (see story 011).
- Leave at least two hours between the final package and the deadline for the upload.

---

## Out of Scope

- Any change to the game itself; bugs found here go back to the story that owns them.
- Store page art beyond what the submission form requires.

---

## Files

- `Game_01 5.8/Saved/Packages/` (output, not committed)
- `docs/devlog/` (Day 3 and Day 4 entries, README)
- `README.md`
- `production/qa/smoke-2026-11-15.md` (the checks above, written up)

## QA Test Cases

*N/A — no qa-lead specs at this tier; implement against the Acceptance Criteria above.*

---

## Test Evidence

*`qa.level: minimal`: tests waived (advisory).*

**Story Type**: Config/Data
**Required evidence**: smoke check pass, written up in `production/qa/smoke-2026-11-15.md`, with the zip size and the commit hash

**Status**: [ ] Not yet created

---

## Dependencies

- Depends on: every story that ships (008, 010, 011; 009 and 012 if they are not cut)
- Unlocks: the submission
