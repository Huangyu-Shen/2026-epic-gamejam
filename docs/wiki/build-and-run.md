# Running, Testing and Releasing

Paths are relative to the repository root. The engine path `D:/Game/EPIC/UE_5.8` is a path on this machine; adjust it to yours. The commands below are written for Git Bash on Windows (they use `$(pwd -W)` and forward slashes).

## 1. Environment setup

| Tool | Requirement | Notes |
|---|---|---|
| Unreal Engine | 5.8 (5.8.4 on this machine: `Engine/Build/Build.version` says `5.8.4`, changelist 58902052; see also [VERSION.md](../engine-reference/unreal/VERSION.md)) | The `EngineAssociation` in the `.uproject` is a GUID registered on this machine; on another machine you will be asked to choose an engine version. C++20 is the minimum for 5.8 |
| Visual Studio | With the "Game development with C++" workload | Compiles the C++; the author reads code in Rider |
| Git + Git LFS | n/a | `.uasset`, `.umap`, `.png`, `.fbx`, `.wav`, `.mp3`, `.ogg`, `.mp4` and a few more go through LFS ([.gitattributes](../../.gitattributes)); `.webp` and `.md` do not |
| Git Bash, jq | n/a | Used by Claude Code's hook scripts and by the commands on this page; not needed just to run the game from the editor |

## 2. Getting and opening the project

```bash
git clone https://github.com/Huangyu-Shen/2026-epic-gamejam.git
cd 2026-epic-gamejam
git lfs pull
```

Open `Game_01 5.8/Game_01.uproject` with UE 5.8. The directory name carries "5.8" because the original `Game_01/` project was tied to an engine registration that no longer exists on this machine; `Game_01 5.8/` is the only project in use, and the repository no longer contains `Game_01/`. The `.sln` files in that folder are generated (ignored by git); regenerate them with the `.uproject` context menu if Visual Studio or Rider asks.

The editor opens `L_Camp` by default (`EditorStartupMap`). The project has two maps: `L_Camp` (game mode `ACampGameMode`) and `L_Greybox` (game mode `ADirectorGameMode`); `Content/Maps/` also holds an unrelated `Test_Map_01_HLOD0_Instancing.uasset` that no code references.

## 3. Building

### 3.1 Command line (the editor must be closed)

```bash
"D:/Game/EPIC/UE_5.8/Engine/Build/BatchFiles/Build.bat" Game_01Editor Win64 Development \
  "-Project=$(pwd -W)/Game_01 5.8/Game_01.uproject" -WaitMutex
```

Run it from the repository root. On success it prints `Result: Succeeded`. For a game (non-editor) build use the target `Game_01` instead of `Game_01Editor`; the two targets are in `Source/Game_01.Target.cs` and `Source/Game_01Editor.Target.cs`.

### 3.2 In the editor

You can use the editor's Build, but **do not use Live Coding hot reload**: it holds the UBT lock, so later command-line builds fail; hot reload also leaves the state abnormal. After changing C++, close the editor, build from the command line, then reopen it.

### 3.3 Common build errors

| Symptom | Cause and fix |
|---|---|
| `C2084 function already has a body`, pointing at a helper function in an anonymous namespace | The Unity build merges several `.cpp` files, and two files have a helper function with the same name. Give the functions distinct names |
| `error C2664 ... cannot convert TObjectPtr<UWidget>* to UWidget**` | The address of a `TObjectPtr` member was passed to a parameter that requires a raw pointer; take it into a local raw pointer first, then assign |
| Opening the editor crashes in some `...Subsystem::Initialize` | The editor preview world has no `GameInstance`, see [known-issues.md](known-issues.md#4-architecture-and-robustness) |

## 4. Running

### 4.1 In the editor (PIE)

1. The editor opens `L_Camp` automatically; click Play.
2. In the camp's left panel, upgrade buildings and pick cards (the STORAGE list below them shows the upgrade options unlocked so far), then click **START RUN**.
3. On the run map: loot, fight, level up, stand in the extraction zone; on the settlement page click **Return to Camp**.

To test the run map directly, open `Content/Maps/L_Greybox.umap` and click Play; there are no camp bonuses applied beyond what the saved camp holds, and the cards are empty. Each PIE session creates a new GameInstance, which loads the save slot again (see [data-and-storage.md](data-and-storage.md#1-save-game)): PIE uses and changes your real `CampSave` unless you start the editor with `-SaveSlot=Name`.

### 4.2 Running from the command line (standalone window)

```bash
"D:/Game/EPIC/UE_5.8/Engine/Binaries/Win64/UnrealEditor.exe" "Game_01 5.8/Game_01.uproject" -game -windowed -ResX=1280 -ResY=720
```

This runs the editor executable in game mode and needs the editor build (`Game_01Editor`) to be compiled. The log goes to `Game_01 5.8/Saved/Logs/Game_01.log`.

**Git Bash path conversion**: when you pass a map name such as `/Game/Maps/L_Greybox` (all probe commands below do), Git Bash rewrites it into a Windows path. Put `MSYS_NO_PATHCONV=1` in front of the command, as the recorded probe runs did: `MSYS_NO_PATHCONV=1 "D:/Game/EPIC/UE_5.8/Engine/Binaries/Win64/UnrealEditor.exe" ...`. This is not shown in every block below to keep them short.

### 4.3 The commands in `project.yaml` are out of date

The `commands` in [project.yaml](../../project.yaml) (build, test, run, smoke) still point at the warm-up project `Game_Jam_01/` (the build command also uses `AutomationTool.exe BuildCookRun` with no staging or packaging options), and `engine.language: "Blueprint"` is wrong for this C++ project; see [known-issues.md](known-issues.md). The commands on this page were written for `Game_01 5.8/`.

## 5. Game controls

All keys are bound in code (`BindKey` and `IsInputKeyDown`), so they cannot be remapped without editing source. The run HUD prints `WASD move   Space dodge   E search   1-3 cut an option`.

| Scene | Input | Behavior |
|---|---|---|
| Run | W A S D | Move (read each tick with `IsInputKeyDown`; ignored while the run is not active, the hero is dead, or the level-up panel is open) |
| Run | Space | Dodge (dash, invulnerability, cooldown) |
| Run | E | Loot the nearest unsearched container within interact range |
| Run | 1 to 3 | Cut one of the three level-up options while the level-up panel is shown (lost film reels cannot be used) |
| Run | K | Debug: the hero dies immediately (also available in packaged builds) |
| Run | Mouse | Click level-up panel buttons, click Return to Camp on the settlement page |
| Camp | Left mouse button on a 3D building | Upgrade that building (a toast shows the result or the reason it failed) |
| Camp | Click the left panel with the mouse | Upgrade, add or remove film cards, START RUN |

The `N` key (new run) exists only in the unused stand-in code ([dependencies.md](dependencies.md#5-things-deliberately-not-done)). The camp has no walkable character.

## 6. Performance probe `-PerfProbe`

```bash
MSYS_NO_PATHCONV=1 "D:/Game/EPIC/UE_5.8/Engine/Binaries/Win64/UnrealEditor.exe" "Game_01 5.8/Game_01.uproject" /Game/Maps/L_Greybox -game -PerfProbe
```

The probe lives in `ADirectorGameMode::BeginPlay` and does nothing without its flag. In the run-map game mode: after a 3-second delay (`FPerfProbeTuning::StartDelaySeconds`) the hero goes into god mode, weapons are turned off, the spawn scheduler is turned off, `SpawnHorde(150, true)` is called (`EnemyCount`), and it samples frame times for 10 seconds (`SampleSeconds`), writes one line to `LogEnemies` and exits:

```text
LogEnemies: Display: PERFPROBE enemies=152 avgFPS=132.3 minFPS=50.0 samples=1324
```

The line goes to `Game_01 5.8/Saved/Logs/Game_01.log`. The run map (`L_Greybox`) must be specified, because the default startup map is the camp, whose game mode is `ACampGameMode`; none of the three run-map probes (`-PerfProbe`, `-OfferProbe`, `-ReelProbe`) would start there (their flags are read only by `ADirectorGameMode`, which only `L_Greybox` uses).

Story 003 measurement record (see the [evidence document](../../production/qa/evidence/story-003-combat-enemies-map-evidence.md) for the original): the line above (1080p standalone window). The average is above 60 and the worst single frame, 50, is below 60; the user accepted this. This is the result from story 003; systems such as progression and UI were added afterwards and it has not been re-measured.

### Level-up probe `-OfferProbe`

```bash
MSYS_NO_PATHCONV=1 "D:/Game/EPIC/UE_5.8/Engine/Binaries/Win64/UnrealEditor.exe" "Game_01 5.8/Game_01.uproject" /Game/Maps/L_Greybox -game -windowed -ResX=1280 -ResY=720 -OfferProbe
```

Run-map game mode (`ADirectorGameMode::StepOfferProbe`): after 3 seconds it makes the hero invulnerable, turns the spawn scheduler off and forces a level-up, then runs one step every 1.5 seconds. It cuts the first offered option that is not the Chain for 6 level-ups (so a Chain on offer is always kept), logs `OFFERPROBE <label> level=... pool=... cutsLeft=... cutSoFar=... attackSpeed=... magnetRadius=... loadout=[...]` lines (plus one `OFFERPROBE   option: ...` line per offered option; `pool` is the number of options available, `attackSpeed` and `magnetRadius` show the Haste and Magnet results) to `LogEnemies`, saves screenshots, and exits by itself (a slow exit is the derived data cache flushing). Screenshots: `Game_01 5.8/Saved/OfferProbe/1-first-offer.png`, `2-loadout-after-cuts.png`, `3-late-offer.png`.

After the late offer the probe resolves it, spawns a 15-enemy horde (`SpawnHorde(15, true)`) and waits up to eight steps for the first Chain attack that hits two or more enemies. In the frame of that attack it saves `4-chain-fire.png` and logs `OFFERPROBE chain fired: hit N enemies in one attack`; the last line is `OFFERPROBE chain: owned=<0|1> multiHitShot=<0|1>`. The Chain only appears in a slot whose camp has the `Chain` option unlocked (use `-SaveSlot=Name` with a slot that has it, for example one written by a `-ReelProbe -ReelCount=3` extraction). The probe does not end the run, so it does not write the save; but a slot with a NEW marker has it taken by the first offer in memory (the file is not rewritten). The three-screenshot flow was verified in the editor and in a packaged build on 2026-11-13 (evidence in `production/qa/evidence/v2-cut-one-keep-two/` and `v2-packaged-build/`); the Chain and horde additions have their evidence in `production/qa/evidence/story-009-unlockable-upgrades/`, which was not opened while writing this page.

### Lost-reel probe `-ReelProbe` and `-ReelProbe=die`

```bash
MSYS_NO_PATHCONV=1 "D:/Game/EPIC/UE_5.8/Engine/Binaries/Win64/UnrealEditor.exe" "Game_01 5.8/Game_01.uproject" /Game/Maps/L_Greybox -game -ReelProbe -SaveSlot=ReelProbe
```

Run-map game mode (`ADirectorGameMode::StepReelProbe`, one step every 1.5 seconds after a 3-second delay; it reuses the `OfferProbe` timing settings): the hero goes into god mode and spawning is turned off, 25 Iron and reels go into the bag (picking a reel up raises heat), then the run ends by extraction, or by death with `-ReelProbe=die`. `-ReelCount=N` (clamped to 1 through 3, default 1) sets how many reel types are put in: the first N of Fast Cut (`ERelicType::Swiftstep`), Wide Shot (`Warbrand`) and Chain Scene (`Aegis`). Three reels fit the bag with the 25 Iron (3 + 3 = 6 slots with the default 6-slot bag). Screenshots go to `Game_01 5.8/Saved/ReelProbe/`: `1-reel-in-bag.png`, then `2-result-extracted.png` or `3-result-died.png`. `REELPROBE` lines in `LogEnemies` show `reelsInBag`, `heat`, `unlocked` (the unlocked options), `anotherReelCanDrop` and `outcome` (`ERunOutcome` as a number: 0 None, 1 Extracted, 2 Died) before and after; for example:

```text
LogEnemies: Display: REELPROBE reel-in-bag reelsInBag=1 heat=15.0 unlocked=[] anotherReelCanDrop=0 outcome=0
LogEnemies: Display: REELPROBE after-extraction reelsInBag=0 heat=0.0 unlocked=[Haste ] anotherReelCanDrop=0 outcome=1
```

After extraction the unlocked list holds `Haste` (with `-ReelCount=3`: `Haste`, `Magnet` and `Chain`); after death it is unchanged and the reels are lost. The run is settled for real and saved, so add `-SaveSlot=Name` (see [data-and-storage.md](data-and-storage.md#1-save-game)) to keep the probe away from the real save: a slot `Name` becomes `Saved/SaveGames/Name.sav`. A fresh slot name starts with an empty camp; to repeat a probe, delete that `.sav` first.

### Camp screenshot `-CampShot`

```bash
"D:/Game/EPIC/UE_5.8/Engine/Binaries/Win64/UnrealEditor.exe" "Game_01 5.8/Game_01.uproject" -game -CampShot
```

Camp game mode (`ACampGameMode::TakeCampShot`): after 4 seconds it logs one `CAMPSHOT iron=... unlocked=[...]` line (category `LogTemp`, not `LogEnemies`), saves `Game_01 5.8/Saved/ReelProbe/camp-storage.png` (the camp panel with the STORAGE list; note the folder is `ReelProbe`) and exits 1.5 seconds later. It starts on the default camp map and reads the current save slot, so combine it with the same `-SaveSlot=Name` as the reel probe to see the unlocked option. It makes no change, so it writes nothing to the save.

### Probes in a packaged build

The packaged executable accepts the same flags (with `-SaveSlot=Name`). A packaged build writes its log, saves and probe screenshots under `<package>/Windows/Game_01/Saved/` (`Logs/Game_01.log`, `SaveGames/`, `ReelProbe/`, `OfferProbe/`), not under the project's `Saved/`. `production/qa/evidence/story-008-lost-film-reels/packaged/` holds a recorded `-ReelProbe` run against the 2026-11-14 package: `LOOTCFG source=table` and the extraction lines above.

## 7. Testing

**The project has 12 automated tests**, all `IMPLEMENT_SIMPLE_AUTOMATION_TEST` tests under `Source/Game_01/Private/Tests/`, all flagged `EditorContext | ProductFilter` (they run in the editor, not in a packaged game):

| File | Test name |
|---|---|
| `StarTuningTest.cpp` (4) | `Game_01.Progression.StarsFollowCopyThresholds`, `Game_01.Progression.CopiesToNextStarCountsDown`, `Game_01.Progression.StarMultipliersGrowWithStars`, `Game_01.Progression.PermanentCutsKeepTheMinimumPool` |
| `ReelUnlockTest.cpp` (5) | `Game_01.Loot.UnlockedSetAddsOnceAndSkipsNone`, `Game_01.Loot.ReelsMapToTheirOptions`, `Game_01.Loot.OnlyUsefulReelsDrop`, `Game_01.Loot.PickRandomSkipsUselessReels`, `Game_01.Loot.V1SaveRelicsBecomeUnlockedOptions` |
| `UnlockableOptionsTest.cpp` (3) | `Game_01.Progression.LockedOptionIsNeverOfferedUntilUnlocked`, `Game_01.Progression.PoolAndPermanentCutsGrowWithEachUnlock`, `Game_01.Progression.AnUnlockedIdThatMatchesNoOptionChangesNothing` |

They need no world: the star rules, the unlock rule (`FOptionUnlock::IsAvailable`) and the reel-to-option mapping are pure functions. They cover the star thresholds and multipliers, the permanent-cut rule, the unlocked set, the reel drop rules, the v1 save migration (including the `SaveVersion` default), and the unlock rule (a locked option is unavailable until its id is unlocked, the pool grows from 7 to 10 with permanent cuts from 3 to 6, an unknown id changes nothing). `UnlockableOptionsTest.cpp` tests the rule on a table of ten options written inside the test, not the real `URunProgressionSubsystem`, so it does not catch a wrong `bStartsLocked` flag in the data tables.

Run them from the editor's Session Frontend (Automation tab) by filtering on `Game_01.`, or headless with the command pattern the studio template uses (`Automation RunTests <Project>.`):

```bash
"D:/Game/EPIC/UE_5.8/Engine/Binaries/Win64/UnrealEditor-Cmd.exe" "$(pwd -W)/Game_01 5.8/Game_01.uproject" \
  -ExecCmds="Automation RunTests Game_01.; Quit" -unattended -nullrhi -stdout -FullStdOutLogOutput
```

The progress notes in `production/session-state/active.md` recorded "9/9 automation tests pass" on 2026-11-14 (4/4 before story 008); story 009 added 3 and the suite was reported as 12 of 12 passing when that story was finished. The exact command line used was not recorded, and no test was run while writing this page, so the headless command above is the template pattern, not a transcript. `RunTests Game_01.` matches every test whose name contains `Game_01.`. `project.yaml` has no `qa.level` key; the effective level is `minimal` because `modes.rigor: minimal` fixes it, so tests are optional under the studio rules, and "retained screenshots" serve as the evidence for what the player sees.

### Visual and flow evidence

| Story | Location |
|---|---|
| 001 | [production/qa/evidence/story-001-loot-bag-heat/](../../production/qa/evidence/story-001-loot-bag-heat/) |
| 002 | [production/qa/evidence/story-002-extraction-death/](../../production/qa/evidence/story-002-extraction-death/) |
| 003 | [production/qa/evidence/story-003-combat-enemies-map/](../../production/qa/evidence/story-003-combat-enemies-map/), [assessment document](../../production/qa/evidence/story-003-combat-enemies-map-evidence.md) |
| 006 | [story-006-camp-colors.png](../../production/qa/evidence/story-006-camp-colors.png) |
| 007 | [production/qa/evidence/story-007-flow-save-ui/](../../production/qa/evidence/story-007-flow-save-ui/), 11 screenshots and one evidence stamp |
| 008 | [production/qa/evidence/story-008-lost-film-reels/](../../production/qa/evidence/story-008-lost-film-reels/), 4 screenshots, `probe-log.txt` and a `packaged/` run |
| 009 | [production/qa/evidence/story-009-unlockable-upgrades/](../../production/qa/evidence/story-009-unlockable-upgrades/) (a new, untracked folder when this page was updated; contents not listed here) |
| v2 (cut one, keep two) | [production/qa/evidence/v2-cut-one-keep-two/](../../production/qa/evidence/v2-cut-one-keep-two/) |
| v2 packaged build | [production/qa/evidence/v2-packaged-build/](../../production/qa/evidence/v2-packaged-build/) |

## 8. Packaging and release

A Windows Development package builds and runs. It was first made to work on 2026-11-13 (fifth attempt) and rebuilt on 2026-11-14. The command recorded in `production/session-state/active.md`:

```bash
"D:/Game/EPIC/UE_5.8/Engine/Build/BatchFiles/RunUAT.bat" BuildCookRun \
  -project="$(pwd -W)/Game_01 5.8/Game_01.uproject" -noP4 -platform=Win64 -clientconfig=Development \
  -build -cook -stage -pak -archive -archivedirectory="$(pwd -W)/Game_01 5.8/Saved/Packages/test-2026-11-13" \
  -utf8output -unattended
```

It takes about 5 minutes; the output is `<archivedirectory>/Windows/Game_01.exe`. The package folder is about 1.1 GB, of which `Game_01.pdb` is 356 MB (leave it out of a zip). There is no Shipping package: `-clientconfig=Development` is the only configuration tried. The packaged exe was checked with `-OfferProbe` and `-ReelProbe`; the camp -> run -> camp loop, audio, and a like-for-like comparison of data-table values in the package against the editor were not checked in a package (the log line `LOOTCFG source=table` shows the loot tables load).

What makes the package work, both in [DefaultGame.ini](../../Game_01%205.8/Config/DefaultGame.ini) under `[/Script/UnrealEd.ProjectPackagingSettings]`:

- `MapsToCook` lists `L_Camp` and `L_Greybox`. Without it only the default map is cooked and START RUN fails.
- `DirectoriesToAlwaysCook` lists `/Game/Data`. The tuning tables are referenced by soft pointer only, so the cooker would skip them and the game would silently use the C++ defaults.

The first successful package also needed the `[Zen] AutoLaunch=False` lines removed from `DefaultEngine.ini` (they made the cook and stage steps fail); they are gone from the file now.

The submission deadline is 2026-11-16 01:59 (UK time), and the user submits it personally ([PLAN.md](../../PLAN.md)).

Checks before release:

- Restrict the `ModelContextProtocol` and `AllToolsets` plugins to Editor ([dependencies.md](dependencies.md#3-engine-plugins)).
- In Shipping builds the weapons' `DrawDebug*` indicator lines disappear ([known-issues.md](known-issues.md#3-legacy-and-dead-code)).
- The tint problem on heroes and enemies ([known-issues.md](known-issues.md#21-the-hero-enemies-and-xp-gems-are-gray-and-tinting-has-no-effect)).
- Packaged saves live in `<package>/Windows/Game_01/Saved/SaveGames/`, separate from the editor's.

## 9. Unreal MCP (AI collaboration)

1. Enable `ModelContextProtocol` and `AllToolsets` in the editor (already enabled in the `.uproject`).
2. Turn on **Auto Start Server** in the editor preferences (in Editor Preferences, not Project Settings).
3. The server address is `http://127.0.0.1:8000/mcp`, and the root [.mcp.json](../../.mcp.json) points at it.
4. Tool calls: `list_toolsets`, `describe_toolset`, `call_tool`.

MCP is only available while the editor is open; with the editor closed the connection is refused. `Game_01 5.8/.mcp.json` is a copy with the same content. The setting is stored in the ignored `Config/DefaultEditorPerProjectUserSettings.ini`, so each machine sets it again.

## 10. Collaboration workflow

| Item | Convention |
|---|---|
| Branches | Trunk-based development; routine features go on a `feature/...` branch plus a PR (auto-merged in the personal repository) |
| Commits | Conventional Commits (`feat:`, `fix:`, `docs:`, `chore:`), with `Story: ...` in the body; commits only when the user asks |
| Releases | One tag, `jam-start`, on the commit before the UE project existed; no game release has been tagged |
| Stories | 13 stories under [production/epics/directors-cut/](../../production/epics/directors-cut/), with the status in the header of each story file |
| Process | `/dev-story` to implement, `/story-done` to accept (both are skills under `.claude/skills/`) |
| Session state | `production/session-state/active.md` (gitignored, not in the repository) |

## 11. FAQ

| Symptom | Cause and fix |
|---|---|
| The editor opens an untitled template map every time | `EditorStartupMap` was not set; it is now set to the camp |
| Pressing Space after clicking a button "presses" the button | The button kept keyboard focus; `UUiActionButton` calls `ReturnFocusToGame()` after a click, and newly created buttons must use it |
| Command-line build fails saying a file is in use | The editor or Live Coding is still open; close it first |
| A probe starts the camp and nothing happens | The map argument was dropped or rewritten by Git Bash: pass `/Game/Maps/L_Greybox` with `MSYS_NO_PATHCONV=1` (see 4.2) |
| Clicking in camp does nothing | Make sure you clicked a button in the left panel or a 3D building; the camp has no walkable character |
| Camp state is lost after restarting the game | Check whether `Game_01 5.8/Saved/SaveGames/CampSave.sav` exists (a packaged build uses its own `Saved/` folder, and a run started with `-SaveSlot=Name` uses `Name.sav`); cleaning `Saved/` also wipes the save |
| A probe changed my real progress | `-ReelProbe` settles a run for real; pass `-SaveSlot=Name` |
| The packaged game uses default numbers instead of the tables | `/Game/Data` is missing from `DirectoriesToAlwaysCook`; look for `LOOTCFG source=c++ defaults` in the log |
