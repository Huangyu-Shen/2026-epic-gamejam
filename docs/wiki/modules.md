# 主要模块职责

源码按功能分目录，头文件在 `Public/<模块>/`，实现在 `Private/<模块>/`。行数含头文件和实现，来自 2026-11-13 的 `wc -l`。路径省略前缀 `Game_01 5.8/Source/Game_01/`。

| 模块 | 行数 | 职责 | 不负责 |
|---|---|---|---|
| [Loot](#loot-战利品背包热度) | 997 | 搜刮容器、背包、遗物、热度 | 撤离结算、界面 |
| [Combat](#combat-战斗) | 384 | 生命值、自动武器 | 武器数值（在 Progression）、敌人移动 |
| [Enemies](#enemies-敌人) | 713 | 敌人、刷怪、群体移动与接触伤害 | 击杀后的经验（在 Progression） |
| [Progression](#progression-局内成长) | 724 | XP、升级、三选一、武器与被动 | 武器开火（在 Combat） |
| [Extraction](#extraction-撤离与营地库存) | 584 | 撤离区、一局的结束与结算、营地库存 | 存档、建筑 |
| [Editing](#editing-局前剪辑) | 277 | 胶片卡数据、槽位、应用到一局 | 选卡界面（在 UI） |
| [Camp](#camp-营地) | 670 | 营地场景、建筑升级与加成、遗物拆解 | 营地面板（在 UI） |
| [Flow](#flow-存档) | 178 | 单槽存档的读写 | 决定何时存（监听库存与建筑变化） |
| [Player](#player-英雄) | 421 | 英雄移动、闪避、搜索、使用遗物、创建 HUD、开局初始化 | 战斗、背包规则 |
| [Game](#game-游戏模式与调参设置) | 189 | 跑图游戏模式、性能探针、`Director Tuning` 设置 | 营地游戏模式（在 Camp） |
| [UI](#ui-界面) | 955 | HUD、营地面板、屏幕提示、共用控件构件 | 游戏规则 |
| [StandIn](#standin-遗留占位) | 256 | 故事 001 的一次性占位角色，已被取代 | — |

## Loot 战利品背包热度

| 类 | 职责 |
|---|---|
| [`ULootBagComponent`](../../Game_01%205.8/Source/Game_01/Private/Loot/LootBagComponent.cpp) | 背包：Iron 按 `ceil(Iron / IronPerSlot)` 占槽，每个遗物占 1 槽，总容量 `BagCapacitySlots`（默认 6）；加遗物升热度，用遗物加 buff 并降热度 |
| [`ALootContainer`](../../Game_01%205.8/Source/Game_01/Private/Loot/LootContainer.cpp) | 可搜索容器（补给箱、宝库），每个只能搜一次；背包满时不消耗，重试不会重新随机 |
| [`ULootHeatSubsystem`](../../Game_01%205.8/Source/Game_01/Private/Loot/LootHeatSubsystem.cpp) | 全局热度 0 到 `MaxHeat`；`GetSpawnPressureMultiplier()` 在 1.0 到 3.0 之间线性插值，刷怪读它 |
| [`ULootConfig`](../../Game_01%205.8/Source/Game_01/Private/Loot/LootConfig.cpp) | 数据资产：遗物表、容器表、调参表，缺失时用构造函数里的内联默认值 |
| [`LootTypes.h`](../../Game_01%205.8/Source/Game_01/Public/Loot/LootTypes.h) | 枚举与数据行：`ERelicType`（Swiftstep、Warbrand、Aegis）、`ELootContainerType`、`FLootStatModifier`、`FRelicRow`、`FLootContainerRow`、`FLootTuning` |

## Combat 战斗

| 类 | 职责 |
|---|---|
| [`UHealthComponent`](../../Game_01%205.8/Source/Game_01/Private/Combat/HealthComponent.cpp) | 生命值、受击后无敌窗口、无敌模式、`Kill()`；`BeginPlay` 从 `HealthTuningTable` 的 `Hero` 行读数值 |
| [`UAutoWeaponComponent`](../../Game_01%205.8/Source/Game_01/Private/Combat/AutoWeaponComponent.cpp) | 每把已拥有的武器按各自间隔自动开火；Blade 打最近的、Bolt 打一条线并穿透、Nova 打半径内全部 |

## Enemies 敌人

| 类 | 职责 |
|---|---|
| [`AEnemy`](../../Game_01%205.8/Source/Game_01/Private/Enemies/Enemy.cpp) | 灰盒敌人，不 Tick；`ApplyDamage` 到 0 时通知管理器并销毁 |
| [`UEnemyManagerSubsystem`](../../Game_01%205.8/Source/Game_01/Private/Enemies/EnemyManagerSubsystem.cpp) | 持有全部活敌人，一个 Tick 完成：追击、分格推开、接触伤害（每个敌人独立冷却） |
| [`UEnemySpawnSubsystem`](../../Game_01%205.8/Source/Game_01/Private/Enemies/EnemySpawnSubsystem.cpp) | 波次调度：在英雄周围的圆环上刷 Grunt，速率 = `(BaseRate + RampPerMinute × 分钟数) × 热度压力`；`SpawnHorde`、`SpawnBoss` 供胶片卡调用 |
| [`EnemyTypes.h`](../../Game_01%205.8/Source/Game_01/Public/Enemies/EnemyTypes.h) | `FEnemyTuning`、`FSwarmTuning`、`FSpawnTuning`、`EEnemyKind` |

## Progression 局内成长

| 类 | 职责 |
|---|---|
| [`URunProgressionSubsystem`](../../Game_01%205.8/Source/Game_01/Private/Progression/RunProgressionSubsystem.cpp) | XP 与等级（需求 = `BaseXp × Growth^(Level-1)`）、三选一生成与应用、武器等级、被动层数、拾取物磁吸；每局由 `ResetProgression()` 清零，起手送 1 级 Blade |
| [`AXpPickup`](../../Game_01%205.8/Source/Game_01/Private/Progression/XpPickup.cpp) | 经验宝石，注册到子系统，由子系统每帧吸引和拾取 |
| [`ProgressionTypes.h`](../../Game_01%205.8/Source/Game_01/Public/Progression/ProgressionTypes.h) | `EWeaponType`（Blade、Bolt、Nova）、`EPassiveType`（Swiftness、Might、Vitality、Greed）、`FXpCurveRow`、`FWeaponRow`、`FPassiveRow` |

## Extraction 撤离与营地库存

| 类 | 职责 |
|---|---|
| [`AExtractionZone`](../../Game_01%205.8/Source/Game_01/Private/Extraction/ExtractionZone.cpp) | 撤离点：英雄站在里面且一局进行中，进度条在 `ExtractionSeconds`（默认 5 秒）内填满；离开清零或衰减；只在有人或在衰减时 Tick |
| [`URunOutcomeSubsystem`](../../Game_01%205.8/Source/Game_01/Private/Extraction/RunOutcomeSubsystem.cpp) | 一局的状态机，只结束一次；结算进库存、写存档、清背包、清热度、广播 `OnRunEnded` |
| [`UCampStockSubsystem`](../../Game_01%205.8/Source/Game_01/Private/Extraction/CampStockSubsystem.cpp) | Iron 与仓库遗物，GameInstance 级 |
| [`FRunSettlement::Compute`](../../Game_01%205.8/Source/Game_01/Private/Extraction/ExtractionTypes.cpp) | 纯函数结算：撤离得 `floor(背包 Iron × 倍率)` 和全部遗物；死亡全部损失 |

## Editing 局前剪辑

[`UFilmCardSubsystem`](../../Game_01%205.8/Source/Game_01/Private/Editing/FilmCardSubsystem.cpp)：两个槽位的胶片卡。卡片效果：`ForceHorde`（开局刷一批 Grunt）、`ForceBoss`（开局刷一个 Boss），收益倍率相乘。选择存在 `UCampMetaSubsystem::PendingFilmCards`，所以能跨 `OpenLevel`。一局结束时清空。

## Camp 营地

| 类 | 职责 |
|---|---|
| [`UCampMetaSubsystem`](../../Game_01%205.8/Source/Game_01/Private/Camp/CampMetaSubsystem.cpp) | 两栋建筑（Smithy 加武器伤害、Clinic 加最大生命）的等级与升级；费用 `floor(BaseCost × CostGrowth^等级)`；拆解遗物；`PendingFilmCards` |
| [`ACampGameMode`](../../Game_01%205.8/Source/Game_01/Private/Camp/CampGameMode.cpp) | 营地场景里没有放置建筑时，自动生成 Smithy、Clinic、仓库 |
| [`ACampPawn`](../../Game_01%205.8/Source/Game_01/Private/Camp/CampPawn.cpp) | 固定相机，鼠标点击建筑升级，创建 `UCampWidget` |
| [`ACampBuilding`](../../Game_01%205.8/Source/Game_01/Private/Camp/CampBuilding.cpp)、[`ACampWarehouse`](../../Game_01%205.8/Source/Game_01/Private/Camp/CampWarehouse.cpp) | 灰盒方块；建筑每级变高并按类型着色 |

## Flow 存档

[`UCampSaveSubsystem`](../../Game_01%205.8/Source/Game_01/Private/Flow/CampSaveSubsystem.cpp) 与 [`UCampSaveGame`](../../Game_01%205.8/Source/Game_01/Public/Flow/CampSaveGame.h)。详见 [data-and-storage.md](data-and-storage.md)。

## Player 英雄

[`AHeroCharacter`](../../Game_01%205.8/Source/Game_01/Private/Player/HeroCharacter.cpp)：WASD 移动、Space 闪避、E 搜索、1 到 6 使用遗物、K 调试自杀。`BeginPlay` 调 `StartRun()` 做开局初始化，再 `CreateHud()`。

## Game 游戏模式与调参设置

| 类 | 职责 |
|---|---|
| [`ADirectorGameMode`](../../Game_01%205.8/Source/Game_01/Private/Game/DirectorGameMode.cpp) | 跑图游戏模式，默认 Pawn 为英雄；命令行带 `-PerfProbe` 时跑 150 敌人帧率探针 |
| [`UDirectorTuningSettings`](../../Game_01%205.8/Source/Game_01/Public/Game/DirectorTuningSettings.h) | 项目设置里的"Director Tuning"，引用 9 张 DataTable，值存在 [DefaultGame.ini](../../Game_01%205.8/Config/DefaultGame.ini) |

## UI 界面

见 [ui.md](ui.md)。

## StandIn 遗留占位

[`AStandInCharacter`](../../Game_01%205.8/Source/Game_01/Private/StandIn/StandInCharacter.cpp)、`AStandInGameMode`：故事 001 的一次性占位，注释写明"故事 003 替换"。代码里已没有人使用它们，只剩 [`HeroCharacter.h`](../../Game_01%205.8/Source/Game_01/Public/Player/HeroCharacter.h) 和 [`DirectorGameMode.h`](../../Game_01%205.8/Source/Game_01/Public/Game/DirectorGameMode.h) 的注释提到它们，见 [known-issues.md](known-issues.md)。
