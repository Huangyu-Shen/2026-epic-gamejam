# 关键类与函数

只列读代码时必须知道的类、签名、边界和"为什么"。路径省略前缀 `Game_01 5.8/Source/Game_01/`，类名链接到实现文件。界面类见 [ui.md](ui.md)，存档见 [data-and-storage.md](data-and-storage.md)。

## 1. 英雄 `AHeroCharacter`

实现：[HeroCharacter.cpp](../../Game_01%205.8/Source/Game_01/Private/Player/HeroCharacter.cpp)，头文件 [HeroCharacter.h](../../Game_01%205.8/Source/Game_01/Public/Player/HeroCharacter.h)。

| 成员 | 行为 | 边界 |
|---|---|---|
| `bool CanAct() const`（私有） | 一局进行中、没死、没有待选升级时才为 true | 没有 `URunOutcomeSubsystem` 时返回 false |
| `void StartRun()`（私有） | 清敌人、重置击杀数和刷怪、清热度、清背包、设最大生命 = 基础 + Clinic 加成、重置成长、重置闪避、`StartNewRun`、应用胶片卡 | `BeginPlay` 里调用一次；没有"再来一局"按键，下一局靠重新加载跑图地图 |
| `void CreateHud()`（私有） | 创建 `URunHudWidget`，设为 `SelfHitTestInvisible`（只有按钮吃点击），显示鼠标，输入模式 `GameAndUI` 不锁鼠标 | 控制器为空时回退到 `GetPlayerController(0)`；已有 HUD 不重复创建 |
| `bool TryDodge()` | 方向为当前 WASD 方向，没按键则朝向；冲刺 0.18 秒、速度 1800、冷却 0.8 秒、无敌 0.25 秒 | `!CanAct()`、冲刺中、冷却中返回 false |
| `bool SearchNearestContainer()` | 在 `InteractRange`（250）内找最近的未搜索 `ALootContainer`，调 `Interact` | 背包满则返回 false，容器保持可搜 |
| `float ReceiveEnemyDamage(float)` | 先乘背包 buff 的 `DamageTakenMultiplier`，再交给 `UHealthComponent::ApplyDamage` | 返回实际伤害，无敌时为 0 |
| `void HandleDied()` | 绑定 `OnDied`，调 `ReportPlayerDied` | — |
| `Tick` | 读 WASD（`IsInputKeyDown`），移速 = `BaseWalkSpeed × 背包移速倍率 × 被动移速倍率`；冲刺期间用带扫掠的 `AddActorWorldOffset` 代替普通移动 | `CanAct()` 为 false 时输入清零 |

按键：Space 闪避、E 搜索、1 到 6 使用对应背包槽、K 自杀（调试）。1 到 3 同时是升级三选一的快捷键，`UseRelicSlot` 在 `CanAct()` 为 false（即升级面板出现）时直接返回，所以不会冲突。

## 2. 背包与遗物 `ULootBagComponent`

实现：[LootBagComponent.cpp](../../Game_01%205.8/Source/Game_01/Private/Loot/LootBagComponent.cpp)。

| 函数 | 行为 | 错误或边界 |
|---|---|---|
| `EBagAddResult CanAdd(int32 Iron, bool bWithRelic)` | 不修改，检查放得下吗 | `Iron < 0`，或 `Iron == 0` 且没有遗物 → `InvalidItem`；超过容量 → `BagFull` |
| `TryAddLoot(int32 Iron, bool bAddRelic, ERelicType)` | 原子地加 Iron 和最多一个遗物；有遗物则 `AddRarePickupHeat` | 失败时什么都不加，发提示 `Pickup refused: ...`、广播 `OnPickupRefused` |
| `bool UseRelic(int32 Index)` | 移除遗物，按 `FRelicRow` 加 buff，`ReduceHeatForRelicUse`，提示 `Used relic: ...` | 下标无效返回 false；没有对应数据行时只警告，不加 buff 但遗物仍被消耗 |
| `FLootStatModifier GetActiveModifier() const` | 所有活跃 buff 的三个倍率相乘 | 无 buff 全部为 1.0 |
| `ClearBag()` | 清 Iron、遗物、buff，停 Tick | 不动热度（热度由结算单独重置） |
| `TickComponent` | 移除到期 buff；没有限时 buff 时关闭自己的 Tick | 默认关闭，只在有 buff 时开 |

默认遗物数值（[LootConfig.cpp](../../Game_01%205.8/Source/Game_01/Private/Loot/LootConfig.cpp) 内联默认）：Swiftstep 移速 ×1.3、Warbrand 伤害 ×1.5、Aegis 受伤 ×0.6，均持续 20 秒。容器：补给箱 Iron 3 到 6、遗物概率 0.2；宝库 Iron 10 到 20、遗物概率 1.0。

## 3. 热度 `ULootHeatSubsystem`

| 函数 | 行为 |
|---|---|
| `float GetSpawnPressureMultiplier() const` | `Lerp(SpawnPressureAtZeroHeat, SpawnPressureAtMaxHeat, 热度/最大热度)`，默认 1.0 到 3.0 |
| `AddHeat(float)` | 夹在 0 到 `MaxHeat`，变化为 0 则不广播 `OnHeatChanged` |
| `AddRarePickupHeat()` / `ReduceHeatForRelicUse()` | 默认 +15 / -8 |
| `SetTuning` | 背包 `BeginPlay` 时把自己的配置推进来；因此热度参数其实来自 `ULootConfig`，不在 `Director Tuning` 里 |

## 4. 生命值 `UHealthComponent`

| 函数 | 行为 |
|---|---|
| `float ApplyDamage(float)` | 已死、无敌、上帝模式或 `Amount <= 0` 返回 0；否则扣血，死亡广播 `OnDied`，没死则给 `HitInvulnerabilitySeconds`（0.25）无敌 |
| `Kill()` | 无视上帝模式和无敌直接清零并广播 |
| `SetMaxHealth(float)` | 设最大值（至少 1）、回满、清无敌；Clinic 加成用它 |
| `IncreaseMaxHealth(float)` | 加上限并同量治疗；Vitality 被动用它 |
| `GetBaseMaxHealth()` | `BeginPlay` 时从 `HealthTuningTable` 的 `Hero` 行读出并记下，所以 Clinic 加成不会叠加 |

## 5. 自动武器 `UAutoWeaponComponent`

[AutoWeaponComponent.cpp](../../Game_01%205.8/Source/Game_01/Private/Combat/AutoWeaponComponent.cpp)。`TickComponent` 在 `bEnabled`、无待选升级、没死、一局进行中时，遍历三种武器，等级 > 0 且冷却到则开火。

伤害公式：`(Row.Damage + Row.DamagePerLevel × (等级 - 1) + Smithy 加成) × 背包伤害倍率 × 被动伤害倍率`。

| 武器 | 默认数值（`DefaultWeapon`） | 命中规则 |
|---|---|---|
| Blade | 伤害 15，每级 +8，间隔 0.4 秒，范围 600 | 最近的一个敌人 |
| Bolt | 伤害 20，每级 +10，间隔 0.9 秒，范围 1200，半宽 80 | 朝最近敌人（没有则朝向）的一条线，穿透 |
| Nova | 伤害 25，每级 +12，间隔 1.6 秒，范围 350 | 半径内全部敌人 |

开火时用 `DrawDebugLine/Circle` 画 0.12 秒的示意，这些调用在发布构建里被引擎编译掉。

## 6. 敌人

### `UEnemyManagerSubsystem::Tick`

[EnemyManagerSubsystem.cpp](../../Game_01%205.8/Source/Game_01/Private/Enemies/EnemyManagerSubsystem.cpp)。三遍：

1. **追击与伤害**：朝英雄在 XY 平面移动，在 `ContactRange × StopRangeFraction`（默认 0.8）处停下，所以敌人围成一圈而不是叠在一起；`Dist <= ContactRange` 且冷却到则 `ReceiveEnemyDamage`。一局结束后（`bCanHurt` 为 false）仍会移动，只是不再伤人。
2. **分格**：按 `SeparationCellSize`（默认 250）把位置放进网格，作用是把两两检测从 O(n²) 降到只查 3×3 邻格。
3. **推开**：重叠的两个身体沿连线方向互相推，速度 = 重叠量 × `SeparationStrength`（8），上限 `MaxSeparationSpeed`（400）。完全重合时用下标的 `cos/sin` 得到一个稳定的伪随机方向。

升级面板出现时整个 Tick 直接返回。

### `UEnemySpawnSubsystem`

| 函数 | 行为 |
|---|---|
| `float GetCurrentSpawnRate() const` | `(BaseRate + RampPerMinute × RunTime/60) × 热度压力`，默认 0.8 + 1.5/分钟 |
| `Tick` | 累加速率，每满 1 个刷一个 Grunt，直到 `MaxAlive`（200）；到上限时把累加器压到 ≤ 1，避免空位出现时一次爆发 |
| `int32 SpawnHorde(int32 Count, bool bIgnoreCap)` | 在英雄周围圆环（半径 1500 到 1900）刷一批；返回实际刷的数量 |
| `AEnemy* SpawnBoss()` | 刷一个 Boss，无视上限；没有英雄返回 null |

Boss 默认值写在构造函数里：生命 1500、移速 180、伤害 35、范围 220、冷却 1.2、缩放 2.5。Grunt：生命 25、移速 330、伤害 10、范围 90、冷却 0.8、缩放 0.6。

## 7. 局内成长 `URunProgressionSubsystem`

[RunProgressionSubsystem.cpp](../../Game_01%205.8/Source/Game_01/Private/Progression/RunProgressionSubsystem.cpp)。

| 函数 | 行为 |
|---|---|
| `HandleEnemyKilled` | 在击杀位置生成 `AXpPickup`，值 = `XpPerKill`（5），Boss ×10；待选升级期间不生成 |
| `Tick` | 倒序遍历拾取物：`CollectRadius`（80）内领取并 `GrantXp(值 × Greed 倍率)`，`MagnetRadius`（600）内以 `MagnetSpeed`（900）吸向英雄 |
| `CheckLevelUp` | `Xp >= XpToNext` 时升级，`XpToNext = BaseXp × Growth^(Level-1)`（30，×1.3），然后 `GenerateOffer` |
| `GenerateOffer` | 候选池 = 未满级的武器和被动；洗牌取最多 3 个；池空则不进入待选状态 |
| `PickOffer(int32 Index)` | 无待选或下标无效时无操作；应用后继续检查是否又够升一级 |
| `GetOfferDescriptions() const` | 界面用的文字，如 `NEW weapon: Bolt`、`Swiftness  Lv0 -> Lv1` |
| `ResetProgression()` | 等级 1、清 XP、起手 Blade 1 级、清被动、销毁遗留拾取物 |

被动默认：Swiftness 每层移速 +10%、Might 伤害 +15%、Vitality 最大生命 +20 并即时治疗、Greed 经验 +25%，每个最多 5 层。

## 8. 一局 `URunOutcomeSubsystem`

| 函数 | 行为 |
|---|---|
| `ReportExtracted(ULootBagComponent*)` | 以 `Extracted` 结束 |
| `ReportPlayerDied(ULootBagComponent*)` | 以 `Died` 结束 |
| `EndRun`（私有） | 已结束则忽略；`FRunSettlement::Compute`；`AddIron` 和 `AddRelics` 进库存；`SaveNow`；清背包；重置热度；广播 `OnRunEnded` |
| `StartNewRun()` | 重置状态，保留 `EarningsMultiplier` |
| `SetEarningsMultiplier(float)` | 至少 0；由 `UFilmCardSubsystem` 在开局设置 |

## 9. 撤离区 `AExtractionZone`

[ExtractionZone.cpp](../../Game_01%205.8/Source/Game_01/Private/Extraction/ExtractionZone.cpp)。占用者是带 `ULootBagComponent` 的 Pawn。进度 = 进度 + `DeltaSeconds / ExtractionSeconds`，满则 `ReportExtracted`。离开：`bResetProgressOnLeave`（默认 true）为真则清零并广播 `OnExtractionInterrupted`，否则每秒衰减 `ProgressDecayPerSecond`。关卡开始时已站在里面的 Pawn 在下一帧通过 `SetTimerForNextTick` 补登记，因为初始重叠不触发事件。进度只通过 `GetProgress()` 和 `OnExtractionProgress` 对外，HUD 读前者。

## 10. 营地

### `UCampMetaSubsystem`

| 函数 | 行为 |
|---|---|
| `int32 GetUpgradeCost(EBuildingType)` | `floor(BaseCost × CostGrowth^当前等级)`，默认 10 × 2^等级 |
| `bool TryUpgrade(EBuildingType, FText& Reason)` | 已满级、Iron 不足时返回 false 并给出文字（`Not enough Iron (need N)`）；成功则扣 Iron、升级、广播 `OnMetaChanged` |
| `float GetWeaponDamageBonus() const` | Smithy 等级 × `DamagePerLevel`（默认 5） |
| `float GetMaxHealthBonus() const` | Clinic 等级 × `MaxHealthPerLevel`（默认 25） |
| `int32 DismantleRelic(int32 Index)` | 移除仓库遗物并加 `RelicDismantleIron`（8，写死）；下标无效返回 0 |
| `RestoreLevels(const TMap<…>&)` | 存档读取用，每个等级夹到 0 到 `MaxLevel`，不在 `Buildings` 里的键被丢弃 |
| `TArray<FName> PendingFilmCards` | 局前选的卡，跨关卡保留，不存盘 |

### `UCampStockSubsystem`

`AddIron`（非正数忽略）、`AddRelics`、`SpendIron`（不足返回 false、什么都不改）、`RemoveRelicAt`、`RestoreState`（Iron 夹到 ≥ 0）。每次变化都广播 `OnStockChanged`。

## 11. 胶片卡 `UFilmCardSubsystem`

| 函数 | 行为 |
|---|---|
| `AddCard(FName)` | 槽满（2）、已选、或不在表里则 false |
| `ApplyActiveCards()` | `ForceHorde` 调 `SpawnHorde(HordeCount, true)`，`ForceBoss` 调 `SpawnBoss()`，再把所有卡的倍率相乘设给 `URunOutcomeSubsystem` |
| `ResetCards()` | 清槽并把倍率设回 1.0；一局结束时自动调用 |
| `DescribeCard(FName)` | 如 `HordeNight: a horde of 40 grunts at the start, Iron earned x1.50` |
| 持久化 | `Initialize` 从 `PendingFilmCards` 恢复，`AddCard`、`RemoveCard`、`ResetCards` 写回 |

默认两张卡的数值在 `DT_FilmCards` 里：`HordeNight`（40 个 Grunt，×1.5）和 `CutKnight`（Boss，×2.0）。数值以截图和 `DescribeCard` 输出为准。

## 12. 性能探针 `ADirectorGameMode`

命令行 `-PerfProbe`：延迟 3 秒后英雄上帝模式、关武器、关调度、`SpawnHorde(150, true)`，采样 10 秒，输出一行 `PERFPROBE enemies=… avgFPS=… minFPS=… samples=…` 到 `LogEnemies`，然后退出。无此参数时 `Tick` 不启用。
