# 项目运行方式

## 1. 环境准备

| 依赖 | 说明 |
|---|---|
| Unreal Engine 5.8 | 项目实际 5.8.4；`project.yaml` 里 `engine.path` 指向 `D:/Game/EPIC/UE_5.8` |
| Git + Git LFS | `.uasset`/`.umap` 等二进制资产走 LFS |
| Visual Studio | 带「C++ 游戏开发」工作负载 |
| Claude Code | 可选（AI 协作开发用），配合 Unreal MCP 插件 |
| Git Bash + Python + jq | 运行 `.claude/hooks/` 的 shell 钩子（Windows 上） |

## 2. 获取代码

```bash
git clone https://github.com/Huangyu-Shen/2026-epic-gamejam.git
cd 2026-epic-gamejam
git lfs pull
```

> UE 项目在 `Game_01 5.8/`（C++ 工程）。`Game_Jam_01/` 是赛前热身项目，gitignored，不在仓库里。

> **为什么目录名带 "5.8"**：原 `Game_01/` 的 `EngineAssociation` 指向本机已不存在的引擎 GUID（`{E4798BF9-…}`），用 UE 5.8 打开时会提示版本不匹配并要求复制。副本把 GUID 改成了本机注册的 `{82227EA2-…}`（→ `D:/Game/EPIC/UE_5.8`），目录名因此变成 `Game_01 5.8/`。两边 `Source/` 内容完全一致，**`Game_01 5.8/` 是当前唯一在用的工程**；`Game_01/` 是冻结在旧引擎上的残留副本，可择机删除。

## 3. 打开与编译

1. 用 UE 5.8 打开 `Game_01 5.8/Game_01.uproject`（首次会提示重建项目模块并编译 C++）。
2. 首次打开会编译 `Game_01` 模块；后续改动可用编辑器内 **Build**（`Ctrl+Alt+F11`）增量编译。

> **注意**（来自会话检查点）：**不要用 Live Coding 热重载**，会导致状态异常；改 C++ 后走常规 Build 再重启编辑器/PIE。

## 4. 运行游戏

### 4.1 编辑器内 PIE

- 打开 `Content/Maps/L_Greybox.umap`（灰盒地图），点 Play（PIE）。

### 4.2 命令行运行（`project.yaml` 中的命令）

以下命令摘自 [project.yaml](../../../project.yaml) 的 `commands` 段，引擎路径按本机实际调整：

```bash
# 运行游戏（窗口化 1280x720）
"D:/Game/EPIC/UE_5.8/Engine/Binaries/Win64/UnrealEditor.exe" \
  "Game_01 5.8/Game_01.uproject" -game -windowed -ResX=1280 -ResY=720

# 编译 + 打包（BuildCookRun）
"D:/Game/EPIC/UE_5.8/Engine/Binaries/DotNET/AutomationTool/AutomationTool.exe" \
  BuildCookRun -project="Game_01 5.8/Game_01.uproject" -platform=Win64 -build -cook

# 冒烟测试（无渲染启动后退出）
"D:/Game/EPIC/UE_5.8/Engine/Binaries/Win64/UnrealEditor-Cmd.exe" \
  "Game_01 5.8/Game_01.uproject" -game -nullrhi -unattended -stdout -ExecCmds="Quit"

# 自动化测试
"D:/Game/EPIC/UE_5.8/Engine/Binaries/Win64/UnrealEditor-Cmd.exe" \
  "Game_01 5.8/Game_01.uproject" -ExecCmds="Automation RunTests Game_01.; Quit" \
  -unattended -nullrhi -stdout -FullStdOutLogOutput
```

> **提示**：`project.yaml` 中这些命令目前仍指向 `Game_Jam_01/`（热身项目路径），需按上表更新为 `Game_01 5.8/`（会话检查点中的待办项，改动需用户批准）。上面已替换为正确路径。

## 5. 游戏操作

| 按键 | 动作 |
|---|---|
| WASD | 移动 |
| Space | 闪避（带无敌帧 + 冷却） |
| E | 搜索最近的未搜索容器 |
| 1–6 | 使用对应背包槽位的遗物 |
| K | 调试：死亡 |
| N | 调试：新一局（清敌人/背包/热度/生命） |
| H | 调试：刷尸潮（40 Grunt） |
| B | 调试：刷 Boss |

> 屏幕上有每帧更新的调试状态（HP/热度/刷怪率/敌人数/击杀/背包/遗物/buff），便于验证。这些调试键与 HUD 在后续 story（004–007）会被正式卡片/UI 取代。

## 6. 性能探针（`-PerfProbe`）

游戏模式内置自动性能探针，命令行加 `-PerfProbe` 触发：

- 延迟 `StartDelaySeconds`（默认 3s）后：英雄 god-mode、`SpawnHorde(EnemyCount=150, 忽略上限)`、开始采样 `SampleSeconds`（默认 10s）。
- 采样结束输出一行 `PERFPROBE` 日志（到 `LogEnemies`，文件在 `Game_01 5.8/Saved/Logs/`）并退出。
- 无 `-PerfProbe` 时探针逻辑完全不运行。

实测记录（story 003）：`PERFPROBE enemies=152 avgFPS=132.3 minFPS=50.0 samples=1324`（1080p standalone `-game`）。

## 7. Unreal MCP 连接（AI 协作开发）

1. 编辑器内启用插件 `ModelContextProtocol` + `AllToolsets`。
2. 编辑器偏好设置里开启 **Auto Start Server**（在 Editor Preferences，非 Project Settings）。
3. 服务地址 `http://127.0.0.1:8000/mcp`，仓库根 `.mcp.json` 指向它。
4. 工具调用方式：`list_toolsets` / `describe_toolset` / `call_tool`（工具搜索已开启）。

> **前提**：MCP 只在**编辑器打开**时可用。MCP 卡死/崩溃时靠频繁存档 + Git 恢复。

## 8. Claude Code 工作流（可选）

在仓库根目录启动 Claude Code（才能加载 `CLAUDE.md`、hooks、权限规则），常用斜杠命令：

| 命令 | 用途 |
|---|---|
| `/start` | 首次引导（配置引擎/阶段） |
| `/brainstorm` / `/map-systems` | 设计 |
| `/create-stories` / `/dev-story` / `/story-done` | 拆故事 → 实现 → 验收 |
| `/story-readiness` | 故事可实施性校验 |
| `/code-review` / `/perf-profile` | 代码审查 / 性能分析 |

## 9. 已知限制与待办

- 敌人/英雄染色不生效：`BasicShapeMaterial` 无 `Color` 参数（`Enemy.cpp::Init` 与 `HeroCharacter.cpp::BeginPlay` 的动态材质 tint 无效），需改用带 Color 向量参数的材质。
- `project.yaml` 的 `commands` 与 `engine.language` 待更新（见上文提示）。
- story 004–007（局内成长、局前剪辑、营地、流程/存档/UI）尚未实现。
- 无许可证；Fab 免费资产可用性与授权未核验。
