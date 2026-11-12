# 主要模块职责

本文按 `Game_01 5.8/Source/Game_01/` 的目录组织，逐模块说明职责。源码采用 `Public/`（头文件）与 `Private/`（实现）镜像结构，模块名即目录名。

## 模块总览

| 模块（目录） | 职责 | 关键类型 |
|---|---|---|
| `Game` | 游戏模式 + 性能探针 | `ADirectorGameMode` |
| `Player` | 玩家角色：移动/闪避/交互/受击/调试 | `AHeroCharacter` |
| `Combat` | 战斗组件：自动武器、生命值 | `UAutoWeaponComponent`、`UHealthComponent` |
| `Enemies` | 敌人实体 + 刷怪 + 群体管理 | `AEnemy`、`UEnemySpawnSubsystem`、`UEnemyManagerSubsystem`、`FEnemyTuning`/`FSwarmTuning`/`FSpawnTuning` |
| `Loot` | 战利品：背包/热度/容器/配置 | `ULootBagComponent`、`ULootHeatSubsystem`、`ALootContainer`、`ULootConfig`、`FLootTuning` 等 |
| `Extraction` | 撤离 + 结算 + 营地库存 | `AExtractionZone`、`URunOutcomeSubsystem`、`UCampStockSubsystem`、`FRunSettlement` |
| `StandIn` | 一次性占位（story 001 用，后被替换） | `AStandInCharacter`、`AStandInGameMode` |
| 模块入口 | UE 模块/目标定义 | `Game_01.cpp/.h`、`Game_01.Build.cs`、`Game_01.Target.cs`、`Game_01Editor.Target.cs` |

---

## 1. `Game` — 游戏模式

**职责**：定义本局玩法入口（默认 Pawn 是英雄），并提供命令行 `-PerfProbe` 触发的自动性能探针。

- `ADirectorGameMode`：继承 `AGameModeBase`。无 `-PerfProbe` 时除默认 Pawn 外几乎无事可做；带 `-PerfProbe` 时在延迟后 god-mode 英雄、生成尸潮、采样帧率、输出一行 `PERFPROBE` 日志并退出。

## 2. `Player` — 玩家角色

**职责**：玩家即英雄。实现斜俯视相机、原始按键控制、闪避（带无敌帧）、搜容器、使用遗物、受击与死亡处理、新局复位，以及大量屏幕调试状态。

- `AHeroCharacter`：继承 `ACharacter`。持有 `Bag`（`ULootBagComponent`）、`Health`（`UHealthComponent`）、`Weapon`（`UAutoWeaponComponent`）三个组件，以及 `BodyMesh`（引擎圆柱灰盒）、`CameraBoom`+`Camera`（固定斜俯视相机）。
- 控制键：WASD 移动、Space 闪避、E 搜最近容器、1–6 使用对应背包槽遗物；调试键 K 死亡、N 新局、H 尸潮、B Boss。

## 3. `Combat` — 战斗

**职责**：可复用的战斗能力组件，与具体角色解耦。

- `UAutoWeaponComponent`：起始武器。每 `Interval` 秒自动攻击范围内最近敌人，无需按键；从背包读伤害倍率；局结束或主人死亡时停火。
- `UHealthComponent`：生命值 + 受击短暂无敌窗口 + god-mode 开关 + 死亡/受伤委托。

## 4. `Enemies` — 敌人

**职责**：敌人实体（无碰撞、无 Tick、无移动组件的引擎方块）由**集中式子系统**驱动，实现割草的群体行为与刷怪调度。

- `AEnemy`：单个敌人 Actor。仅持有 `Body`（引擎方块）、`Tuning`（一行参数）、`Health`、`NextContactTime`；提供 `Init`（应用参数行）、`ApplyDamage`、`GetBodyRadius`。
- `UEnemyManagerSubsystem`（Tickable）：持有全部存活敌人，在**一个 Tick** 内完成：追击英雄（XY 平面）、朝向移动方向、网格推挤分离、接触伤害（每敌人独立冷却）。
- `UEnemySpawnSubsystem`（Tickable）：波次调度器。按 `(BaseRate + RampPerMinute×分钟) × 热度刷怪压力` 每秒在英雄周围环形带刷 Grunt，受 `MaxAlive` 上限约束；提供 `SpawnHorde`/`SpawnBoss`。
- 类型定义：`EEnemyKind`（Grunt/Boss）、`FEnemyTuning`（单怪参数）、`FSwarmTuning`（群体移动/分离）、`FSpawnTuning`（刷怪调度）、日志分类 `LogEnemies`。

## 5. `Loot` — 战利品

**职责**：核心「留着还是用掉」循环的实现——背包、热度、容器搜索、数据驱动配置。

- `ULootBagComponent`：玩家本局背包。Iron 与遗物共享硬槽位；加遗物升温、用遗物施加 buff 并降温；提供只读内容与 `GetActiveModifier`（合并所有激活 buff 的倍率，供 Pawn 读取）。
- `ULootHeatSubsystem`（World）：全局热度。`GetSpawnPressureMultiplier` 供刷怪器按热度插值压力倍率。
- `ALootContainer`：补给箱/宝库。每容器可搜一次，产出来自 `FLootContainerRow`；背包满时保留已掷结果避免重复掷。
- `ULootConfig`（DataAsset）：数据驱动配置，含可选 DataTable（遗物/容器/调优）+ 内联默认值，无内容也能运行。
- 类型定义：`ERelicType`（Swiftstep/Warbrand/Aegis）、`ELootContainerType`、`EBagAddResult`、`FLootStatModifier`、`FRelicRow`、`FLootContainerRow`、`FLootTuning`、`FActiveRelicBuff`、日志分类 `LogLoot`。

## 6. `Extraction` — 撤离与结算

**职责**：撤离读条、死亡与撤离的结算、营地库存（局外保留）。

- `AExtractionZone`：撤离点。携带背包的 Pawn 站在范围内读条满后调用 `ReportExtracted`；离开时清空/衰减读条。
- `URunOutcomeSubsystem`（World）：本局状态机，**恰好结束一次**（撤离或死亡）；结算背包到营地、清空背包、重置热度、广播 `OnRunEnded`。
- `UCampStockSubsystem`（GameInstance）：营地 Iron 与仓库遗物，跨局保留（持久化留待 story 007）。
- 类型定义：`ERunOutcome`、`FExtractionTuning`、`FRunResult`、`FRunSettlement`（纯结算数学，无 UObject，可脱离编辑器测试）、日志分类 `LogExtraction`。

## 7. `StandIn` — 占位（可弃）

**职责**：story 001（战利品/背包/热度）用的临时 Pawn/GameMode，story 003 已被 `AHeroCharacter`/`ADirectorGameMode` 取代，仅保留作为历史/兜底。

- `AStandInCharacter`：临时玩家（WASD、E 搜索、1–6 用遗物、K 死亡、N 新局）。
- `AStandInGameMode`：临时游戏模式（默认 Pawn 为 `AStandInCharacter`）。

## 8. 模块入口与构建

| 文件 | 职责 |
|---|---|
| `Game_01.cpp` | `IMPLEMENT_PRIMARY_GAME_MODULE`，注册主模块 |
| `Game_01.h` | 仅包含 `CoreMinimal.h`（无自定义 API） |
| `Game_01.Build.cs` | 模块依赖：`Core`、`CoreUObject`、`Engine`、`InputCore`、`EnhancedInput` |
| `Game_01.Target.cs` | Game 目标（`TargetType.Game`，BuildSettings V7，UE5_8） |
| `Game_01Editor.Target.cs` | Editor 目标（`TargetType.Editor`） |

## 9. 非源码模块（仓库层）

- **设计模块**（`design/` + `docs/plans/`）：PRD、11 个 GDD、来源计划——决定"做什么"。
- **引擎参考模块**（`docs/engine-reference/`）：版本锁定 UE API 快照——决定"怎么写"。
- **工具链模块**（`.claude/`）：agent/skill/hook/script——编排"谁来做、怎么做"。
- **生产模块**（`production/`）：epic/story/QA/会话状态——记录"做到哪、验收了没"。
