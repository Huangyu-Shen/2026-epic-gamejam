# 运行、测试与发布

路径以仓库根为基准。引擎路径 `D:/Game/EPIC/UE_5.8` 是本机路径，按实际调整。

## 1. 环境准备

| 工具 | 要求 | 说明 |
|---|---|---|
| Unreal Engine | 5.8（本机 5.8.4，见 [VERSION.md](../engine-reference/unreal/VERSION.md)） | `.uproject` 里的 `EngineAssociation` 是本机注册的 GUID，换机器会提示选引擎版本 |
| Visual Studio | 带"使用 C++ 的游戏开发"工作负载 | 编译 C++；作者用 Rider 读代码 |
| Git + Git LFS | — | `.uasset`、`.umap`、`.png` 等走 LFS（[.gitattributes](../../.gitattributes)） |
| Git Bash、jq | — | 仅 Claude Code 的 hook 脚本用；纯跑游戏不需要 |

## 2. 获取与打开

```bash
git clone https://github.com/Huangyu-Shen/2026-epic-gamejam.git
cd 2026-epic-gamejam
git lfs pull
```

用 UE 5.8 打开 `Game_01 5.8/Game_01.uproject`。目录名带 "5.8" 是因为原 `Game_01/` 的引擎 GUID 指向本机已不存在的引擎；副本改了 GUID，是当前唯一在用的工程。仓库里不再有 `Game_01/`（见 `git ls-files`）。

编辑器会默认打开 `L_Camp`（`EditorStartupMap`）。

## 3. 编译

### 3.1 命令行（编辑器必须关闭）

```bash
"D:/Game/EPIC/UE_5.8/Engine/Build/BatchFiles/Build.bat" Game_01Editor Win64 Development \
  "-Project=$(pwd -W)/Game_01 5.8/Game_01.uproject" -WaitMutex
```

成功时输出 `Result: Succeeded`。本仓库的 Claude 会话是通过名为 `GameBuild` 的 Windows 计划任务跑这条命令的，因为沙箱不能写 `%LOCALAPPDATA%\UnrealBuildTool`。

### 3.2 编辑器里

可以用编辑器的 Build，但**不要用 Live Coding 热重载**：它占着 UBT 的锁，之后命令行编译会失败；热重载还会让状态异常。改了 C++ 就关编辑器、命令行编译、再打开。

### 3.3 常见编译错误

| 现象 | 原因与处理 |
|---|---|
| `C2084 函数已有主体`，指向匿名命名空间里的辅助函数 | Unity 构建把多个 `.cpp` 合并，两个文件里有同名的辅助函数。给函数起不重名的名字 |
| `error C2664 … TObjectPtr<UWidget>* 无法转换为 UWidget**` | 取 `TObjectPtr` 成员的地址传给要求原始指针的参数；先用局部原始指针接，再赋值 |
| 打开编辑器崩溃在某个 `…Subsystem::Initialize` | 编辑器预览世界没有 `GameInstance`，见 [known-issues.md](known-issues.md#4-架构与健壮性) |

## 4. 运行

### 4.1 编辑器里（PIE）

1. 编辑器自动打开 `L_Camp`，点 Play。
2. 在营地左侧面板升级、拆解、选卡，点 **START RUN**。
3. 跑图：搜刮、打怪、升级、站进撤离区；结算页点 **Return to Camp**。

要直接测跑图，打开 `Content/Maps/L_Greybox.umap` 点 Play；没有营地加成，卡片为空。

### 4.2 命令行运行（独立窗口）

```bash
"D:/Game/EPIC/UE_5.8/Engine/Binaries/Win64/UnrealEditor.exe" "Game_01 5.8/Game_01.uproject" -game -windowed -ResX=1280 -ResY=720
```

### 4.3 `project.yaml` 里的命令已过期

[project.yaml](../../project.yaml) 的 `commands`（build、test、run、smoke）和 `engine.language: "Blueprint"` 还指向热身项目 `Game_Jam_01/`，见 [known-issues.md](known-issues.md)。上面的命令是按 `Game_01 5.8/` 写的。

## 5. 游戏操作

| 场景 | 操作 | 行为 |
|---|---|---|
| 跑图 | W A S D | 移动 |
| 跑图 | Space | 闪避（冲刺、无敌、冷却） |
| 跑图 | E | 搜刮最近的容器 |
| 跑图 | 1 到 6 | 使用对应背包槽的遗物（升级面板出现时 1 到 3 选升级项） |
| 跑图 | K | 调试：英雄立即死亡 |
| 跑图 | 鼠标 | 点升级面板按钮、点结算页的 Return to Camp |
| 营地 | 鼠标点 3D 建筑 | 升级 |
| 营地 | 鼠标点左侧面板 | 升级、拆解遗物、加入或移出胶片卡、START RUN |

## 6. 性能探针 `-PerfProbe`

```bash
"D:/Game/EPIC/UE_5.8/Engine/Binaries/Win64/UnrealEditor.exe" "Game_01 5.8/Game_01.uproject" /Game/Maps/L_Greybox -game -PerfProbe
```

在跑图游戏模式下：延迟 3 秒后英雄上帝模式、关武器、关刷怪调度、`SpawnHorde(150, true)`，采样 10 秒，输出一行 `PERFPROBE …` 到 `LogEnemies`（文件在 `Game_01 5.8/Saved/Logs/`）后退出。无该参数时不运行。

必须指定跑图地图（`L_Greybox`），因为默认启动图是营地，营地里没有 `ADirectorGameMode`，探针不会启动。这一点是读代码推断，没有实际用命令行验证。

故事 003 的实测记录（原文见 [证据文档](../../production/qa/evidence/story-003-combat-enemies-map-evidence.md)）：`enemies=152 avgFPS=132.3 minFPS=50.0 samples=1324`（1080p 独立窗口）。平均超过 60，最差单帧 50 低于 60，用户已接受。这是故事 003 时的结果，之后加了成长、界面等系统，没有重测。

## 7. 测试

**项目里没有自动化测试**：`Source/` 里没有 `IMPLEMENT_SIMPLE_AUTOMATION_TEST` 或 `DEFINE_SPEC`，也没有 `tests/` 目录。本次没有运行任何测试。`project.yaml` 的 `qa.level` 是 `minimal`，测试按规则豁免，改为"保留截图"作为证据。

### 视觉与流程证据

| 故事 | 位置 |
|---|---|
| 001 | [production/qa/evidence/story-001-loot-bag-heat/](../../production/qa/evidence/story-001-loot-bag-heat/) |
| 002 | [production/qa/evidence/story-002-extraction-death/](../../production/qa/evidence/story-002-extraction-death/) |
| 003 | [production/qa/evidence/story-003-combat-enemies-map/](../../production/qa/evidence/story-003-combat-enemies-map/)、[评估文档](../../production/qa/evidence/story-003-combat-enemies-map-evidence.md) |
| 006 | [story-006-camp-colors.png](../../production/qa/evidence/story-006-camp-colors.png) |
| 007 | [production/qa/evidence/story-007-flow-save-ui/](../../production/qa/evidence/story-007-flow-save-ui/)，共 11 张截图和一份证据戳 |

## 8. 打包与发布

`project.yaml` 的 build 命令是 `AutomationTool BuildCookRun -platform=Win64 -build -cook`，**目前没有为 `Game_01 5.8/` 跑通过**，也没有 Shipping 包。提交截止是 2026-11-16 01:59（英国时间），由用户本人提交。

发布前的检查：

- `ModelContextProtocol`、`AllToolsets` 插件限制到 Editor（[dependencies.md](dependencies.md#3-引擎插件)）。
- Shipping 构建里武器的 `DrawDebug*` 示意线会消失（[known-issues.md](known-issues.md#3-遗留与死代码)）。
- 英雄、敌人的染色问题（[known-issues.md](known-issues.md#21-英雄敌人经验宝石是灰色染色无效)）。

## 9. Unreal MCP（AI 协作）

1. 编辑器里启用 `ModelContextProtocol` 和 `AllToolsets`（已在 `.uproject` 里启用）。
2. 编辑器偏好设置里开 **Auto Start Server**（在 Editor Preferences，不是 Project Settings）。
3. 服务地址 `http://127.0.0.1:8000/mcp`，根目录的 [.mcp.json](../../.mcp.json) 指向它。
4. 工具调用：`list_toolsets`、`describe_toolset`、`call_tool`。

MCP 只在编辑器开着时可用。`Game_01 5.8/.mcp.json` 是同内容的副本。

## 10. 协作流程

| 项 | 约定 |
|---|---|
| 分支 | 主干开发，日常功能走 `feature/…` 分支加 PR（个人仓库自动合并） |
| 提交 | Conventional Commits（`feat:`、`fix:`、`docs:`、`chore:`），正文写 `Story: …` |
| 故事 | [production/epics/directors-cut/](../../production/epics/directors-cut/) 下 7 个故事，状态在每个故事文件头 |
| 流程 | `/dev-story` 实现 → `/story-done` 验收 → `/git-push-project` 提交 |
| 会话状态 | `production/session-state/active.md`（gitignore，不进仓库） |

## 11. 常见问题

| 现象 | 原因与处理 |
|---|---|
| 编辑器每次打开的是未命名的模板地图 | 没设 `EditorStartupMap`，现在已设为营地 |
| 点按钮后按 Space 会"按下"按钮 | 按钮保持了键盘焦点；`UUiActionButton` 点击后调 `ReturnFocusToGame()`，新建按钮必须用它 |
| 命令行编译失败，提示被占用 | 编辑器或 Live Coding 还开着，先关 |
| 营地点了没有反应 | 确认点的是左侧面板的按钮或 3D 建筑，营地没有可行走角色 |
| 重开游戏后营地状态丢了 | 检查 `Game_01 5.8/Saved/SaveGames/CampSave.sav` 是否存在；`Saved/` 被清理会连存档一起清掉 |
