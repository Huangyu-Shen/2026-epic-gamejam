# Camp & Meta Progression

> **Status**: Designed
> **Author**: 开发者本人 + Claude
> **Last Updated**: 2026-11-12
> **Last Verified**: 2026-11-12
> **Implements Pillar**: P3 Short takes, visible growth;主题 Director's Cut(剪辑室)

## Summary

局外成长做"薄":一个固定镜头的 3D 营地,四栋建筑各管几条吸血鬼幸存者式的属性升级。升级只花通用材料,成本按指数增长。带出来的遗物在这里分解成材料,或者留在仓库里囤着。剪辑室负责解锁新片段卡、新武器和更多胶片槽,即"恢复被剪掉的片段"。

> **Quick reference** — Layer: `Feature` · Priority: `MVP` · Key deps: `extraction-death`, `loot-inventory-heat`, `game-flow`

## Overview

营地是"片场后台"。玩家回来后看到材料入库,点击建筑打开升级面板,花材料把属性升一级,然后进剪辑室准备下一条。局外成长不追求丰富,只负责一件事:让"带出去"值得。

## Player Fantasy

"片场一点点像样起来了。"每次成功撤离回来,都能马上花掉材料,看到数字变大;剪辑室里又亮起一张新卡,下一局有了新的玩法。

## Detailed Design

### Core Rules

**营地场景**

1. 固定镜头俯视营地,不能移动和旋转。场景里有 4 栋建筑和一个出口("Set Gate",装饰用)。
2. 鼠标悬停建筑:描边高亮 + 建筑名;点击:打开该建筑的面板。
3. 右上角常驻显示仓库材料数量。
4. 出发只能通过剪辑室面板(见 `directors-cut-editing.md`)。

**四栋建筑与属性**

每条属性有独立的等级,按 `camp-meta` 的成本公式逐级购买。属性加成在每局开局时一次性生效。

| 建筑 | 属性 | 档位 | 每级效果 | 满级 | 基础成本 |
|---|---|---|---|---|---|
| Blacksmith 铁匠铺 | Might | Must | 伤害 +5% | 5 | 10 |
| | Area | Must | 范围 +10% | 3 | 15 |
| | Amount | Should | 投射物类武器数量 +1 | 1 | 60 |
| Clinic 诊所 | Max HP | Must | 生命上限 +10 | 5 | 10 |
| | Recovery | Must | 每秒回复 +0.2 生命 | 3 | 15 |
| | Revival | Should | 复活次数 +1 | 1 | 60 |
| Storehouse 仓库 | Bag Capacity | Should | 背包 +2 格 | 4 | 12 |
| | Greed | Should | 物资点产出 +10% | 5 | 12 |
| Editing Room 剪辑室 | Film Slots | Should | 胶片槽 +1 | 3 | 15 |
| | Luck | Should | 贮藏箱遗物概率 +5% | 3 | 15 |

**剪辑室的一次性解锁(Should)**

| 解锁项 | 类型 | 成本 |
|---|---|---|
| Short Cut | 片段卡 | 8 |
| Elite Patrol | 片段卡 | 10 |
| Late Exits | 片段卡 | 10 |
| Locked Exits | 片段卡 | 15 |
| Extended Cut | 片段卡 | 15 |
| Throwing Axes | 武器 | 20 |
| Lightning Rune | 武器 | 30 |

解锁后立即生效:卡出现在剪辑室卡库,武器进入局内升级池(见 `run-progression.md`)。

**材料配方**

- Must 档:所有成本都用 Iron 支付。
- Should 档:
  - 铁匠铺:Iron
  - 诊所:Herbs
  - 仓库:Iron 和 Herbs 各付一半(各自向上取整)
  - 剪辑室(属性和解锁):Arcane Dust

**仓库与分解(Must)**

1. 仓库面板列出所有材料数量和囤着的遗物。
2. 遗物可以"分解"(Salvage),按 `loot-inventory-heat.md` 的分解产出表换成材料。分解前确认一次,不可撤销。
3. 遗物也可以一直留着,不计上限。仓库里的遗物**不能**带进局内,只能囤着或分解(带入局内属于"以后再做")。
4. Must 档的仓库面板挂在营地的一个箱子上;Should 档改为仓库建筑的面板。

**建筑外观(Could)**

每栋建筑的外观按它所有属性等级之和分 3 个阶段:0–2 级、3–6 级、7 级以上。阶段变化时播放一次短暂的施工特效。

### States and Transitions

| 营地状态 | 说明 |
|---|---|
| Idle | 浏览营地,悬停高亮 |
| PanelOpen | 某栋建筑的面板打开,背景变暗,不能点其他建筑 |
| EditingRoom | 剪辑室面板(见 `directors-cut-editing.md`) |

升级流程:点击"Upgrade" → 检查材料 → 扣材料 → 等级 +1 → 刷新面板 → 自动保存。材料不足时按钮置灰,并显示还差多少。

### Interactions with Other Systems

| 系统 | 交互 |
|---|---|
| `extraction-death.md` | 撤离成功和 NG 补偿写入仓库 |
| `loot-inventory-heat.md` | 分解产出表;背包格数、Greed、Luck 由本系统提供 |
| `player-combat.md` | 开局时叠加 Might、Area、Amount、Max HP、Recovery、Revival |
| `run-progression.md` | 武器解锁 |
| `directors-cut-editing.md` | 胶片槽数量和卡的解锁状态 |
| `game-flow.md` | 自动保存的时机;存档内容 |
| `ui-hud.md` | 营地 HUD、建筑面板、仓库面板 |

## Formulas

**升级成本**

```
Cost(L) = round(BaseCost × g^(L − 1))
```

| 变量 | 含义 | 默认 | 范围 |
|---|---|---|---|
| L | 要购买的等级 | — | 1–5 |
| BaseCost | 该属性的基础成本(见表) | 10–60 | — |
| g | 增长底数 | 1.8 | 1.5–2.2 |

| 基础成本 | L1 | L2 | L3 | L4 | L5 | 满级合计 |
|---|---|---|---|---|---|---|
| 10 | 10 | 18 | 32 | 58 | 105 | 223 |
| 12 | 12 | 22 | 39 | 70 | 126 | 269 |
| 15 | 15 | 27 | 49 | — | — | 91(3 级封顶) |

**前期节奏校验(对应需求 R19:前 2–3 局完成 2–3 次升级)**

- 第 1 局:不插卡(M = 1.0),5 分钟撤离,约 20 Iron + 1 件遗物(分解约 8–10)≈ 28 Iron(见 `loot-inventory-heat.md` 的预期收益)。
- 可以买:Might L1(10)+ Max HP L1(10)= 20,剩 8。→ **第 1 局后 2 次升级**。
- 第 2 局:插满 Must 卡(M = 2.25),约 20 × 2.25 = 45 + Boss 遗物 ≈ 55 Iron。
- 可以再买:Might L2(18)+ Max HP L2(18)+ Area L1(15)。→ **第 2 局后共 5 次升级**,满足 R19。
- 满级全部 Must 属性约需 `223 + 91 + 223 + 91 = 628` Iron,约 12–20 局。jam 评委不会买满,这是给愿意多玩的人留的长线。

**开局属性汇总**

```
MightBonus(营地) = 0.05 × MightLevel
AreaBonus(营地) = 0.10 × AreaLevel
MaxHP 增量 = 10 × MaxHPLevel
Recovery = 0.2 × RecoveryLevel(每秒)
```

例:Might 2 级 → +10% 伤害,与 `player-combat.md` 的例子一致。

## Edge Cases

| 情况 | 处理 |
|---|---|
| 材料刚好够 | 允许购买,剩 0 |
| 已满级 | 按钮显示 "MAX",不再显示成本 |
| 仓库为空时打开分解 | 显示 "No relics in storage" |
| 分解中途退出游戏 | 分解是瞬间完成并立即保存的,不存在中间状态 |
| 解锁武器时正在局内 | 不可能:只能在营地解锁 |
| 剪辑室槽位升级后 | 新槽为空 |
| 存档里有 Must 档之外的数据,但游戏只做完了 Must | 不会发生:数据结构一开始就按全档位设计,未实现的属性显示为锁定 |

## Dependencies

- 上游:`extraction-death`(入库)、`loot-inventory-heat`(分解产出)、`game-flow`(保存)。
- 下游:`player-combat`(开局属性)、`run-progression`(武器解锁)、`directors-cut-editing`(槽位和卡)、`loot-inventory-heat`(背包、Greed、Luck)、`ui-hud`(面板)。

## Tuning Knobs

| 参数 | 默认值 | 安全范围 | 影响 |
|---|---|---|---|
| 增长底数 g | 1.8 | 1.5–2.2 | 后期成长有多慢 |
| 各属性基础成本 | 见表 | 5–80 | 前期能多快升级 |
| 各属性每级效果 | 见表 | ±50% | 局外成长对局内的影响力 |
| 解锁成本 | 8–30 | 5–60 | 新内容出现的速度 |

## Visual/Audio Requirements

- 营地:夜晚的片场后台,暖色的片场灯照着几栋帐篷和木屋;剪辑室是一辆亮着放映灯的马车或小屋。
- 升级:按钮按下后建筑上方飘出属性图标 + "叮"的确认音。
- 分解:遗物碎成光点、汇入材料图标。

## Game Feel

营地操作一步到位:点建筑 → 点升级,两次点击完成一次升级。不要确认弹窗(分解除外)。

## UI Requirements

营地 HUD(材料数量)、建筑面板(属性列表、等级点、当前值 → 下一级值、成本、升级按钮)、仓库面板(材料、遗物、分解按钮)。见 `ui-hud.md`。

## Cross-References

- `design/gdd/loot-inventory-heat.md`(预期单局收益、分解产出)
- `design/gdd/directors-cut-editing.md`(槽位和卡)
- `design/gdd/run-progression.md`(武器解锁)
- `design/gdd/player-combat.md`(属性生效公式)

## Acceptance Criteria

- **AC1(Must)**:悬停建筑出现描边和名字,点击打开对应面板。
- **AC2(Must)**:持有 28 Iron 时,能购买 Might L1(10)和 Max HP L1(10),剩余 8 Iron;Might L2 显示成本 18 且按钮置灰。
- **AC3(Must)**:购买 Max HP L1 后,下一局开局生命上限为 110。
- **AC4(Must)**:分解一件 Warlord's Sigil,仓库增加 10 Iron,遗物消失。
- **AC5(Must)**:升级后退出重进,等级和材料保持不变。
- **AC6(Should)**:在剪辑室解锁 Elite Patrol 后,它出现在剪辑室卡库里且可插入。
- **AC7(Should)**:购买 Film Slots L1 后,剪辑室显示 3 个槽。
- **AC8(Must)**:新存档前 2 局(第 1 局不插卡、第 2 局插满 Must 卡,都成功撤离)结束后,至少完成 2 次升级。

## Open Questions

- 仓库里的遗物能否带进局内(装备栏)?已定为"以后再做"。
