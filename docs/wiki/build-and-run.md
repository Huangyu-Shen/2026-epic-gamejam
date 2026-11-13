# Running, Testing and Releasing

Paths are relative to the repository root. The engine path `D:/Game/EPIC/UE_5.8` is a path on this machine; adjust it to yours.

## 1. Environment setup

| Tool | Requirement | Notes |
|---|---|---|
| Unreal Engine | 5.8 (5.8.4 on this machine, see [VERSION.md](../engine-reference/unreal/VERSION.md)) | The `EngineAssociation` in the `.uproject` is a GUID registered on this machine; on another machine you will be asked to choose an engine version |
| Visual Studio | With the "Game development with C++" workload | Compiles the C++; the author reads code in Rider |
| Git + Git LFS | — | `.uasset`, `.umap`, `.png` and so on go through LFS ([.gitattributes](../../.gitattributes)) |
| Git Bash, jq | — | Used only by Claude Code's hook scripts; not needed just to run the game |

## 2. Getting and opening the project

```bash
git clone https://github.com/Huangyu-Shen/2026-epic-gamejam.git
cd 2026-epic-gamejam
git lfs pull
```

Open `Game_01 5.8/Game_01.uproject` with UE 5.8. The directory name carries "5.8" because the engine GUID of the original `Game_01/` pointed at an engine that no longer exists on this machine; the copy has a changed GUID and is the only project currently in use. The repository no longer contains `Game_01/` (see `git ls-files`).

The editor opens `L_Camp` by default (`EditorStartupMap`).

## 3. Building

### 3.1 Command line (the editor must be closed)

```bash
"D:/Game/EPIC/UE_5.8/Engine/Build/BatchFiles/Build.bat" Game_01Editor Win64 Development \
  "-Project=$(pwd -W)/Game_01 5.8/Game_01.uproject" -WaitMutex
```

On success it prints `Result: Succeeded`. The Claude sessions in this repository run this command through a Windows scheduled task named `GameBuild`, because the sandbox cannot write to `%LOCALAPPDATA%\UnrealBuildTool`.

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
2. In the camp's left panel, upgrade, dismantle and pick cards, then click **START RUN**.
3. On the run map: loot, fight, level up, stand in the extraction zone; on the settlement page click **Return to Camp**.

To test the run map directly, open `Content/Maps/L_Greybox.umap` and click Play; there are no camp bonuses and the cards are empty.

### 4.2 Running from the command line (standalone window)

```bash
"D:/Game/EPIC/UE_5.8/Engine/Binaries/Win64/UnrealEditor.exe" "Game_01 5.8/Game_01.uproject" -game -windowed -ResX=1280 -ResY=720
```

### 4.3 The commands in `project.yaml` are out of date

The `commands` in [project.yaml](../../project.yaml) (build, test, run, smoke) and `engine.language: "Blueprint"` still point at the warm-up project `Game_Jam_01/`, see [known-issues.md](known-issues.md). The commands above were written for `Game_01 5.8/`.

## 5. Game controls

| Scene | Input | Behavior |
|---|---|---|
| Run | W A S D | Move |
| Run | Space | Dodge (dash, invulnerability, cooldown) |
| Run | E | Loot the nearest container |
| Run | 1 to 6 | Use the relic in the matching bag slot (1 to 3 pick a level-up option while the level-up panel is shown) |
| Run | K | Debug: the hero dies immediately |
| Run | Mouse | Click level-up panel buttons, click Return to Camp on the settlement page |
| Camp | Click a 3D building with the mouse | Upgrade |
| Camp | Click the left panel with the mouse | Upgrade, dismantle relics, add or remove film cards, START RUN |

## 6. Performance probe `-PerfProbe`

```bash
"D:/Game/EPIC/UE_5.8/Engine/Binaries/Win64/UnrealEditor.exe" "Game_01 5.8/Game_01.uproject" /Game/Maps/L_Greybox -game -PerfProbe
```

In the run-map game mode: after a 3-second delay the hero goes into god mode, weapons are turned off, the spawn scheduler is turned off, `SpawnHorde(150, true)` is called, and it samples for 10 seconds, writes one `PERFPROBE ...` line to `LogEnemies` (the file is in `Game_01 5.8/Saved/Logs/`) and then exits. Without the argument it does not run.

The run map (`L_Greybox`) must be specified, because the default startup map is the camp, which has no `ADirectorGameMode`, so the probe would not start. This is inferred from reading the code and has not been verified with an actual command-line run.

Story 003 measurement record (see the [evidence document](../../production/qa/evidence/story-003-combat-enemies-map-evidence.md) for the original): `enemies=152 avgFPS=132.3 minFPS=50.0 samples=1324` (1080p standalone window). The average is above 60 and the worst single frame, 50, is below 60; the user accepted this. This is the result from story 003; systems such as progression and UI were added afterwards and it has not been re-measured.

## 7. Testing

**The project has no automated tests**: `Source/` contains no `IMPLEMENT_SIMPLE_AUTOMATION_TEST` or `DEFINE_SPEC`, and there is no `tests/` directory. No tests were run for this document. `qa.level` in `project.yaml` is `minimal`, so tests are exempt under the rules, and "retained screenshots" serve as evidence instead.

### Visual and flow evidence

| Story | Location |
|---|---|
| 001 | [production/qa/evidence/story-001-loot-bag-heat/](../../production/qa/evidence/story-001-loot-bag-heat/) |
| 002 | [production/qa/evidence/story-002-extraction-death/](../../production/qa/evidence/story-002-extraction-death/) |
| 003 | [production/qa/evidence/story-003-combat-enemies-map/](../../production/qa/evidence/story-003-combat-enemies-map/), [assessment document](../../production/qa/evidence/story-003-combat-enemies-map-evidence.md) |
| 006 | [story-006-camp-colors.png](../../production/qa/evidence/story-006-camp-colors.png) |
| 007 | [production/qa/evidence/story-007-flow-save-ui/](../../production/qa/evidence/story-007-flow-save-ui/), 11 screenshots and one evidence stamp |

## 8. Packaging and release

The build command in `project.yaml` is `AutomationTool BuildCookRun -platform=Win64 -build -cook`, which **has not been made to work for `Game_01 5.8/` so far**, and there is no Shipping package. The submission deadline is 2026-11-16 01:59 (UK time), and the user submits it personally.

Checks before release:

- Restrict the `ModelContextProtocol` and `AllToolsets` plugins to Editor ([dependencies.md](dependencies.md#3-engine-plugins)).
- In Shipping builds the weapons' `DrawDebug*` indicator lines disappear ([known-issues.md](known-issues.md#3-legacy-and-dead-code)).
- The tint problem on heroes and enemies ([known-issues.md](known-issues.md#21-the-hero-enemies-and-xp-gems-are-gray-and-tinting-has-no-effect)).

## 9. Unreal MCP (AI collaboration)

1. Enable `ModelContextProtocol` and `AllToolsets` in the editor (already enabled in the `.uproject`).
2. Turn on **Auto Start Server** in the editor preferences (in Editor Preferences, not Project Settings).
3. The server address is `http://127.0.0.1:8000/mcp`, and the root [.mcp.json](../../.mcp.json) points at it.
4. Tool calls: `list_toolsets`, `describe_toolset`, `call_tool`.

MCP is only available while the editor is open. `Game_01 5.8/.mcp.json` is a copy with the same content.

## 10. Collaboration workflow

| Item | Convention |
|---|---|
| Branches | Trunk-based development; routine features go on a `feature/...` branch plus a PR (auto-merged in the personal repository) |
| Commits | Conventional Commits (`feat:`, `fix:`, `docs:`, `chore:`), with `Story: ...` in the body |
| Stories | 7 stories under [production/epics/directors-cut/](../../production/epics/directors-cut/), with the status in the header of each story file |
| Process | `/dev-story` to implement -> `/story-done` to accept -> `/git-push-project` to commit |
| Session state | `production/session-state/active.md` (gitignored, not in the repository) |

## 11. FAQ

| Symptom | Cause and fix |
|---|---|
| The editor opens an untitled template map every time | `EditorStartupMap` was not set; it is now set to the camp |
| Pressing Space after clicking a button "presses" the button | The button kept keyboard focus; `UUiActionButton` calls `ReturnFocusToGame()` after a click, and newly created buttons must use it |
| Command-line build fails saying a file is in use | The editor or Live Coding is still open; close it first |
| Clicking in camp does nothing | Make sure you clicked a button in the left panel or a 3D building; the camp has no walkable character |
| Camp state is lost after restarting the game | Check whether `Game_01 5.8/Saved/SaveGames/CampSave.sav` exists; cleaning `Saved/` also wipes the save |
