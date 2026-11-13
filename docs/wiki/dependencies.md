# 依赖关系

## 1. 模块之间怎么依赖

整体依赖图见 [architecture.md 的第 3 节](architecture.md#3-分层与依赖图)。这里是按目录汇总的"谁引用谁"，由头文件 `#include` 和源码里的 `GetSubsystem<>` 调用整理得出。

| 目录 | 引用（`#include` 统计） | 被谁引用 |
|---|---|---|
| `Camp/` | Extraction、Game、UI | Combat、Editing、Flow、Player、UI |
| `Combat/` | Camp、Enemies、Extraction、Game、Loot、Progression | Game、Player、Progression、UI |
| `Editing/` | Camp、Enemies、Extraction、Game | Player、UI |
| `Enemies/` | Extraction、Game、Loot、Player、Progression | Combat、Editing、Game、Player、Progression |
| `Extraction/` | Flow、Loot | Camp、Combat、Editing、Enemies、Flow、Player、UI |
| `Flow/` | Camp、Extraction、Loot | Extraction |
| `Game/` | Combat、Enemies、Player | Camp、Combat、Editing、Enemies、Progression |
| `Loot/` | UI（只调 `UHudMessageSubsystem::Post`） | Combat、Enemies、Extraction、Flow、Player、UI |
| `Player/` | Camp、Combat、Editing、Enemies、Extraction、Loot、Progression、UI | Enemies、Game、Progression、UI |
| `Progression/` | Combat、Enemies、Game、Player | Combat、Enemies、Player、UI |
| `UI/` | Camp、Combat、Editing、Extraction、Loot、Player、Progression | Camp、Loot、Player |

统计方法：对每个目录的头文件和实现文件，数它 `#include "<其他目录>/..."` 的目录集合（2026-11-13）。`Game/` 目录里的 `UDirectorTuningSettings` 被各系统读取，`ADirectorGameMode` 引用英雄、敌人和武器。

**有环**：Enemies↔Player↔Progression、Progression↔Combat、Extraction↔Flow、Camp↔UI、Loot↔UI。原因是敌人要伤害英雄、英雄要读成长、成长要加英雄生命、结算要写存档而存档要读库存。这些环都在同一个 C++ 模块 `Game_01` 里，编译没有问题；没有任何自动检查守住依赖方向，也没有规则文件。新增系统时参考 [architecture.md 的第 3 节](architecture.md#3-分层与依赖图)的约定：界面只读不改，营地状态放 GameInstance，每局状态放 World。

## 2. UE 模块依赖

声明在 [Game_01.Build.cs](../../Game_01%205.8/Source/Game_01/Game_01.Build.cs)：

| 类型 | 模块 | 用途 |
|---|---|---|
| Public | `Core`、`CoreUObject`、`Engine` | 基础 |
| Public | `InputCore` | `EKeys`，原始按键绑定 |
| Public | `EnhancedInput` | **声明了但源码里没有任何 `#include`**，当前没用到 |
| Public | `DeveloperSettings` | `UDirectorTuningSettings`（项目设置页） |
| Public | `UMG` | 界面控件 |
| Public | `Slate`、`SlateCore` | 字体信息、`FSlateApplication::SetAllUserFocusToGameViewport` |
| Private | 无 | — |

`PCHUsage = UseExplicitOrSharedPCHs`。目标文件 [Game_01.Target.cs](../../Game_01%205.8/Source/Game_01.Target.cs) 和 [Game_01Editor.Target.cs](../../Game_01%205.8/Source/Game_01Editor.Target.cs) 都是 `BuildSettingsVersion.V7`、`EngineIncludeOrderVersion.Unreal5_8`。

**Unity 构建的坑**：UBT 会把多个 `.cpp` 合并编译。匿名命名空间里同名的辅助函数会冲突。例如 `RunHudWidget.cpp` 和 `CampWidget.cpp` 曾各有一个 `RelicName`，合并后报 C2084，现在分别叫 `HudRelicLabel`、`CampRelicLabel`。新增文件时给内部函数起带文件特征的名字。

## 3. 引擎插件

[Game_01.uproject](../../Game_01%205.8/Game_01.uproject)：

| 插件 | 范围 | 用途 |
|---|---|---|
| `ModelingToolsEditorMode` | 仅 Editor | 编辑器建模模式（模板默认） |
| `ModelContextProtocol` | 全部 | 让 Claude 等 AI 通过 MCP 连接编辑器 |
| `AllToolsets` | 全部 | MCP 的工具集合（Actor、资产、场景、UMG 等） |
| `Terminal` | 全部 | 编辑器内终端 |

`ModelContextProtocol` 和 `AllToolsets` 是开发期工具，不是游戏运行需要的；它们标记为"全部"，打包时会带上，发行前应该确认是否要限制到 Editor。

## 4. 第三方依赖

没有任何第三方库。游戏代码只依赖 UE 自带模块和上面的引擎插件。

## 5. 刻意没有做的事

- **没有 Enhanced Input 资产**：输入用 `BindKey` 绑原始按键，源码注释和故事都写明"无输入资产"，为了灰盒期快。
- **没有 GAS**：技能、伤害、状态都是自己写的组件和子系统；[`.claude/agents`](../../.claude/agents/) 里有 `ue-gas-specialist` 但项目没使用 GAS。
- **没有物理碰撞的敌人**：敌人无碰撞、不 Tick，目的是 150 个敌人的帧率。

## 6. 仓库里不参与构建的目录

| 目录 | 内容 |
|---|---|
| `.claude/` | AI 协作用的 agent、skill、hook、脚本、规则 |
| `design/`、`docs/`、`production/` | 设计文档、计划、QA 证据 |
| `Game_01 5.8/Binaries`、`Intermediate`、`Saved`、`DerivedDataCache` | 生成物，已 gitignore |
| `Game_Jam_01/` | 赛前热身工程，gitignore，不在仓库里 |
