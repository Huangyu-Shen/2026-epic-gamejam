# Code Wiki — 2026 Epic AI MegaJam

本目录是仓库的**结构化代码维基（Code Wiki）**，由仓库自动分析生成，用于让新读者（或新会话的 AI）快速建立对项目结构、模块、关键类、依赖与运行方式的完整心智模型。

> 项目一句话：单人参加 2026 Epic AI MegaJam 的 Unreal Engine 5.8 项目，主题 **Director's Cut**，用 Claude Code + 一组游戏工作室 agent 在 4 天内做一款奇幻斜俯视「割草搜打撤」游戏。

## 文档索引

| 文档 | 内容 |
|---|---|
| [architecture.md](architecture.md) | 整体架构：仓库分层、目录结构、系统数据流、四层模型 |
| [modules.md](modules.md) | 主要模块职责：UE 源码各模块 + 各 Subsystem 的职责边界 |
| [classes.md](classes.md) | 关键类与函数说明：逐类说明职责、关键属性与关键方法 |
| [dependencies.md](dependencies.md) | 依赖关系：UE 模块依赖、类/子系统间的调用关系图 |
| [build-and-run.md](build-and-run.md) | 项目运行方式：环境准备、编译、运行、测试、性能探针 |

## 技术栈速览

| 层 | 选择 |
|---|---|
| 引擎 | Unreal Engine 5.8（项目实际 5.8.4） |
| 语言 | C++ 为主（`Source/Game_01/`），Blueprint 为辅（数据/表现） |
| 渲染 | Lumen + Nanite（deferred）、Substrate、DX12/SM6 |
| 物理 | Chaos |
| AI 工具 | Claude Code + Unreal MCP（`ModelContextProtocol` + `AllToolsets` 插件） |
| 工作流 | Claude Code Game Studios（CCGS）：agent / skill / hooks / 脚本 |
| 版本控制 | Git + Git LFS，托管 GitHub |

## 项目状态

- 当前阶段：**Systems Design**（见 [production/stage.txt](../../production/stage.txt)）
- Epic：`Director's Cut (MVP)`，7 个 story（见 [EPIC.md](../../production/epics/directors-cut/EPIC.md)）
- 已实现 story 001–003：战利品/背包/热度、撤离/死亡、局内战斗/敌人/灰盒地图（story 004–007 尚未开始）

## 源码地图（快速定位）

```
Game_01 5.8/
├── Game_01.uproject          # UE 工程（模块 Game_01，启用 MCP 插件）
├── Config/                   # 引擎/游戏/输入配置
├── Content/Maps/             # L_Greybox.umap（灰盒地图）
└── Source/Game_01/
    ├── Game_01.cpp/.h        # 模块入口
    ├── Game_01.Build.cs      # 模块依赖声明
    ├── Game_01.Target.cs     # Game 目标
    ├── Game_01Editor.Target.cs # Editor 目标
    ├── Public/  &  Private/  # 头文件与实现（镜像目录结构）
    │   ├── Combat/           # 自动武器、生命值组件
    │   ├── Enemies/          # 敌人、刷怪、敌人管理子系统
    │   ├── Extraction/       # 撤离点、营地库存、结算子系统
    │   ├── Game/             # 游戏模式（DirectorGameMode）
    │   ├── Loot/             # 战利品、背包、热度、容器
    │   ├── Player/           # 玩家角色（HeroCharacter）
    │   └── StandIn/          # 一次性占位 Pawn/GameMode（story 001 用）
```

非源码层（设计文档 + AI 工具链 + 生产管理）见 [architecture.md](architecture.md)。
