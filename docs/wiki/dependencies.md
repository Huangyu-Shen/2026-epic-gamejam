# 依赖关系

本文描述三类依赖：UE 引擎模块依赖、源码模块间依赖、运行时对象（类/子系统）调用关系。

## 1. UE 引擎模块依赖

由 [Game_01.Build.cs](../../../Game_01%205.8/Source/Game_01/Game_01.Build.cs) 声明：

| 依赖类型 | 模块 | 用途 |
|---|---|---|
| Public | `Core` | 基础类型/容器 |
| Public | `CoreUObject` | UObject/UCLASS/USTRUCT/UENUM |
| Public | `Engine` | Actor/Component/World/GameMode/DataTable/DataAsset |
| Public | `InputCore` | `EKeys` 等按键常量 |
| Public | `EnhancedInput` | 增强输入（当前主要用原始按键绑定，仍声明依赖） |

**目标定义**（`Game_01.Target.cs` / `Game_01Editor.Target.cs`）：`BuildSettingsVersion.V7` + `EngineIncludeOrderVersion.Unreal5_8`。

**插件**（`Game_01.uproject`）：

| 插件 | 说明 |
|---|---|
| `ModelContextProtocol` | Unreal MCP 服务端（供 Claude Code 连接） |
| `AllToolsets` | MCP 全部工具集 |
| `ModelingToolsEditorMode` | 编辑器建模工具（仅 Editor） |

## 2. 源码模块间依赖（include 方向）

箭头「→」表示 include / 使用方向。

```
Combat（UAutoWeaponComponent, UHealthComponent）
   ├─→ Loot（UAutoWeaponComponent 读 ULootBagComponent 的伤害倍率）

Player（AHeroCharacter）
   ├─→ Combat（持有 UAutoWeaponComponent、UHealthComponent）
   ├─→ Loot（持有 ULootBagComponent；SearchNearestContainer 用 ALootContainer、ULootHeatSubsystem）
   ├─→ Enemies（调试键 H/B 调 UEnemySpawnSubsystem；读 UEnemyManagerSubsystem 状态）
   └─→ Extraction（死亡/新局调 URunOutcomeSubsystem）

Enemies
   ├─→ Player（UEnemyManagerSubsystem 读 AHeroCharacter 位置；UEnemySpawnSubsystem 绕英雄刷怪）
   └─→ Extraction（UEnemyManagerSubsystem 读 URunOutcomeSubsystem 判断能否造成伤害）

Extraction
   ├─→ Loot（URunOutcomeSubsystem 读 ULootBagComponent、ULootHeatSubsystem）
   └─→（UCampStockSubsystem 是 GameInstanceSubsystem，被 URunOutcomeSubsystem 调用来入库）

Game（ADirectorGameMode）
   ├─→ Player（默认 Pawn 指向 AHeroCharacter）
   ├─→ Enemies（性能探针调 SpawnHorde）
   └─→ Combat（god-mode 调 Health->bGodMode）

StandIn → Loot（AStandInCharacter 持有 ULootBagComponent，历史占位）
```

**依赖方向总原则**：`Combat` 与 `Loot` 是相对底层的可复用能力；`Player`/`Enemies`/`Extraction`/`Game` 是上层编排者。子系统通过 `GetWorld()->GetSubsystem<T>()` / `GetGameInstance()->GetSubsystem<T>()` 解耦获取，不直接持有彼此指针。

## 3. 运行时对象调用关系图

```
                 ┌───────────────────────┐
                 │   ADirectorGameMode   │  (默认 Pawn / 性能探针)
                 └───────────┬───────────┘
                             │ 默认 Pawn
                             ▼
                 ┌───────────────────────┐
                 │    AHeroCharacter     │
                 │ Bag/Health/Weapon     │
                 └──┬────────┬────────┬──┘
                    │        │        │
        ┌───────────▼─┐ ┌────▼─────┐ ┌▼──────────────┐
        │UAutoWeapon  │ │UHealth   │ │ULootBag       │
        │Component    │ │Component │ │Component      │
        └─────┬───────┘ └────┬─────┘ └┬───────┬──────┘
              │ 找最近敌人    │OnDied  │升温/降温│
              ▼              ▼        ▼       │
   ┌──────────────────┐  ┌──────────────────┐│
   │UEnemyManager     │  │URunOutcome       ││
   │Subsystem         │  │Subsystem         ││
   │ (Tick: 追击/分离/│  │ (结束一次/结算)   ││
   │  接触伤害)        │  └────────┬─────────┘│
   └────────┬─────────┘           │           │
            │接触伤害→ReceiveDamage│           │
            └──────────→ AHeroCharacter ──────┘
                         (ApplyDamage)
            ▲
            │ SpawnHorde/SpawnBoss
   ┌────────┴─────────┐         ┌──────────────────┐
   │UEnemySpawn       │◄─热度──│ULootHeatSubsystem│◄── ULootBagComponent
   │Subsystem (刷怪)  │ 压力    └──────────────────┘
   └──────────────────┘
            ▲
   ┌────────┴─────────┐   ┌──────────────────────────┐
   │ AEnemy (N个)      │   │ AExtractionZone          │──读条满──► URunOutcomeSubsystem.ReportExtracted
   └──────────────────┘   │ ALootContainer           │──Interact─► ULootBagComponent.TryAddLoot
                          └──────────────────────────┘
   URunOutcomeSubsystem.EndRun
        │
        ├─► UCampStockSubsystem (GameInstance, 跨局) : AddIron / AddRelics
        ├─► ULootBagComponent.ClearBag()
        └─► ULootHeatSubsystem.ResetHeat()
```

## 4. 关键依赖要点

1. **性能关键路径**：`UEnemyManagerSubsystem` 单 Tick 驱动全部敌人（AEnemy 零 Tick），是割草手感与帧率的核心；`UEnemySpawnSubsystem` 按热度压力 + 时间斜坡调度刷怪。
2. **"留着还是用掉"的闭环**：`ULootBagComponent`（拾取升温/使用降温）↔ `ULootHeatSubsystem`（压力）→ `UEnemySpawnSubsystem`（刷怪速率）→ 玩家承压 → 决定用遗物或带走。
3. **结算闭环**：`URunOutcomeSubsystem` 是唯一「局结束」裁决点，向 `UCampStockSubsystem`（跨局）入库，实现"带出去建营地"。
4. **解耦方式**：子系统间一律通过 `GetSubsystem<T>()` 运行时获取；组件间通过 `OwnerBag`/`OwnerHealth` 弱引用 + 委托（`OnDied`/`OnEnemyKilled` 等）通信，避免硬耦合。
5. **数据驱动解耦**：所有调优数值经 `ULootConfig`（DataAsset）/`FEnemyTuning`/`FSwarmTuning`/`FSpawnTuning` 配置，代码不写死平衡数值。

## 5. 仓库层的依赖

- **设计 → 实现**：`design/prd/master-prd.md`（范围）→ `design/gdd/*.md`（系统）→ `production/epics/directors-cut/story-*.md`（故事）→ `Game_01 5.8/Source/**`（代码）。
- **工具链 → 运行时**：`project.yaml`（配置唯一事实源）被 `.claude/skills/**` 经 `resolve_config` 读取；`CLAUDE.md` 决定 agent 行为与审批边界。
- **引擎参考 → 实现**：`docs/engine-reference/unreal/` 为 skill/agent 提供版本锁定 API，指导 C++ 写法。
