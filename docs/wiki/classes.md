# 关键类与函数说明

按模块逐类说明职责、关键属性（`UPROPERTY`）与关键方法（`UFUNCTION`/虚函数）。文件路径均为 `Game_01 5.8/Source/Game_01/` 下。

---

## 1. Player — `AHeroCharacter`

**文件**：`Public/Player/HeroCharacter.h` / `Private/Player/HeroCharacter.cpp`
**基类**：`ACharacter`。玩家角色：斜俯视相机、原始按键、闪避、交互、受击与死亡。

### 关键组件

| 属性 | 类型 | 说明 |
|---|---|---|
| `Bag` | `ULootBagComponent*` | 本局背包（Iron + 遗物 + buff） |
| `Health` | `UHealthComponent*` | 生命值 |
| `Weapon` | `UAutoWeaponComponent*` | 自动武器 |
| `BodyMesh` | `UStaticMeshComponent*` | 灰盒身体（引擎圆柱） |
| `CameraBoom`/`Camera` | `USpringArmComponent*`/`UCameraComponent*` | 固定斜俯视相机（-60° 俯角，不随 Pawn 旋转） |

### 关键属性

| 属性 | 默认 | 说明 |
|---|---|---|
| `InteractRange` | 250 | E 键搜索容器的半径（cm） |
| `BaseWalkSpeed` | 600 | 基础移速（遗物 buff 前） |
| `CameraDistance` | 1400 | 相机高度 |
| `Dodge` | `FDodgeTuning` | 闪避调优（时长/速度/冷却/无敌秒） |
| `DebugHordeSize` | 40 | H 调试键刷的尸潮数量 |

### 关键方法

| 方法 | 说明 |
|---|---|
| `SearchNearestContainer()` | 在 `InteractRange` 内找最近未搜索容器并 `Interact`，返回是否成功 |
| `TryDodge()` | 朝移动方向闪避；冷却/已闪避/不可行动时返回 false；闪避开始授予无敌帧 |
| `ReceiveEnemyDamage(Amount)` | 敌人接触伤害入口：乘以背包 `DamageTakenMultiplier` 后交给 `Health->ApplyDamage` |
| `IsDead()` | 委托给 `Health->IsDead()` |
| `BeginPlay()` | 绑定 `Health->OnDied` → `HandleDied`；给身体染色（蓝） |
| `Tick(DeltaSeconds)` | 读原始 WASD → 应用移动/闪避 → 更新移速倍率 → 打印调试状态 |
| `SetupPlayerInputComponent(...)` | 绑定 Space/E/K/N 与 H/B/1–6 键 |
| `HandleDied()` | 死亡时调用 `URunOutcomeSubsystem::ReportPlayerDied(Bag)` |
| `StartNewRunKey()` | 复位敌人/杀敌数/刷怪/热度/背包/生命 → `StartNewRun` |
| `CanAct()` | 局活跃且未死亡时返回 true（移动/交互/用遗物都受此门控） |

> **调试键**：K 死亡、N 新局、H 尸潮（`SpawnHorde(DebugHordeSize)`）、B Boss（`SpawnBoss()`）。

---

## 2. Combat — `UAutoWeaponComponent`

**文件**：`Public/Combat/AutoWeaponComponent.h`
**基类**：`UActorComponent`。起始武器：每 `Interval` 秒自动攻击范围内最近敌人。

### 关键属性

| 属性 | 默认 | 说明 |
|---|---|---|
| `Tuning` | `FWeaponTuning` | Damage 15 / Interval 0.4 / Range 600 / 命中线显示参数 |
| `bEnabled` | true | 开关（性能探针会关闭以保敌人存活） |
| `OwnerBag` | 瞬态 | 主人背包（读伤害倍率） |
| `OwnerHealth` | 瞬态 | 主人生命（死亡停火） |

### 关键方法

| 方法 | 说明 |
|---|---|
| `TickComponent(...)` | 冷却结束后在 `Range` 内找最近敌人并 `ApplyDamage(Damage × 背包伤害倍率)`，绘制调试命中线 |

---

## 3. Combat — `UHealthComponent`

**文件**：`Public/Combat/HealthComponent.h`
**基类**：`UActorComponent`。生命值 + 受击无敌窗口 + god-mode。

### 关键属性

| 属性 | 默认 | 说明 |
|---|---|---|
| `Tuning` | `FHealthTuning` | MaxHealth 100 / HitInvulnerabilitySeconds 0.25 |
| `bGodMode` | false | 无敌开关 |
| `OnDied` | 委托 | 生命归零时触发一次 |
| `OnDamaged` | 委托 | 每次命中触发（参数：实际伤害、新生命值） |

### 关键方法

| 方法 | 说明 |
|---|---|
| `ApplyDamage(Amount)` | 施加伤害；死亡/无敌/受击无敌期内忽略；存活命中开启受击无敌窗口；返回实际伤害（0 表示被忽略） |
| `Kill()` | 无视无敌/god-mode 直接归零并触发 `OnDied` |
| `ResetHealth()` | 回满并清无敌（新局） |
| `GrantInvulnerability(Seconds)` | 授予免疫（取更长窗口） |
| `IsDead()`/`IsInvulnerable()`/`GetHealth()`/`GetMaxHealth()` | 只读查询 |

---

## 4. Enemies — `AEnemy`

**文件**：`Public/Enemies/Enemy.h`
**基类**：`AActor`。灰盒敌人：引擎方块、无碰撞、无 Tick、无移动组件（移动由管理器统一驱动）。

### 关键属性

| 属性 | 说明 |
|---|---|
| `Tuning` | `FEnemyTuning`（该敌人参数行，`Init` 设置） |
| `NextContactTime` | 下次可接触伤害的世界时间（由管理器写入） |
| `Body` | 引擎方块网格（`UStaticMeshComponent`） |
| `EngineCubeSize` | 静态常量 100（方块边长 cm） |

### 关键方法

| 方法 | 说明 |
|---|---|
| `Init(InTuning)` | 应用参数行：缩放、生命值（生成后立即调用） |
| `ApplyDamage(Amount)` | 减生命；归零时报给管理器并销毁 |
| `GetBodyRadius()` | 半身宽（用于推挤分离） |
| `EndPlay(...)` | 反注册（`UnregisterEnemy`） |

---

## 5. Enemies — `UEnemyManagerSubsystem`

**文件**：`Public/Enemies/EnemyManagerSubsystem.h`
**基类**：`UTickableWorldSubsystem`。持有全部存活敌人并**在一个 Tick** 内驱动所有敌人。

### 关键属性

| 属性 | 说明 |
|---|---|
| `Tuning` | `FSwarmTuning`（StopRangeFraction/SeparationCellSize/SeparationStrength/MaxSeparationSpeed） |
| `OnEnemyKilled` | 委托（story 004 用于经验） |
| `Enemies`/`KillCount` | 存活列表 / 击杀计数 |
| 复用 scratch buffer | `Positions`/`Yaws`/`Pushes`/`Grid`（避免每 Tick 分配） |

### 关键方法

| 方法 | 说明 |
|---|---|
| `RegisterEnemy`/`UnregisterEnemy` | 加入/移出群体 |
| `NotifyEnemyKilled(Enemy)` | 击杀计数 + 广播 `OnEnemyKilled` |
| `FindNearestEnemy(Location, Range)` | 范围内最近存活敌人（供武器使用） |
| `GetAliveCount()`/`GetKillCount()` | 只读统计 |
| `ClearAll()` | 销毁全部敌人（不计数） |
| `ResetKillCount()` | 击杀计数归零（新局） |
| `Tick(DeltaTime)` | 三段式：① 追击+朝向+接触伤害；② 网格分桶；③ 3×3 邻域推挤分离，最后统一写回位置 |

> **性能设计**：敌人零 Tick，全部移动/碰撞集中在管理器单 Tick，避免数百 Actor 各自 Tick 的开销。

---

## 6. Enemies — `UEnemySpawnSubsystem`

**文件**：`Public/Enemies/EnemySpawnSubsystem.h`
**基类**：`UTickableWorldSubsystem`。波次调度器（刷怪）。

### 关键属性

| 属性 | 说明 |
|---|---|
| `Tuning` | `FSpawnTuning`（BaseRate/RampPerMinute/MaxAlive/RingMinRadius/RingMaxRadius） |
| `GruntTuning`/`BossTuning` | Grunt/Boss 参数行 |
| `RunTime`/`SpawnAccumulator` | 活跃局时长 / 刷怪累加器 |

### 关键方法

| 方法 | 说明 |
|---|---|
| `SpawnHorde(Count, bIgnoreCap=false)` | 环绕英雄刷一波 Grunt，受 `MaxAlive` 限制（除非忽略），返回实际刷出数 |
| `SpawnBoss()` | 在环上刷一个 Boss（忽略上限），无英雄返回 null |
| `GetCurrentSpawnRate()` | 当前每秒刷怪率：`(BaseRate + RampPerMinute×分钟) × 热度压力倍率` |
| `GetRunTime()` | 活跃局时长 |
| `SetSchedulerEnabled(bool)` | 开关调度（性能探针用） |
| `ResetRun()` | 清零局时长与累加器（不删敌人） |
| `Tick(DeltaTime)` | 按速率累加并在环形带 `SpawnOnRing` 刷 Grunt |

---

## 7. Loot — `ULootBagComponent`

**文件**：`Public/Loot/LootBagComponent.h`
**基类**：`UActorComponent`。玩家本局背包：Iron + 遗物共享硬槽位；实现"留着还是用掉"。

### 关键属性

| 属性 | 说明 |
|---|---|
| `Config` | `ULootConfig*`（未设置则用内联默认） |
| `OnBagChanged`/`OnPickupRefused`/`OnRelicUsed`/`OnBuffsChanged` | 变化委托 |
| `IronCount`/`Relics`/`ActiveBuffs` | 内部状态（Iron 数 / 遗物列表 / 激活 buff 列表） |

### 关键方法

| 方法 | 说明 |
|---|---|
| `CanAdd(Iron, bWithRelic)` | 检查是否放得下（不真正添加） |
| `TryAddLoot(Iron, bAddRelic, RelicType)` | 原子添加 Iron + 可选遗物；失败则什么都不加并触发 `OnPickupRefused`；加遗物会升温 |
| `TryAddIron`/`TryAddRelic` | 便捷封装 |
| `UseRelic(Index)` | 移除该槽遗物、施加 buff、降温；索引无效返回 false |
| `ClearBag()` | 清空背包与 buff（死亡/结算用，**不**动热度） |
| `GetIronCount()`/`GetRelics()`/`GetRelicCount()`/`GetUsedSlots()`/`GetCapacitySlots()`/`IsFull()` | 只读内容 |
| `GetActiveModifier()` | 合并所有激活 buff 的 `FLootStatModifier`（无 buff 时倍率全 1.0） |
| `GetActiveBuffs()` | 激活 buff 列表（副本） |
| `GetAddResultText(Result)` | 结果的人类可读文本（HUD/调试） |

---

## 8. Loot — `ULootHeatSubsystem`

**文件**：`Public/Loot/LootHeatSubsystem.h`
**基类**：`UWorldSubsystem`。全局本局热度。

### 关键方法

| 方法 | 说明 |
|---|---|
| `SetTuning(InTuning)` | 背包在 BeginPlay 时把配置推给它 |
| `GetHeat()`/`GetHeatNormalized()` | 当前热度 / 归一化 0..1 |
| `GetSpawnPressureMultiplier()` | 刷怪压力倍率：按归一化热度从 `SpawnPressureAtZeroHeat` 插值到 `SpawnPressureAtMaxHeat` |
| `AddHeat(Amount)`/`AddRarePickupHeat()`/`ReduceHeatForRelicUse()`/`ResetHeat()` | 增减/升温/降温/归零（均钳制到 0..MaxHeat） |
| `OnHeatChanged` | 委托（参数：新热度、增量） |

---

## 9. Loot — `ALootContainer`

**文件**：`Public/Loot/LootContainer.h`
**基类**：`AActor`。补给箱/宝库，每容器可搜一次。

### 关键属性

| 属性 | 说明 |
|---|---|
| `ContainerType` | 使用哪一行容器数据 |
| `ConfigOverride` | 可选覆盖配置（否则用互动者背包的配置） |
| `Mesh` | 引擎方块（搜索后压扁表示已打开） |
| `OnSearched` | 委托（参数：容器、Iron、是否得遗物、遗物类型） |

### 关键方法

| 方法 | 说明 |
|---|---|
| `Interact(InInstigator)` | 代表互动者搜索；已搜/无背包/背包拒绝时返回 false；掷出的结果被记住，重试不会重新掷 |
| `HasBeenSearched()` | 只读 |

---

## 10. Loot — `ULootConfig`

**文件**：`Public/Loot/LootConfig.h`
**基类**：`UDataAsset`。数据驱动战利品配置，各 DataTable 可选，缺省回退到内联默认值（无内容也能跑）。

### 关键属性

| 属性 | 说明 |
|---|---|
| `RelicTable`/`ContainerTable`/`TuningTable` | 可选 DataTable |
| `DefaultTuning`/`DefaultRelics`/`DefaultContainers` | 内联默认值 |

### 关键方法

| 方法 | 说明 |
|---|---|
| `GetTuning()` | 有效调优（表行优先，否则默认） |
| `GetRelicRows()`/`FindRelicRow(Type, Out)`/`FindContainerRow(Type, Out)` | 取有效行 |
| `PickRandomRelic(OutType)` | 按 DropWeight 加权随机遗物类型 |

---

## 11. Extraction — `AExtractionZone`

**文件**：`Public/Extraction/ExtractionZone.h`
**基类**：`AActor`。撤离点（灰盒圆盘 + 盒子触发体）。

### 关键属性

| 属性 | 说明 |
|---|---|
| `Tuning` | `FExtractionTuning`（ExtractionSeconds 5 / bResetProgressOnLeave / ProgressDecayPerSecond） |
| `OnExtractionProgress`/`OnExtractionInterrupted` | 读条委托 |
| `DiscMesh`/`ZoneVolume` | 圆盘网格 / 触发盒 |
| `Occupant` | 当前撤离者背包（弱引用） |

### 关键方法

| 方法 | 说明 |
|---|---|
| `GetProgress()` | 读条值 0..1 |
| `Tick(DeltaSeconds)` | 有撤离者时读条递增，满后调用 `ReportExtracted`；仅在有人或读条衰减时 Tick |
| `HandleBeginOverlap`/`HandleEndOverlap` | 进出触发体 |
| `TryEnter(OtherActor)`/`Interrupt()` | 占位 / 中断并清读条 |

---

## 12. Extraction — `URunOutcomeSubsystem`

**文件**：`Public/Extraction/RunOutcomeSubsystem.h` / `Private/Extraction/RunOutcomeSubsystem.cpp`
**基类**：`UWorldSubsystem`。本局状态机，**恰好结束一次**。

### 关键属性

| 属性 | 说明 |
|---|---|
| `OnRunEnded` | 委托（参数 `FRunResult`） |
| `bRunActive`/`Outcome`/`LastResult` | 局状态 |
| `EarningsMultiplier` | 结算倍率（story 005 的卡片会设置） |

### 关键方法

| 方法 | 说明 |
|---|---|
| `IsRunActive()`/`GetOutcome()`/`GetLastResult()`/`GetEarningsMultiplier()` | 只读 |
| `SetEarningsMultiplier(NewMultiplier)` | 设置结算倍率（钳到 ≥0） |
| `ReportExtracted(Bag)` | 成功撤离：`EndRun(Bag, Extracted)` |
| `ReportPlayerDied(Bag)` | 死亡：`EndRun(Bag, Died)` |
| `StartNewRun()` | 复位局状态（保留倍率） |
| `EndRun(Bag, Outcome)` | 私有核心：算结算 → 营地入库 → 清背包 → 重置热度 → 打印/广播结果 |

---

## 13. Extraction — `UCampStockSubsystem`

**文件**：`Public/Extraction/CampStockSubsystem.h`
**基类**：`UGameInstanceSubsystem`。营地 Iron 与仓库遗物（整个游戏会话跨局保留）。

### 关键方法

| 方法 | 说明 |
|---|---|
| `AddIron(Amount)`/`AddRelics(NewRelics)` | 入库（非正/空列表忽略） |
| `GetIron()`/`GetWarehouseRelics()` | 只读 |
| `OnStockChanged` | 委托（Iron 或仓库变化时） |

> **持久化**：当前为内存态；持久化留待 story 007，营地 UI/建筑留待 story 006。

---

## 14. Extraction — `FRunSettlement`

**文件**：`Public/Extraction/ExtractionTypes.h`
**纯静态结算数学**（不触碰 UObject，可脱离编辑器单测）。

```cpp
static FRunResult Compute(int32 BagIron, const TArray<ERelicType>& BagRelics,
                          float EarningsMultiplier, ERunOutcome Outcome);
```

- **Extracted**：`IronGained = floor(BagIron × EarningsMultiplier)`，全部遗物入库（`RelicsBanked`）。
- **Died**：一无所获，整包报为丢失（`IronLost` + `RelicsLost`）。
- **None**：空结果。

---

## 15. Game — `ADirectorGameMode`

**文件**：`Public/Game/DirectorGameMode.h`
**基类**：`AGameModeBase`。默认 Pawn 为英雄；`-PerfProbe` 触发性能探针。

### 关键属性

| 属性 | 说明 |
|---|---|
| `PerfProbe` | `FPerfProbeTuning`（StartDelaySeconds 3 / SampleSeconds 10 / EnemyCount 150） |

### 关键方法

| 方法 | 说明 |
|---|---|
| `BeginPlay()` | 检测 `-PerfProbe` 命令行，有则启动探针 |
| `Tick(DeltaSeconds)` | 采样帧时间，采样结束输出 `PERFPROBE` 日志并退出 |
| `StartPerfProbe()` | 英雄 god-mode、`SpawnHorde(EnemyCount, 忽略上限)`、开始采样 |

---

## 16. StandIn — `AStandInCharacter` / `AStandInGameMode`（可弃）

**文件**：`Public/StandIn/*.h`
story 001 的一次性占位 Pawn/GameMode，已被 story 003 替换。`AStandInCharacter` 提供 WASD/E/1–6/K/N，`AStandInGameMode` 默认 Pawn 指向它。仅供历史参考。

---

## 17. 数据结构汇总（`USTRUCT`）

| 结构 | 模块 | 用途 |
|---|---|---|
| `FDodgeTuning` | Player | 闪避时长/速度/冷却/无敌 |
| `FWeaponTuning` | Combat | 武器伤害/间隔/射程 |
| `FHealthTuning` | Combat | 生命上限/受击无敌 |
| `FEnemyTuning` | Enemies | 单怪：血量/移速/接触伤害/射程/冷却/缩放 |
| `FSwarmTuning` | Enemies | 群体：停距/分离网格/分离强度/上限 |
| `FSpawnTuning` | Enemies | 刷怪：基础速率/每分钟递增/上限/环形带 |
| `FLootStatModifier` | Loot | 移速/伤害/受伤倍率（`Stack()` 合并） |
| `FRelicRow` | Loot | 遗物定义行 |
| `FLootContainerRow` | Loot | 容器产出定义行 |
| `FLootTuning` | Loot | 背包/热度调优 |
| `FActiveRelicBuff` | Loot | 当前生效 buff |
| `FExtractionTuning` | Extraction | 撤离读条规则 |
| `FRunResult` | Extraction | 结算结果 |
| `FPerfProbeTuning` | Game | 性能探针参数 |

### 枚举汇总（`UENUM`）

| 枚举 | 值 |
|---|---|
| `EEnemyKind` | Grunt, Boss |
| `ERelicType` | Swiftstep（移速）, Warbrand（伤害）, Aegis（减伤） |
| `ELootContainerType` | SupplyCrate, Vault |
| `EBagAddResult` | Success, BagFull, InvalidItem |
| `ERunOutcome` | None, Extracted, Died |
