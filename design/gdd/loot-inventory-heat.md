# Loot, Inventory & Heat

> **Status**: Designed
> **Author**: 开发者本人 + Claude
> **Last Updated**: 2026-11-12
> **Last Verified**: 2026-11-12
> **Implements Pillar**: P1 Every pickup is a choice(核心系统)

## Summary

本游戏最有特色的系统。玩家在固定物资点搜出通用材料和稀有遗物,装进有限的背包。背包里东西的总价值就是**热度**:热度越高,怪越多越硬。遗物可以在局内当场使用,换取能逆转局势的效果,同时降低热度;也可以留着带出去,在营地分解成大量材料。

> **Quick reference** — Layer: `Core` · Priority: `MVP` · Key deps: `map-level`, `player-combat`, `enemies-spawning`, `camp-meta`

## Overview

- **材料**:Iron(铁料)、Herbs(药草)、Arcane Dust(奥术尘)。每种对应地图上的一类区域,玩家知道去哪儿拿什么。
- **遗物 Relics**:稀有物品。局内可以使用,也可以带出去分解。
- **背包**:按格子计算容量,材料可以堆叠。
- **热度 Heat**:背包总价值换算成 4 个档位,直接驱动刷怪导演。

## Player Fantasy

"包越鼓,心越慌。"每捡一件宝贝都在加码:现在用掉,稳稳活下来;还是赌一把,背着它冲向撤离点,回去给营地换一整栋新建筑。

## Detailed Design

### Core Rules

**材料**

| 材料 | 档位 | 单位价值 | 主要来源区域 | 主要用途(见 `camp-meta.md`) |
|---|---|---|---|---|
| Iron | Must | 1 | Armory Quarter(军械区) | 铁匠铺、仓库 |
| Herbs | Should | 1 | Herb Gardens(药圃) | 诊所、仓库 |
| Arcane Dust | Should | 2 | Mage Tower(法师塔) | 剪辑室 |

Must 档只有 Iron:所有物资点都产 Iron,所有遗物都分解成 Iron。进入 Should 档后,才按上表区分。

**物资点**

| 类型 | 档位 | 搜索时间 | 产出 | 地图数量 |
|---|---|---|---|---|
| Supply Crate 补给箱 | Must | 2 s | 2–4 个本区域材料 | 12 |
| Cache 贮藏箱 | Should | 4 s | 4–7 个本区域材料;有 20% 概率额外得 1 件遗物 | 5 |
| Vault 宝库 | Must | 8 s | 1 件必得遗物 + 4–6 个本区域材料 | 2 |
| Elite Chest 精英宝箱 | Should | 2 s | 同 Cache,材料随机 | 精英死亡处 |
| Boss Hoard Boss 战利品 | Must | 拾取即得 | 1 件必得遗物 + 8 个材料(随机种类) | Boss 死亡处 |

- 每个物资点每局只能搜一次,搜过后外观变成"已打开"。
- 物资点的位置和类型是固定的,玩家可以记住路线(见 `map-level.md`)。
- **搜索**:站在物资点 250 cm 内按住 E,显示进度条。移出范围或闪避会中断,进度清零。受伤不中断。
- 搜索完成后,产出直接进背包;背包放不下的部分掉在物资点旁的地上。

**遗物**

| 遗物 | 档位 | 价值 | 使用效果 | 分解产出 |
|---|---|---|---|---|
| Phoenix Feather | Must | 25 | 立刻回满生命,并获得 3 s 无敌 | 8 Herbs |
| Warlord's Sigil | Must | 30 | 立刻获得 2 次升级 | 10 Iron |
| Storm Idol | Must | 30 | 屏幕内所有普通怪立即死亡;对精英和 Boss 造成 300 伤害 | 6 Arcane Dust |
| Arcane Core | Should | 35 | 30 s 内伤害 +50%、冷却 −30% | 8 Arcane Dust |
| Blood Chalice | Should | 20 | 20 s 内每击杀 1 只怪回复 1 生命 | 6 Herbs |
| Golden Reel | Should | 40 | 本局所有撤离点立即开放,撤离读条时间减半 | 10 Arcane Dust |

- Must 档的分解产出全部按同等数量换算成 Iron(如 Phoenix Feather → 8 Iron)。
- 遗物出处的抽取:从当前已开放档位的遗物里等概率抽一件。
- 同一种临时效果(如两个 Arcane Core)不叠加,只刷新持续时间。

**背包**

- 基础 8 格;营地仓库的 Bag Capacity 每级 +2 格,最多 16 格。
- 一种材料一格最多堆 20 个,堆满后另占一格。一件遗物占一格。
- **背包面板**:按 Tab 打开,时间放慢到 30%(不暂停),可以在面板里使用或丢弃物品。再按 Tab 或 Esc 关闭。
- **快捷使用**:按 1 / 2 / 3 依次使用背包里的第 1 / 2 / 3 件遗物(按拾取顺序)。
- **丢弃**:丢在原地,变成地上的物品;地上的物品只要靠近按一下 E 就能捡起(背包有空位时)。地上物品存在到本局结束。
- 背包满时,搜出的物品掉在地上,并提示 "Bag Full"。

**热度**

1. 热度 = 背包内所有物品的价值之和,每次背包变化时立即重算。地上的物品不计入。
2. 热度换算成 4 个档位,档位决定刷怪倍率和敌人生命倍率(本文件是这张表的唯一来源;`enemies-spawning.md` 引用它)。
3. 档位上升立即生效;档位下降要在新档位持续 3 s 后才生效,防止来回跳。
4. 档位变化时,HUD 播放提示和音效。

| 档位 | 热度 | HeatSpawnMult | HeatHPMult | 额外效果 |
|---|---|---|---|---|
| Calm | 0–19 | 1.0 | 1.0 | — |
| Noticed | 20–49 | 1.3 | 1.15 | — |
| Hunted | 50–89 | 1.7 | 1.35 | — |
| Infamous | 90+ | 2.2 | 1.6 | 每 45 s 刷一只精英(Should) |

**"留着还是用掉"的平衡原则**

- 当场使用:获得即时力量,而且热度立刻下降,怪变少变弱。
- 带出去:分解产出大约等于一次宝库的材料;再加上局前剪辑的收益倍率,只作用于材料(见 `directors-cut-editing.md`)。
- 目标:一件遗物带出去分解的价值,约等于营地一次 2–3 级升级的成本。这样,在大多数情况下"带出去"都值得认真考虑。

### States and Transitions

| 物资点状态 | 说明 |
|---|---|
| Closed | 可搜索,带高亮和图标 |
| Searching | 玩家按住 E 时,显示进度条 |
| Opened | 已搜过,变暗,不可再搜 |

| 背包面板状态 | 时间 |
|---|---|
| Closed | 100% |
| Open | 30% |

### Interactions with Other Systems

| 系统 | 交互 |
|---|---|
| `enemies-spawning.md` | 提供热度档位倍率;接收精英和 Boss 的掉落请求 |
| `player-combat.md` | 交互键(E)、快捷键(1/2/3);遗物修改生命、无敌、伤害、冷却 |
| `run-progression.md` | Warlord's Sigil 调用升级接口 |
| `extraction-death.md` | 撤离成功时交出背包内容;死亡时清空背包;Golden Reel 修改撤离规则 |
| `directors-cut-editing.md` | 收益倍率在撤离结算时作用于材料 |
| `camp-meta.md` | 背包格数、Greed、Luck 来自营地;遗物分解在营地进行 |
| `map-level.md` | 物资点的位置、类型、所属区域 |

## Formulas

**热度**

```
Heat = Σ(材料数量 × 材料单位价值) + Σ(遗物价值)
```

例:背包里有 18 Iron(18)+ 6 Arcane Dust(12)+ Warlord's Sigil(30)→ Heat = 60 → Hunted 档。此时用掉 Warlord's Sigil,热度降到 30,3 s 后降到 Noticed 档。

**物资点产出**

```
Yield = floor(RandomInt(Min, Max) × (1 + 0.10 × GreedLevel))
```

| 变量 | 含义 | 范围 |
|---|---|---|
| Min, Max | 物资点类型的产出区间 | 见物资点表 |
| GreedLevel | 营地仓库 Greed 等级 | 0–5 |

例:补给箱产出 3,Greed 2 级 → `floor(3 × 1.2) = 3`;贮藏箱产出 6,Greed 5 级 → `floor(6 × 1.5) = 9`。

**贮藏箱的遗物概率**

```
RelicChance = 0.20 + 0.05 × LuckLevel
```

LuckLevel 范围 0–3,概率 20%–35%。

**预期单局收益**(用来校验 `camp-meta.md` 的成本曲线)

一局 5 分钟、只用起始卡,玩家大约能搜 5 个补给箱和 1 个宝库:
`5 × 3 + 5 = 20` 个材料,外加 1 件遗物(分解约 8–10 个)。成功撤离一次,总计约 28 个材料。

## Edge Cases

| 情况 | 处理 |
|---|---|
| 背包满时搜出遗物 | 遗物掉在地上,提示 "Bag Full" |
| 背包面板打开时被打死 | 正常死亡结算;面板自动关闭 |
| 背包面板打开时升级 | 先关闭面板,再进入 LevelUp |
| 死亡时地上有自己丢的东西 | 一起丢失 |
| 撤离读条中丢弃物品 | 允许,读条不中断 |
| 使用 Storm Idol 时屏幕内没有怪 | 照常消耗,播放特效(防止玩家困惑于为什么没反应) |
| Golden Reel 在所有撤离点都已开放时使用 | 只生效"读条时间减半" |
| 两次快捷键按得太快 | 每次使用之间至少间隔 0.3 s |
| 物资点被敌人围住 | 不做特殊处理,这正是热度和取舍的一部分 |

## Dependencies

- 上游:`map-level`(物资点)、`camp-meta`(背包格数、Greed、Luck)、`player-combat`(输入)。
- 下游:`enemies-spawning`(热度倍率)、`extraction-death`(背包内容)、`run-progression`(Warlord's Sigil)、`ui-hud`(背包、热度表、搜索进度)。

## Tuning Knobs

| 参数 | 默认值 | 安全范围 | 影响 |
|---|---|---|---|
| 热度档位阈值 | 20 / 50 / 90 | ±50% | 多快进入高压 |
| 档位倍率 | 见热度表 | 刷怪 1.0–3.0,生命 1.0–2.0 | 热度的惩罚力度 |
| 档位下降延迟 | 3 s | 1–5 s | 档位是否来回跳 |
| 背包基础格数 | 8 | 6–12 | 取舍的紧张程度 |
| 每格堆叠上限 | 20 | 10–50 | 背包是否被材料塞满 |
| 搜索时间 | 2 / 4 / 8 s | 1–12 s | 搜刮的风险 |
| 搜索半径 | 250 cm | 150–400 | 操作宽容度 |
| 背包面板时间缩放 | 30% | 0–50% | 管理背包的压力 |
| 遗物价值 | 20–40 | 10–60 | 带着遗物的热度代价 |
| 遗物分解产出 | 6–10 | 4–20 | "带出去"的吸引力 |

## Visual/Audio Requirements

- 物资点按类型区分外观:补给箱是木箱,贮藏箱带锁,宝库有发光的门;材料种类用颜色区分(Iron 灰蓝、Herbs 绿、Arcane Dust 紫)。
- 遗物有独特图标和光柱,掉在地上时远处也能看到。
- 热度档位变化:HUD 热度表变色 + 一声"片场警报"式的提示音;Infamous 时画面边缘出现淡淡的红色胶片颗粒。
- 使用遗物要有醒目的全屏或角色特效(Storm Idol 全屏闪电)。

## Game Feel

捡到遗物的那一刻要有分量:短暂慢动作(0.2 s)+ 独特音效。热度上升要让玩家"听到"压力,而不只是看到数字。

## UI Requirements

热度表、背包面板、背包快捷栏(显示前 3 件遗物和对应按键)、搜索进度、"Bag Full" 提示、物资点在 HUD 边缘的方向标记。见 `ui-hud.md`。

## Cross-References

- `design/gdd/enemies-spawning.md`(引用本文件的热度倍率表)
- `design/gdd/camp-meta.md`(分解、背包格数、Greed、Luck)
- `design/gdd/directors-cut-editing.md`(收益倍率)
- `design/gdd/extraction-death.md`(带出与丢失)

## Acceptance Criteria

- **AC1(Must)**:按住 E 2 s 搜索补给箱,背包里增加 2–4 个 Iron,箱子变为已打开,不能再搜。
- **AC2(Must)**:搜索中移出 250 cm 范围,进度清零。
- **AC3(Must)**:背包热度 ≥ 20 时,HUD 显示 Noticed,且刷怪速率按 1.3 倍计算(日志可查)。
- **AC4(Must)**:使用 Warlord's Sigil 后,连续弹出 2 次升级选择,热度减少 30。
- **AC5(Must)**:背包 8 格占满后,再搜出的物品掉在地上并提示 "Bag Full"。
- **AC6(Must)**:按 Tab 打开背包,游戏速度变为 30%,可以使用和丢弃物品。
- **AC7(Should)**:Armory Quarter 的补给箱只产 Iron,Herb Gardens 只产 Herbs,Mage Tower 只产 Arcane Dust。
- **AC8(Must)**:热度从 Hunted 降到 Noticed 的阈值以下后,3 s 后档位才变化。

## Open Questions

- 丢在地上的物品不计热度,玩家可能先把宝物丢在撤离点附近,打完再回来捡。默认允许,视为一种策略;如果试玩发现它破坏了紧张感,改成地上物品 30 s 后消失。
