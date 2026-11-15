# Running, Testing and Releasing

Paths are relative to the repository root. The engine path `D:/Game/EPIC/UE_5.8` is a path on this machine; adjust it to yours. Commands are written for Git Bash on Windows (`$(pwd -W)` and forward slashes). Checked against commit `bfe9e16` (2026-11-15). Commands were read from code, configs and recorded logs; nothing on this page was re-run while writing it, except where a line says so.

## 1. Environment setup

| Tool | Requirement | Notes |
|---|---|---|
| Unreal Engine | 5.8 (5.8.4 on this machine; see [VERSION.md](../engine-reference/unreal/VERSION.md)) | `EngineAssociation` in `Game_01.uproject` is a GUID registered on this machine; elsewhere the editor asks for an engine version. C++20 is the minimum |
| Visual Studio | "Game development with C++" workload | Compiles the C++; the author reads code in Rider |
| Git + Git LFS | n/a | `.uasset`, `.umap`, `.fbx`, `.obj`, `.png`, `.tga`, `.psd`, `.exr`, `.wav`, `.mp3`, `.ogg`, `.mp4` go through LFS ([.gitattributes](../../.gitattributes)) |
| Git Bash, jq | n/a | Used by Claude Code's hook scripts and by the commands here; not needed to run the game from the editor |
| GPU | DirectX 12, shader model 6 | `Config/DefaultEngine.ini`: DX12 default RHI, `PCD3D_SM6`, Lumen GI and reflections, virtual shadow maps, Substrate and ray tracing switched on |

### 1.1 The Fab asset packs are not in git

The repository holds only `Content/Data` (12 tuning tables), `Content/Maps` (`L_Camp`, `L_Greybox`, an unused test map) and `Content/Materials/M_FlatColor`. The 3D models and audio come from Fab packs that sit in `Content/` on the author's machine as untracked folders (the code uses `RPGHeroSquad`, `MonsterForSurvivalGame`, `RPGTinyFantasyForest`, `LowPolyHouse`, `WW2Generic`, `Free_Sounds_Pack`, `PathOfAdventure`; other imported packs are unused). Both maps use only engine assets.

**A fresh clone still builds and plays**, but looks and sounds like a greybox: the hero is a blue cylinder, enemies are red cubes (the boss a bigger purple one), containers are cubes, the extraction point has only its green disc and beam, the camp buildings are orange, green and blue cubes, the floor is the engine template floor, and there is **no sound and no music** (a missing sound is a silent no-op logged once, `AUDIO first request: ... silent`). The editor log will show `CDO Constructor (...): Failed to find /Game/...` errors for each missing pack path; see [known-issues.md](known-issues.md#23-asset-packs-from-fab-are-not-in-git-the-code-reaches-them-by-hard-path). If you import a pack into a running editor, restart the editor: some finders are `static` and keep the failed result.

**Credits and licences** are in the assets table of [docs/devlog/README.md](../devlog/README.md#assets-and-credits) (pack, author, licence on the Fab page, use) and, for the two CC BY packs (music by Kounine, buildings by Winyunq) plus the other authors, at the bottom of the Esc menu in the game (`PauseMenuWidget.cpp:142-146`). Keep those credits when shipping.

## 2. Getting and opening the project

```bash
git clone https://github.com/Huangyu-Shen/2026-epic-gamejam.git
cd 2026-epic-gamejam
git lfs pull
```

Open `Game_01 5.8/Game_01.uproject` with UE 5.8. The folder name carries "5.8" because the original project was tied to an engine registration that no longer exists here; `Game_01 5.8/` is the only game project (the git-ignored `Game_Jam_01/` was a warm-up and is not part of the repository). The `.sln` files are generated and ignored; regenerate them from the `.uproject` context menu if the IDE asks.

`GameDefaultMap` and `EditorStartupMap` are both `L_Camp`. The project has two maps: `L_Camp` (`ACampGameMode`) and `L_Greybox` (`ADirectorGameMode`, the run map); `Content/Maps/` also holds an unreferenced `Test_Map_01_HLOD0_Instancing.uasset`.

## 3. Building

### 3.1 Command line (the editor must be closed)

```bash
"D:/Game/EPIC/UE_5.8/Engine/Build/BatchFiles/Build.bat" Game_01Editor Win64 Development \
  "-Project=$(pwd -W)/Game_01 5.8/Game_01.uproject" -WaitMutex
```

Run it from the repository root; success prints `Result: Succeeded`. For the non-editor target use `Game_01` (targets in `Source/Game_01.Target.cs` and `Source/Game_01Editor.Target.cs`, both `BuildSettingsVersion.V7`, include order 5.8). The module `Game_01` depends on Core, CoreUObject, Engine, InputCore, EnhancedInput, DeveloperSettings, UMG, Slate, SlateCore (`Game_01.Build.cs`).

### 3.2 In the editor

Do not use Live Coding hot reload: it holds the UBT lock so later command-line builds fail, and leaves the state abnormal. After changing C++, close the editor, build from the command line, reopen.

### 3.3 Common build errors

| Symptom | Cause and fix |
|---|---|
| `C2084 function already has a body`, in an anonymous namespace | The Unity build merges several `.cpp` files and two have a helper with the same name; rename one |
| `C2664 ... cannot convert TObjectPtr<UWidget>* to UWidget**` | The address of a `TObjectPtr` member was passed where a raw pointer is needed; copy it to a local raw pointer first |
| The editor crashes in some `...Subsystem::Initialize` | The editor preview world has no `GameInstance`; see [known-issues.md](known-issues.md#4-architecture-and-robustness) |

## 4. Running

### 4.1 In the editor (PIE)

1. The editor opens `L_Camp`; click Play.
2. In the camp's left panel upgrade buildings (Smithy costs stone, Clinic costs wood) and add film cards, then click **START RUN**.
3. On the run map: loot, fight, level up, stand in the extraction zone; on the result page click **Return to Camp**.

To test the run map directly open `Content/Maps/L_Greybox.umap` and play. Each PIE session creates a new GameInstance that loads the save slot again ([data-and-storage.md](data-and-storage.md#1-save-game)): PIE uses and changes your real `CampSave` unless you start the editor with `-SaveSlot=Name`.

### 4.2 Standalone window from the command line

```bash
"D:/Game/EPIC/UE_5.8/Engine/Binaries/Win64/UnrealEditor.exe" "Game_01 5.8/Game_01.uproject" -game -windowed -ResX=1280 -ResY=720
```

Needs the compiled editor build. The log is `Game_01 5.8/Saved/Logs/Game_01.log`.

**Git Bash path conversion**: a map argument such as `/Game/Maps/L_Greybox` is rewritten into a Windows path. Put `MSYS_NO_PATHCONV=1` in front of the command (not repeated in every block below).

### 4.3 The commands in `project.yaml` are out of date

`commands.*` in [project.yaml](../../project.yaml) still point at `Game_Jam_01/` and `engine.language` is `"Blueprint"`; see [known-issues.md](known-issues.md#17-projectyaml-still-points-at-the-warm-up-project). The commands on this page are for `Game_01 5.8/`.

## 5. Game controls

All keys are bound in code (`BindKey`, `IsInputKeyDown`); none can be remapped without editing source.

| Scene | Input | Behavior |
|---|---|---|
| Run | W A S D | Move (read each tick; ignored when the run is not active, the hero is dead, or a level-up offer is open) |
| Run | Space | Dodge: short dash with invulnerability and a cooldown |
| Run | E | Search the nearest unsearched container within 250 cm |
| Run | 1, 2, 3 | Take that option of the level-up offer |
| Run | 4, 5, 6 | Cut that option first (one cut per level-up while permanent cuts remain); the offer stays open |
| Run | Esc | Open or close the pause menu (not while a level-up offer is showing): Resume, Settings (three volume levels, steps of 10 percent, this session only), Return to camp (asks twice, the bag is lost), Quit game; the game is paused while it is open; credits and controls at the bottom |
| Run | K | Debug: the hero dies at once (also in a package) |
| Run | Mouse | Click the level-up buttons; click Return to Camp on the result page |
| Camp | Left mouse button on a 3D building | Upgrade it (a toast shows the result or the reason it failed) |
| Camp | Left panel, MENU button (top right), Esc | Upgrade, add or remove film cards (2 slots), START RUN; the menu has Resume, Settings, Quit game |

The run HUD prints `WASD move   Space dodge   E search   1-3 take an upgrade   4-6 cut one   Esc menu`. The camp has no walkable character. The `N` key exists only in the unused stand-in code.

## 6. Performance probe `-PerfProbe`, and the other test probes

Probes are command-line flags read in `BeginPlay` of the game modes. None does anything without its flag. The flags found in the source (`grep -rn "FParse::Param\|FParse::Value" "Game_01 5.8/Source/Game_01/Private"`):

| Flag | Read in | Map | Purpose |
|---|---|---|---|
| `-PerfProbe` | `DirectorGameMode.cpp:246` | run | 150-enemy frame-rate sample |
| `-OfferProbe` | `:251` | run | Level-up panel, cuts, loadout, weapon visuals |
| `-ReelProbe`, `-ReelProbe=die`, `-ReelCount=N` | `:263-264`, `:385` | run | Lost film reel in the bag, then extract or die |
| `-PaceProbe`, `-PaceSpeed=N`, `-PaceSpawnBase=`, `-PaceSpawnRamp=`, `-PaceGruntHealth=`, `-PaceGruntContact=`, `-PaceHeroHealth=` | `:256`, `:308-317` | run | A bot plays up to 10 minutes; the overrides apply to that run only |
| `-WeaponShowcase` | `:46` | run | Gun and Orbit at 3 copies against 30 grunts, 2 screenshots |
| `-PauseProbe` | `:77` | run | Opens the Esc menu, Settings, volume, the two-step Return to camp |
| `-EdgeProbe` | `:116` | run | Hero at the floor edge with a horde and a boss, logs the farthest enemy |
| `-BossProbe` | `:152` | run | Spawns the boss, cuts options one by one, 3 screenshots |
| `-ZoneShot` | `:202` | run | Screenshots of the extraction point and the bar filling |
| `-CampShot` | `CampGameMode.cpp:93` | camp | Camp panel screenshot |
| `-SaveSlot=Name` | `CampSaveSubsystem.cpp:18` | any | Use `Saved/SaveGames/Name.sav` instead of `CampSave.sav` |

The run-map flags are read only by `ADirectorGameMode`, so they need the map on the command line (the default map is the camp). Screenshots go to `Saved/<ProbeName>/` (`PerfProbe` has none). Most probes exit by themselves.

### `-PerfProbe`

```bash
MSYS_NO_PATHCONV=1 "D:/Game/EPIC/UE_5.8/Engine/Binaries/Win64/UnrealEditor.exe" "Game_01 5.8/Game_01.uproject" /Game/Maps/L_Greybox -game -PerfProbe
```

After 3 s (`FPerfProbeTuning::StartDelaySeconds`) the hero gets god mode, weapons and the spawn scheduler are turned off, `SpawnHorde(150, true)` runs, frame times are sampled for 10 s and one line goes to `LogEnemies` before the exit:

```text
LogEnemies: Display: PERFPROBE enemies=152 avgFPS=132.3 minFPS=50.0 samples=1324
```

That line is the story 003 measurement (1080p window; user accepted it). It predates progression, UI, audio and the models. The dev log (day 3, entry 19) reports about 130 FPS on warm runs with the Fab models and weapons off, packaged; the 2026-11-14 package run is in `production/qa/evidence/story-010-number-retune/packaged-perf-and-pace.txt`. This page did not re-measure.

### `-OfferProbe`

```bash
MSYS_NO_PATHCONV=1 "D:/Game/EPIC/UE_5.8/Engine/Binaries/Win64/UnrealEditor.exe" "Game_01 5.8/Game_01.uproject" /Game/Maps/L_Greybox -game -windowed -ResX=1280 -ResY=720 -OfferProbe
```

(`StepOfferProbe`.) After 3 s: god mode, scheduler off, forced level-up; then one step per 1.5 s. It plays `LevelUps` = 6 level-ups; at each it takes the offer whose text contains "Chain" (else the first) after cutting another option when a cut is allowed. It logs `OFFERPROBE <first-offer|after-level-ups|late-offer> level=... pool=... cutsLeft=... cutSoFar=... attackSpeed=... magnetRadius=... loadout=[...]` plus one `OFFERPROBE   option: ...` line per option, and saves `Saved/OfferProbe/1-first-offer.png`, `2-loadout-after-cuts.png`, `3-late-offer.png`. Then it spawns a 15-enemy horde and photographs the first attack that hits something for each owned weapon (`fx-<Weapon>.png`), waits up to eight steps for a Chain attack that hits two or more (`4-chain-fire.png`), logs `OFFERPROBE chain: owned=<0|1> multiHitShot=<0|1>` and exits. The Chain only appears in a slot that has it unlocked; use a `-SaveSlot` that has it (for example from a `-ReelProbe -ReelCount=3` extraction). A NEW marker is taken from the in-memory stock by the first offer; the file is not rewritten.

### `-ReelProbe`

```bash
MSYS_NO_PATHCONV=1 "D:/Game/EPIC/UE_5.8/Engine/Binaries/Win64/UnrealEditor.exe" "Game_01 5.8/Game_01.uproject" /Game/Maps/L_Greybox -game -ReelProbe -SaveSlot=ReelProbe
```

(`StepReelProbe`; same delay and step as the offer probe.) God mode, scheduler off, 25 wood (`TryAddIron`) and N reels go into the bag (`-ReelCount=N`, clamped 1 to 3, default 1: Fast Cut / `Swiftstep`, Wide Shot / `Warbrand`, Chain Scene / `Aegis`); then the run ends by extraction, or by death with `-ReelProbe=die`. Screenshots: `Saved/ReelProbe/1-reel-in-bag.png`, then `2-result-extracted.png` or `3-result-died.png`. `REELPROBE` lines (`LogEnemies`) show `reelsInBag`, `heat`, `unlocked`, `anotherReelCanDrop`, `outcome` (0 None, 1 Extracted, 2 Died):

```text
LogEnemies: Display: REELPROBE reel-in-bag reelsInBag=1 heat=15.0 unlocked=[] anotherReelCanDrop=0 outcome=0
LogEnemies: Display: REELPROBE after-extraction reelsInBag=0 heat=0.0 unlocked=[Haste ] anotherReelCanDrop=0 outcome=1
```

After extraction the unlocked list holds `Haste` (with `-ReelCount=3`: `Haste`, `Magnet`, `Chain`); after death nothing changes. The run is settled for real and saved, so always pass `-SaveSlot=Name`; delete `Saved/SaveGames/Name.sav` to repeat on a fresh camp.

### `-PaceProbe` and the showcase probes

`-PaceProbe` makes the hero walk in a circle, always take the first offer and never cut; it runs at `-PaceSpeed=N` times speed (clamped 1 to 8, default 4, via global time dilation), logs `TUNING` lines once and a `PACEPROBE t=..s level=.. hp=.. alive=.. spawnRate=..` line every 30 game seconds, ends on death, extraction or 600 s with `PACEPROBE END outcome=...`. Its evidence is in `production/qa/evidence/story-010-number-retune/`. `-WeaponShowcase`, `-PauseProbe`, `-EdgeProbe`, `-BossProbe` and `-ZoneShot` save screenshots to `Saved/Showcase/`, `PauseProbe/`, `EdgeProbe/`, `BossProbe/`, `ZoneShot/` and exit after 5 to 11 s; their evidence is under `production/qa/evidence/` (`story-011-art-audio-feel`, `story-012-cut-knight`, `pause-menu`). The pause probe has to drive later steps from the engine ticker because the open menu pauses the world and its timers.

### `-CampShot`

```bash
"D:/Game/EPIC/UE_5.8/Engine/Binaries/Win64/UnrealEditor.exe" "Game_01 5.8/Game_01.uproject" -game -CampShot
```

After 4 s it logs `CAMPSHOT iron=... unlocked=[...]` (`LogTemp`; `iron` is the wood stock), saves `Saved/ReelProbe/camp-storage.png` (the folder name is `ReelProbe`) and exits 1.5 s later. Writes nothing to the save.

### Probes in a package

The packaged exe accepts the same flags. A package writes log, saves and screenshots under `<package>/Windows/Game_01/Saved/`. The probes are not stripped from any build configuration ([known-issues.md](known-issues.md#3-legacy-and-dead-code)).

## 7. Testing

**12 automated tests**, all `IMPLEMENT_SIMPLE_AUTOMATION_TEST`, flagged `EditorContext | ProductFilter` (editor only, not a package), in `Source/Game_01/Private/Tests/`:

| File | Test name |
|---|---|
| `StarTuningTest.cpp` (4) | `Game_01.Progression.StarsFollowCopyThresholds`, `CopiesToNextStarCountsDown`, `StarMultipliersGrowWithStars`, `PermanentCutsKeepTheMinimumPool` |
| `ReelUnlockTest.cpp` (5) | `Game_01.Loot.UnlockedSetAddsOnceAndSkipsNone`, `ReelsMapToTheirOptions`, `OnlyUsefulReelsDrop`, `PickRandomSkipsUselessReels`, `V1SaveRelicsBecomeUnlockedOptions` |
| `UnlockableOptionsTest.cpp` (3) | `Game_01.Progression.LockedOptionIsNeverOfferedUntilUnlocked`, `PoolAndPermanentCutsGrowWithEachUnlock`, `AnUnlockedIdThatMatchesNoOptionChangesNothing` |

They need no world: the star rules, the unlock rule, the reel-to-option mapping and the v1 migration helper are pure functions. `UnlockableOptionsTest.cpp` checks the rule on a 16-option table written inside the test (13 available at the start, 16 with all three unlocked, permanent cuts 9 to 12), not on the real `URunProgressionSubsystem`, so it does not catch a wrong `bStartsLocked` in the data.

```bash
"D:/Game/EPIC/UE_5.8/Engine/Binaries/Win64/UnrealEditor-Cmd.exe" "$(pwd -W)/Game_01 5.8/Game_01.uproject" \
  -ExecCmds="Automation RunTests Game_01; Quit" -unattended -nullrhi
```

Reported by the maintainer on 2026-11-15: **12 of 12 pass** with this command (not re-run while writing this page). The filter `Game_01` matches every test whose name contains it; add `-stdout -FullStdOutLogOutput` to see the log. Or use the editor's Session Frontend (Automation tab), filter `Game_01.`. On a clone without the packs the run also prints the missing-asset errors from 1.1; they do not fail the tests.

`project.yaml` has no `qa` key (only `modes.rigor: minimal`). Per `CLAUDE.md`, minimal waives the tests but not the screenshot rule: a player-visible change is launched and a screenshot kept in `production/qa/evidence/`.

### Visual and flow evidence

| Story | Location |
|---|---|
| 001 | [story-001-loot-bag-heat/](../../production/qa/evidence/story-001-loot-bag-heat/) |
| 002 | [story-002-extraction-death/](../../production/qa/evidence/story-002-extraction-death/) |
| 003 | [story-003-combat-enemies-map/](../../production/qa/evidence/story-003-combat-enemies-map/), [assessment](../../production/qa/evidence/story-003-combat-enemies-map-evidence.md) |
| 006 | [story-006-camp-colors.png](../../production/qa/evidence/story-006-camp-colors.png) |
| 007 | [story-007-flow-save-ui/](../../production/qa/evidence/story-007-flow-save-ui/) |
| 008 | [story-008-lost-film-reels/](../../production/qa/evidence/story-008-lost-film-reels/) (with a `packaged/` run) |
| 009 | [story-009-unlockable-upgrades/](../../production/qa/evidence/story-009-unlockable-upgrades/) |
| 010 | [story-010-number-retune/](../../production/qa/evidence/story-010-number-retune/), [smoke-2026-11-14.md](../../production/qa/smoke-2026-11-14.md) |
| 011 | [story-011-art-audio-feel/](../../production/qa/evidence/story-011-art-audio-feel/) (19 screenshots, `packaged-notes.txt`) |
| 012 | [story-012-cut-knight/](../../production/qa/evidence/story-012-cut-knight/) |
| Esc menu | [pause-menu/](../../production/qa/evidence/pause-menu/); `abandon-button/` (untracked, the replaced button) |
| v2 | [v2-cut-one-keep-two/](../../production/qa/evidence/v2-cut-one-keep-two/), [v2-packaged-build/](../../production/qa/evidence/v2-packaged-build/) |

## 8. Packaging and release

A Windows Development package builds and runs (first made to work 2026-11-13, fifth attempt; later packages up to `Saved/Packages/story-012e`). The recorded command (`production/session-logs/session-log.md`):

```bash
"D:/Game/EPIC/UE_5.8/Engine/Build/BatchFiles/RunUAT.bat" BuildCookRun \
  -project="$(pwd -W)/Game_01 5.8/Game_01.uproject" -noP4 -platform=Win64 -clientconfig=Development \
  -build -cook -stage -pak -archive -archivedirectory="$(pwd -W)/Game_01 5.8/Saved/Packages/<name>" \
  -utf8output -unattended
```

Close the editor first. It takes about 5 minutes; the output is `<archivedirectory>/Windows/Game_01.exe`. A package folder is about 1.1 GB: `Game_01.exe` 337 MB, `Game_01.pdb` 376 MB (358 MiB), the pak files about 279 MB. Without the `.pdb` it is about 0.7 GB, so leave it out of a zip. `Saved/` is git-ignored. Only `-clientconfig=Development` has been tried; there is no Shipping package. The packages hold the Fab models and sounds because the packs were in `Content/` when they were cooked.

What makes the package work, in [DefaultGame.ini](../../Game_01%205.8/Config/DefaultGame.ini), `[/Script/UnrealEd.ProjectPackagingSettings]`:

- `MapsToCook`: `L_Camp` and `L_Greybox` (otherwise START RUN fails).
- `DirectoriesToAlwaysCook`: `/Game/Data` (the tuning tables are soft references only; without it the game silently uses C++ defaults, look for `LOOTCFG source=c++ defaults`), plus `/Game/Free_Sounds_Pack/cue` and `/Game/PathOfAdventure/Loop/CUE` (the sound cues are soft references too).

The `[Zen] AutoLaunch=False` lines that once broke cook and stage were removed from `DefaultEngine.ini`. Packaged probe runs (`-OfferProbe`, `-ReelProbe`) and the author's playtests of the package are on record (dev log day 3); a package built from a clone without the packs was not tried.

The submission deadline is 2026-11-16 01:59 UK time and the user submits it personally ([PLAN.md](../../PLAN.md)).

Checks before release:

- Restrict `ModelContextProtocol`, `AllToolsets` and `Terminal` to Editor ([dependencies.md](dependencies.md#3-engine-plugins)).
- Decide about the `K` key, the probes and `DebugGrantWeapon` / `DebugCut`, which are in every build.
- Keep the CC BY credits (Esc menu) and the pack list in the dev log.
- Packaged saves are in `<package>/Windows/Game_01/Saved/SaveGames/`, separate from the editor's.

## 9. Unreal MCP (AI collaboration)

1. Enable `ModelContextProtocol` and `AllToolsets` (already enabled in the `.uproject`).
2. Turn on **Auto Start Server** in Editor Preferences (not Project Settings).
3. The server address is `http://127.0.0.1:8000/mcp`; the root [.mcp.json](../../.mcp.json) points at it (`Game_01 5.8/.mcp.json` is an identical copy).
4. Tool calls: `list_toolsets`, `describe_toolset`, `call_tool`.

MCP is only available while the editor is open; with the editor closed the connection is refused. The setting is stored in `Config/DefaultEditorPerProjectUserSettings.ini`, which is git-ignored, so each machine sets it again.

## 10. Collaboration workflow

| Item | Convention |
|---|---|
| Branches | Trunk-based on `main`; features go through a branch and a pull request (the latest commits are `... (#11)`) |
| Commits | Conventional Commits (`feat:`, `fix:`, `docs:`, `chore:`), story id in the body; **commit, push and tag only when the user asks** ([CLAUDE.md](../../CLAUDE.md)) |
| Releases | One tag, `jam-start`, on the commit before the UE project existed; no game release is tagged |
| Stories | 13 under [production/epics/directors-cut/](../../production/epics/directors-cut/); the status is in each file's header (001 to 010 and 012 Complete, 011 In Progress, 013 Ready; the table in `EPIC.md` is out of date) |
| Process | `/dev-story` to implement, `/story-done` to accept (skills under `.claude/skills/`) |
| Session state | `production/session-state/active.md` (git-ignored) |

## 11. FAQ

| Symptom | Cause and fix |
|---|---|
| The editor opens an untitled template map | `EditorStartupMap` is not set; it is `L_Camp` in `DefaultEngine.ini` |
| Pressing Space after clicking a button "presses" it | The button kept focus; `UUiActionButton` calls `ReturnFocusToGame()` after a click, new buttons must too |
| The command-line build says a file is in use | The editor or Live Coding is still open; close it |
| A probe starts the camp and nothing happens | The map argument was dropped or rewritten by Git Bash: pass `/Game/Maps/L_Greybox` with `MSYS_NO_PATHCONV=1` |
| Clicking in the camp does nothing | Click a button in the left panel or a 3D building; the camp has no walkable character |
| Camp state is lost after restarting | Check `Game_01 5.8/Saved/SaveGames/CampSave.sav` (a package has its own `Saved/`; `-SaveSlot=Name` uses `Name.sav`); cleaning `Saved/` wipes the save |
| A probe changed my real progress | `-ReelProbe` settles a run for real; pass `-SaveSlot=Name` |
| The game is grey cubes and silent | The Fab packs are not in `Content/` (1.1); expected on a fresh clone |
| The packaged game uses default numbers | `/Game/Data` is missing from `DirectoriesToAlwaysCook`; look for `LOOTCFG source=c++ defaults` |
| The game freezes while the Esc menu is open | By design: it pauses the world; Esc or Resume continues |
