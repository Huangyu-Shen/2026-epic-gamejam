# Enemies & Spawning

> **Status**: Designed
> **Author**: 开发者本人 + Claude
> **Last Updated**: 2026-11-12
> **Last Verified**: 2026-11-12
> **Implements Pillar**: P1 Every pickup is a choice(热度让怪变多),P2 You direct the cut(片段卡决定精英/尸潮/Boss)

## Summary

定义所有敌人的数值和行为,以及一个"刷怪导演"如何决定何时、何地、刷多少怪。刷怪量由三件事共同决定:时间推移、背包热度、出发前插入的片段卡。

> **Quick reference** — Layer: `Core` · Priority: `MVP` · Key deps: `player-combat`, `loot-inventory-heat`, `directors-cut-editing`

## Overview

敌人从玩家视野外不断涌来,数量随时间增长。背包越值钱,热度越高,刷怪越快、怪越硬。片段卡可以加入周期性尸潮、精英巡逻和 Boss "The Cut Knight"。所有敌人行为都很简单:追玩家、碰撞伤害、少数远程。难度来自数量,不来自复杂 AI。

## Player Fantasy

"整部电影的群演都在冲我来。"压力看得见、摸得着,而且是我自己选的:插了尸潮卡、背包塞满宝物,就要承受后果。

## Detailed Design

### Core Rules

**刷怪导演**

1. 局内全程由一个刷怪导演按固定频率(每 0.25 s)结算一次刷怪。
2. 每次结算按当前刷怪速率(公式见下)累积"刷怪点数",点数 ≥ 1 时刷出一只怪,扣 1 点。
3. **刷新位置**:在玩家周围 1600–2200 cm 的圆环上随机取点,要求在镜头视野之外、在可行走区域上、不在封闭建筑内部。连续 5 次找不到合法点时,改用最近的预设刷怪点(见 `map-level.md`)。
4. **同屏上限**:常态 150 只,尸潮期间 200 只。达到上限后暂停刷新,点数继续累积,但最多存 20 点。
5. **回收**:离玩家超过 3500 cm 的普通怪,传送到玩家周围的刷新圆环上(血量保持不变)。精英和 Boss 不回收。
6. **怪种选择**:按当前开放的怪种权重表抽取(Must 档只有 Skeleton Grunt)。
7. **成长**:敌人生命按时间和热度放大(公式见下);移速和伤害不随时间增长,只受热度影响。

**敌人行为(通用)**

- 近战怪:直接朝玩家移动,绕开障碍物,彼此之间互相推挤避免重叠;碰到玩家造成接触伤害。
- 远程怪:与玩家保持一定距离,按间隔发射直线投射物,投射物可被闪避穿过。
- 所有敌人受击时被轻微击退(Boss 免疫),死亡时掉落经验宝石。

**敌人清单**

| 敌人 | 档位 | 生命 | 移速 cm/s | 伤害 | 经验 | 行为 |
|---|---|---|---|---|---|---|
| Skeleton Grunt | Must | 20 | 280 | 接触 8 | 1 | 近战追击 |
| Ghoul Runner | Should | 12 | 450 | 接触 6 | 1 | 近战追击,5 只一群刷出 |
| Bone Archer | Should | 15 | 250 | 箭 6 | 2 | 保持 800 cm 距离,每 2.5 s 射一箭,箭速 900 cm/s |

**怪种权重(Should 档全开时)**

| 时间 | Grunt | Runner | Archer |
|---|---|---|---|
| 0:00–2:00 | 100 | 0 | 0 |
| 2:00–5:00 | 60 | 30 | 10 |
| 5:00 以后 | 45 | 35 | 20 |

**精英(Should)**

- 任意怪种的强化版:生命 ×6、体型 ×1.5、伤害 ×1.3,带金色描边。
- 掉落经验 10,外加一个"精英宝箱"(相当于一个贮藏箱的掉落,见 `loot-inventory-heat.md`)。
- 来源:片段卡 Elite Patrol(每 60 s 一只),以及热度最高档 Infamous(每 45 s 一只)。两个来源独立计时。

**尸潮 Horde(Must,来自片段卡 Horde Night)**

- 1:30 开始,每 90 s 一次,每次持续 15 s。
- 尸潮期间刷怪速率 ×3,所有怪从同一个方向(以玩家为中心的 90° 扇形)涌来,方向每次随机。
- 开始前 3 s 在屏幕边缘显示来袭方向的警告。

**Boss: The Cut Knight(Must,来自片段卡 The Cut Knight)**

- 在 4:00 登场(如果插了 Short Cut 卡,改为 3:00)。登场前 5 s 全屏提示 "The Cut Knight has entered the scene."
- 从 Boss 场地(墓园)出发,走向玩家。
- 生命 2500(不随时间和热度放大),移速 300 cm/s,接触伤害 15,免疫击退。
- 技能循环(每个技能之间间隔 2 s):
  1. **Charge**:地面显示 1.0 s 的直线预警,然后冲刺 1200 cm,命中造成 25 伤害。
  2. **Ground Slam**:地面显示 1.2 s 的圆形预警(半径 500 cm),然后落下,造成 30 伤害。
  3. **Rally the Extras**(每 20 s 至多一次):召唤 10 只 Skeleton Grunt 围在 Boss 身边。
- 击败后掉落一堆"宝库级"战利品(1 件必得遗物 + 8 个材料,见 `loot-inventory-heat.md`)和 50 经验。
- Boss 不阻止撤离:玩家可以不打 Boss,直接撤。
- 第二阶段为 Could 档,默认不做。

### States and Transitions

| 敌人状态 | 说明 |
|---|---|
| Spawning(0.3 s) | 从地面升起的出场动画,不可受伤,也不造成伤害 |
| Chasing / Kiting | 正常行为 |
| Attacking | 远程怪射击、Boss 释放技能 |
| HitReact(0.1 s) | 闪白 + 击退 |
| Dying(0.5 s) | 死亡动画,掉落物生成,随后回收进对象池 |

### Interactions with Other Systems

| 系统 | 交互 |
|---|---|
| `loot-inventory-heat.md` | 读取当前热度档位,换算成刷怪和生命倍率;精英和 Boss 的掉落 |
| `directors-cut-editing.md` | 读取本局的卡组合,决定是否开启尸潮、精英巡逻、Boss,以及时长相关的时间表 |
| `run-progression.md` | 死亡掉落经验宝石 |
| `player-combat.md` | 接收武器伤害,对玩家造成伤害 |
| `map-level.md` | 预设刷怪点、Boss 场地、不可刷怪区 |
| `ui-hud.md` | 尸潮方向警告、Boss 血条、Boss 登场提示 |

## Formulas

**刷怪速率(只/秒)**

```
SpawnRate(t) = S0 × (1 + t / T_ramp) × HeatSpawnMult × HordeMult
```

| 变量 | 含义 | 默认 | 范围 |
|---|---|---|---|
| t | 本局已进行的秒数 | — | 0–840 |
| S0 | 开局刷怪速率 | 1.5 只/s | 0.8–3.0 |
| T_ramp | 速率翻倍所需的时间 | 120 s | 60–240 |
| HeatSpawnMult | 热度档位的刷怪倍率,定义在 `loot-inventory-heat.md` | 1.0–2.2 | — |
| HordeMult | 尸潮期间为 3,否则为 1 | 1 或 3 | — |

例:t = 300 s(5:00),热度档位 Hunted(×1.7),非尸潮 → `1.5 × (1 + 2.5) × 1.7 = 8.9 只/s`。实际会被 150 只的同屏上限压住。

**敌人生命**

```
EnemyHP = BaseHP × (1 + 0.15 × t / 60) × HeatHPMult
```

| 变量 | 含义 | 范围 |
|---|---|---|
| BaseHP | 怪种基础生命(表中的值;精英再 ×6) | 12–120 |
| HeatHPMult | 热度档位的生命倍率,定义在 `loot-inventory-heat.md` | 1.0–1.6 |

例:5:00 时 Skeleton Grunt、热度 Noticed(×1.15)→ `20 × 1.75 × 1.15 = 40.25`,取 40。此时起始武器 Sword Arc 3 级(16 伤害)需要砍 3 刀,玩家会感到明显的压力,从而推动选择升级或撤离。

**敌人伤害**

```
EnemyDamage = BaseDamage × (精英 ? 1.3 : 1.0)
```

不随时间增长,压力来自数量。

## Edge Cases

| 情况 | 处理 |
|---|---|
| 玩家站在地图角落,圆环大部分落在地图外 | 只在合法点上刷;连续失败则用预设刷怪点 |
| Boss 登场时玩家已经撤离 | 局已结束,Boss 不刷 |
| Boss 卡在地形里 | 每 5 s 检查一次,如果 3 s 内没有位移,传送到离玩家 1500 cm 的合法点 |
| 热度档位在尸潮中途变化 | 立即按新倍率计算,不必等尸潮结束 |
| 同屏达到上限时轮到 Boss 刷新 | Boss 不受上限限制,必定登场 |
| 精英的两个来源同时触发 | 刷两只,不合并 |

## Dependencies

- 上游:`loot-inventory-heat`(热度倍率)、`directors-cut-editing`(卡组合)、`map-level`(刷怪点和 Boss 场地)、`player-combat`(玩家位置和伤害)。
- 下游:`run-progression`(经验)、`loot-inventory-heat`(精英和 Boss 掉落)、`ui-hud`(警告和 Boss 血条)。

## Tuning Knobs

| 参数 | 默认值 | 安全范围 | 影响 |
|---|---|---|---|
| S0 | 1.5 只/s | 0.8–3.0 | 开局压力 |
| T_ramp | 120 s | 60–240 | 压力增长的速度 |
| 生命时间增长 | 0.15 / 分钟 | 0.05–0.3 | 后期怪物有多硬 |
| 同屏上限 | 150 | 100–250 | 性能与压力 |
| 刷新圆环 | 1600–2200 cm | 1200–3000 | 怪从哪里出现 |
| 回收距离 | 3500 cm | 2500–5000 | 怪是否总在身边 |
| 尸潮间隔 / 时长 / 倍率 | 90 s / 15 s / ×3 | 60–150 / 10–25 / ×2–4 | 尸潮卡的风险 |
| 精英间隔 | 60 s | 30–120 | 精英巡逻卡的风险 |
| Boss 生命 | 2500 | 1500–4000 | Boss 战时长(目标 40–70 s) |
| Boss 登场时间 | 4:00 | 2:30–6:00 | 玩家的准备时间 |

## Visual/Audio Requirements

- 每种敌人的轮廓和颜色可以一眼区分:Grunt 骨白,Runner 惨绿,Archer 深紫;精英金边。
- 远程攻击和 Boss 技能必须有地面预警(红色描边区域)。
- Boss 登场:镜头短暂拉远 1 s,低沉的鼓点进入。

## Game Feel

小怪"一刀一个",死亡反馈利落。中期开始需要两三下,让玩家感到局势在恶化。Boss 的技能预警清晰,躲开时要有"我躲过去了"的爽感。

## UI Requirements

尸潮方向警告、Boss 登场字幕、Boss 血条。见 `ui-hud.md`。

## Cross-References

- `design/gdd/loot-inventory-heat.md`(热度档位倍率表)
- `design/gdd/directors-cut-editing.md`(卡的风险和时间表)
- `design/gdd/map-level.md`(刷怪点、Boss 场地)

## Acceptance Criteria

- **AC1(Must)**:开局后 10 s 内,屏幕外持续有 Skeleton Grunt 出现并追击玩家。
- **AC2(Must)**:热度从 Calm 升到 Hunted 后,30 s 内的刷怪数量明显上升(日志统计 ≥ 1.5 倍)。
- **AC3(Must)**:插了 Horde Night 卡时,1:30 出现方向警告,随后 15 s 内怪从同一方向大量涌来。
- **AC4(Must)**:插了 The Cut Knight 卡时,4:00 Boss 登场;它的三个技能都有预警,击败后掉落含遗物的战利品堆。
- **AC5(Must)**:不插 Boss 卡时,整局都不出现 Boss。
- **AC6(Must)**:同屏敌人数从不超过 150(尸潮时 200)。
- **AC7(Should)**:插了 Elite Patrol 卡时,每 60 s 出现一只金边精英,击败后掉落精英宝箱。
- **AC8(Must)**:150 只敌人同屏时帧率 ≥ 60 FPS(开发机,1080p)。

## Open Questions

- 敌人寻路用导航网格还是直接追击加避障?交给实现计划,以性能为准。
