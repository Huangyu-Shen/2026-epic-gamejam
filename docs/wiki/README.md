# Code Wiki — 2026 Epic AI MegaJam

给要读懂或修改这个游戏代码的人看的文档。每页都对照源码写成；与旧文档或注释冲突时以代码为准。

> 项目一句话：单人参加 2026 Epic AI MegaJam 的 Unreal Engine 5.8 项目，主题 **Director's Cut**，一款奇幻斜俯视的"割草 × 搜打撤 × 薄营地"游戏，用 Claude Code 在 4 天内做成。

对应代码：`main` 分支提交 `cd44b7f`（2026-11-13），故事 001 到 007 全部完成。源码根目录 [`Game_01 5.8/Source/Game_01/`](../../Game_01%205.8/Source/Game_01/)。

## 一页看懂

| 项 | 内容 |
|---|---|
| 形态 | 单机 UE 5.8 游戏，一个 C++ 模块 `Game_01`（约 6350 行，其中界面 955 行、遗留占位 256 行） |
| 语言与框架 | C++20；界面是用 C++ 搭的 UMG；没有蓝图类 |
| 两张地图 | `L_Camp`（营地，默认启动）、`L_Greybox`（跑图） |
| 核心循环 | 营地升级和选卡 → 跑图搜刮、打怪、升级 → 撤离或死亡 → 结算回营地 |
| 状态存放 | 营地状态在 GameInstance 子系统（跨关卡），每局状态在 World 子系统（换图清零） |
| 数据 | 9 张 DataTable（`Director Tuning` 引用）；表缺失时用代码里的默认值 |
| 存档 | 单槽 `CampSave`，存 Iron、仓库遗物、建筑等级；位置 `Game_01 5.8/Saved/SaveGames/` |
| 自动测试 | **没有**（`qa.level: minimal` 豁免），证据是截图 |
| 构建状态 | `Game_01Editor Win64 Development` 构建成功（2026-11-13） |

## 目录

| 文档 | 内容 |
|---|---|
| [architecture.md](architecture.md) | 技术栈、两张地图与游戏模式、分层与依赖图、运行时结构、3 条关键数据流、设计原则、仓库目录树 |
| [modules.md](modules.md) | 11 个源码目录各自的职责与边界 |
| [classes.md](classes.md) | 关键类与函数：签名、行为、边界、默认数值、公式 |
| [ui.md](ui.md) | HUD、营地面板、屏幕提示、共用控件构件 |
| [data-and-storage.md](data-and-storage.md) | 存档、9 张调参表、配置文件、单位约定、版本兼容性 |
| [dependencies.md](dependencies.md) | 目录间依赖、UE 模块与插件、第三方依赖、Unity 构建的坑 |
| [build-and-run.md](build-and-run.md) | 环境、编译、运行、操作、性能探针、测试现状、发布前检查、常见问题 |
| [known-issues.md](known-issues.md) | 读代码确认的问题：存档、数据、视觉、遗留代码、健壮性 |

## 阅读路线

| 你是谁 | 建议顺序 |
|---|---|
| 第一次接触 | README → [architecture.md](architecture.md) → [build-and-run.md](build-and-run.md) |
| 改玩法规则 | [modules.md](modules.md) → [classes.md](classes.md) → [data-and-storage.md](data-and-storage.md) |
| 改界面 | [ui.md](ui.md) → [architecture.md 第 4 节](architecture.md#4-运行时结构) |
| 加新系统 | [architecture.md 第 3 节](architecture.md#3-分层与依赖图) → [dependencies.md](dependencies.md) → [known-issues.md](known-issues.md) |

## 术语表

| 词 | 含义 | 出处 |
|---|---|---|
| Iron | 唯一的通用材料，搜刮得到，撤离后进营地，用来升级建筑 | `ULootBagComponent`、`UCampStockSubsystem` |
| 遗物（Relic） | Swiftstep、Warbrand、Aegis 三种，可留着带回营地拆解，也可当场使用换 buff | `ERelicType` |
| 热度（Heat） | 背包里放遗物升高，用遗物降低；决定刷怪压力 | `ULootHeatSubsystem` |
| 撤离 | 站进撤离区把进度条填满，背包结算进营地 | `AExtractionZone` |
| 胶片卡（Film card） | 局前选的难度卡，强制尸潮或 Boss，换更高的收益倍率 | `UFilmCardSubsystem` |
| 剪辑室（Editing room） | 营地里选胶片卡的区域（Should 级里还有"恢复被剪掉的片段"，未做） | [CampWidget](ui.md#4-营地界面-ucampwidget) |
| Smithy / Clinic | 两栋建筑：加武器伤害 / 加最大生命 | `UCampMetaSubsystem` |
| 软暂停 | 升级面板出现时敌人、刷怪、武器停止，引擎时间没停 | [architecture.md](architecture.md#4-运行时结构) |
| 故事 001 到 007 | 实现任务编号 | [production/epics/directors-cut/](../../production/epics/directors-cut/) |
| MVP feature N | 游戏简报里的功能编号 | [design/game-brief.md](../../design/game-brief.md) |

## 维护约定

| 改了什么 | 要同步更新 |
|---|---|
| 新增或改名一个类、函数、公开字段 | [classes.md](classes.md)，必要时 [modules.md](modules.md) |
| 新增源码目录或系统 | [modules.md](modules.md)、[dependencies.md](dependencies.md) 的依赖表和 [architecture.md](architecture.md) 的图 |
| 改调参表、行名、存档字段 | [data-and-storage.md](data-and-storage.md)；改存档布局还要加版本检查并更新 [known-issues.md](known-issues.md) |
| 改 HUD 或营地面板 | [ui.md](ui.md) |
| 改 `Build.cs`、插件、构建命令 | [dependencies.md](dependencies.md)、[build-and-run.md](build-and-run.md) |
| 修掉一个已知问题 | 从 [known-issues.md](known-issues.md) 删除对应条目 |

## 与旧版 wiki 的差异

旧版 wiki（故事 001 到 003 时期）已被整体重写。原来的 6 个文件名全部保留（README、architecture、modules、classes、dependencies、build-and-run），新增 `ui.md`、`data-and-storage.md`、`known-issues.md` 三页。旧文档里被纠正的错误见交付说明和 [known-issues.md](known-issues.md#21-英雄敌人经验宝石是灰色染色无效)。
