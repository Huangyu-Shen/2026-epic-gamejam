# Story 008: Lost film reels — relics become unlock keys

> **Epic**: Director's Cut (MVP)
> **Status**: Complete (editor); packaged-build check below
> **Layer**: N/A (minimal)
> **Type**: Integration
> **Estimate**: M
> **Tier**: Must (`design/prd/master-prd.md` §6 row 5) — not subject to the Day 3 18:00 cut
> **Manifest Version**: N/A (minimal — no control manifest)
> **Last Updated**: 2026-11-14 (reviewed against the PRD and the source tree; second review filled the gaps marked "Review 2")

## Context

**GDD**: `design/game-brief.md`
**Requirement**: `Brief MVP feature 3` (v2 list)

**ADR Governing Implementation**: N/A (minimal — no ADRs)
**ADR Decision Summary**: N/A (minimal — no ADRs)
**ADR Version**: N/A (minimal — no ADRs)

**Engine**: Unreal 5.8 | **Risk**: HIGH (`docs/engine-reference/unreal/VERSION.md`, 5.8 row)
**Engine Notes**: none (no ADR engine-compatibility analysis at minimal)

**Control Manifest Rules (this layer)**: N/A (minimal — no control manifest)

---

## Acceptance Criteria

Removing the v1 behaviour:
- [x] A carried reel cannot be used in a run: the number-key "use" binding, `ULootBagComponent::UseRelic`, the relic buff and the heat drop on use are gone, and no HUD text still mentions using a relic.
- [x] A reel cannot be dismantled (salvaged) at the camp warehouse: the button and the code path behind it are gone.
- [x] Every screen that showed a "relic" now says "lost film reel" (HUD, bag, result screen, camp panel). All text is English.

The new behaviour:
- [x] Picking up a reel still raises heat (the existing pickup heat). This is how "heat rises while you carry one" is met; no new per-second heat.
- [x] Each reel type unlocks one fixed upgrade option. The reel-to-option mapping is data, not code.
- [x] Extracting with a reel adds its option to the save's unlocked set. Several reels in one bag each unlock their own option.
- [x] Dying loses the reel and unlocks nothing; the result screen says the reel was lost.
- [x] A reel type whose option is already unlocked no longer drops, so the player never carries out a reel that does nothing. When every option is unlocked, containers give no reel.
- [x] (Review 2) A reel type that is already in the bag does not drop again in the same run. Today `ULootConfig::PickRandomRelic` can roll the same type twice, and the second copy would cost a bag slot and heat and unlock nothing.
- [x] (Review 2) A reel type with no option mapped to it, or mapped to an option that does not exist yet, never drops. Until story 009 adds options this means only the placeholder reel drops; if 009 ships one option, only one reel type drops.
- [x] (Review 2) While at least one reel can still drop, the vault always holds one (the vault's reel chance stays 1.0), so the first run of a new save can reach the "carry a reel out" experience without luck.
- [x] The result screen names the option each extracted reel unlocked.
- [x] The camp storage panel lists the unlocked options (PRD §6 row 8).

Save and data:
- [x] The unlocked set is saved and survives a restart of the game. (Review 2) It is saved at settlement, so quitting on the result screen does not lose the unlock.
- [x] (Review 2) The save stores each unlocked option by a stable name, not by enum number. Story 009 adds enum values, and a number saved today must not point at a different option tomorrow.
- [x] (Review 2) The unlocked set lives in a game-instance subsystem (today's `UCampStockSubsystem`), because the run map and the camp map are different worlds and `URunProgressionSubsystem` is a world subsystem that is rebuilt every run.
- [x] A v1 save (it holds `WarehouseRelics`) still loads: each stored relic becomes its unlocked option, and `SaveVersion` goes to 2. (Review 2) Duplicate relics in a v1 warehouse collapse into one unlock and give no Iron; relics whose type has no mapped option are dropped. Only development saves exist, so nothing is owed to players.
- [x] Bag, heat and container tuning (`FLootTuning`, container rows, reel drop chance) is read from a data table under `/Game/Data` through Director Tuning. Today these are C++ defaults in `LootConfig.cpp` only, so story 010 cannot tune heat without this.
- [x] (Review 2) The packaged build really loads that table. A missing table falls back to the C++ defaults without an error, so the check is a log line that prints the loaded heat values at run start, read from the packaged build's log, with one value in the table set different from the C++ default.

Evidence:
- [x] Retained screenshots in `production/qa/evidence/`: result screen after extracting a reel, result screen after dying with a reel, camp storage panel with an unlocked option, (Review 2) and the run HUD with a reel in the bag showing the new name.

---

## Implementation Notes

- This changes v1 behaviour (brief "Changes to v1 behavior"): relic use and warehouse dismantling go away. Say so in the closing summary.
- The unlock becomes visible in a level-up offer only once story 009 adds an option to unlock. Build 008 against one placeholder mapping, then finish the "seen in the next run" check inside story 009.
- Keep the unlocked set as plain data (a set of option ids) with pure add / contains functions, so it can be unit tested without a world.
- Remove the same `UseRelicSlot` path from `AStandInCharacter` (the greybox stand-in) or it stops compiling.
- After the feature works in the editor, re-run the packaged build probe (command in `production/session-state/active.md`).
- (Review 2) Dead code to remove with the v1 behaviour, beyond `UseRelic`: `UCampMetaSubsystem::DismantleRelic` and its `RelicDismantleIron` value, `UCampStockSubsystem::RemoveRelicAt`, `ULootHeatSubsystem::ReduceHeatForRelicUse`, and `FLootStatModifier` (the relic buff struct). Removing only the callers leaves them compiling but unreachable.
- (Review 2) `FRunResult` carries `RelicsBanked` and `RelicsLost`; the result screen reads them. Keep the fields, change what the screen prints for them.
- (Review 2) A new data table is a `.uasset`, which has to be made in the editor (by hand or through the Unreal MCP server; the server was not reachable in the last session, so check it first). `/Game/Data` is already in the always-cook list.
- (Review 2) Check what happens when a reel is offered to a full bag. Today `TryAddLoot` refuses the whole pickup with "Bag is full". Confirm by playing that the refused reel is not silently destroyed; if it is, leave it in the container.

---

## Out of Scope

- The three unlockable options themselves (story 009).
- Changing heat values (story 010); this story only makes them tunable.
- NG film on death (PRD Should tier, not in the brief's v2 list).

---

## Files

All paths are under `Game_01 5.8/Source/Game_01/`.

- `Public/Loot/LootTypes.h` (`FRelicRow`: drop `Buff`, add the unlocked option; `FLootTuning`: drop `HeatReductionOnRelicUse`)
- `Private/Loot/LootBagComponent.cpp` + header (remove `UseRelic`), `LootHeatSubsystem` (remove the heat drop on use), `LootConfig` (table loading, skip unlocked reel types), `LootContainer.cpp` (drop roll)
- `Private/Player/HeroCharacter.cpp` + header, `Private/StandIn/StandInCharacter.cpp` + header (remove `UseRelicSlot`)
- `Public/Flow/CampSaveGame.h`, `Private/Flow/CampSaveSubsystem.cpp` (unlocked set, `SaveVersion` 2, v1 migration)
- `Extraction/CampStockSubsystem` (warehouse relics become the unlocked set; remove the remove-relic call), `Extraction/RunOutcomeSubsystem` (unlock on extraction)
- (Review 2) `Camp/CampMetaSubsystem` (remove `DismantleRelic` and the dismantle Iron value)
- `Private/UI/CampWidget.cpp`, `Private/UI/RunHudWidget.cpp` (text, result screen, storage panel)
- `Public/Game/DirectorTuningSettings.h` + `Config/DefaultGame.ini` (loot tuning table), new `Content/Data/DT_Loot*` table
- `Private/Tests/` (new: unlocked-set and v1-save migration test)

## QA Test Cases

*N/A — no qa-lead specs at this tier; implement against the Acceptance Criteria above.*

---

## Test Evidence

*`qa.level: minimal`: tests waived (advisory), but the screenshots are not.*

**Story Type**: Integration
**Required evidence**: integration test or documented playtest; retained screenshots of the screens touched, in `production/qa/evidence/`

**Status**: [x] Created: `production/qa/evidence/story-008-lost-film-reels/` (4 screenshots + `probe-log.txt`), 5 automation tests in `ReelUnlockTest.cpp` (all pass, with the other 4 the suite is 9 of 9)

---

## Dependencies

- Depends on: `production/epics/directors-cut/story-001-loot-bag-heat.md`, `production/epics/directors-cut/story-002-extraction-death.md`, `production/epics/directors-cut/story-007-flow-save-ui.md` (all Complete)
- Unlocks: `production/epics/directors-cut/story-009-unlockable-upgrades.md`, `production/epics/directors-cut/story-010-number-retune.md` (heat becomes tunable)

---

## Completion Notes

**Completed**: 2026-11-14
**Changes to v1 behaviour (say so in the summary):** a reel can no longer be used in a run (keys 1-6 for relics are gone; 1-3 only cut a level-up option); the camp warehouse no longer dismantles relics for Iron; a reel's buff and the heat drop on use are gone. The Fast Cut reel is the only one that drops until story 009 gives the other two an option.

**What was built**
- Reels unlock the option named in `DT_Reels` (`UnlocksOption`); the unlocked set lives in `UCampStockSubsystem` and is saved by option name, `SaveVersion` 2.
- Drop rules: a reel drops only if it maps to an option, the option is not unlocked yet and the bag does not hold that reel type (`FRelicRow::IsDroppable`, `ULootConfig::PickRandomRelic`). The vault still rolls a reel at chance 1.0 whenever one can drop.
- Loot tuning is now data: `DT_LootTuning`, `DT_LootContainers`, `DT_Reels` in `/Game/Data`, wired through Director Tuning in `Config/DefaultGame.ini`. The bag logs `LOOTCFG source=table|c++ defaults ...` at run start.
- New probes: `-ReelProbe` / `-ReelProbe=die` and `-CampShot`, plus `-SaveSlot=Name` so a probe never touches the real save.

**Found by playing, not by compiling**
- The first version defaulted `SaveVersion` to 2. A save file stores only values that differ from the class default, so a v1 file (version 1, equal to the old default, never written) read back as version 2 and skipped the migration. Found by loading a copy of the real dev save: the migration log line never appeared. Fixed by keeping the default at 1 and writing `CurrentSaveVersion` (2) explicitly. The new test `Game_01.Loot.V1SaveRelicsBecomeUnlockedOptions` was seen to fail against the unfixed default before the fix went in.
- After the fix, the real dev save (8 relics: Warbrand and Aegis only, none of the mapped type) migrates to 0 unlocked options, as the rules say. The mapped-reel migration is covered by the unit test only, not by a real save.

**Not done / not checked**
- Dead code still present on purpose: the enum is still named `ERelicType` and the bag members `Relics` (renaming would touch the save format); only text and behaviour changed.
- Not tried by hand: searching a real crate or vault and picking up a reel with E (the probe puts the reel in the bag directly). The drop rules are covered by unit tests and by the probe's `anotherReelCanDrop` log field.
- Packaged build (Development, `Saved/Packages/story-008`): the log line reads `LOOTCFG source=table ... heatPerReel=15.0`, so the three tables are cooked and loaded; the reel probe extracts, the unlock is saved, and a second launch of the camp on the same slot loads `1 unlocked options` (`production/qa/evidence/story-008-lost-film-reels/packaged/`). The table values equal the C++ defaults, so `source=table` is the proof, not a changed number; story 010 changes the numbers.
- Not checked in the package: the full camp -> run -> camp loop by hand, and audio (none exists yet).
