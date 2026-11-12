# Director's Cut Editing (Pre-Run)

> **Status**: Designed
> **Author**: 开发者本人 + Claude
> **Last Updated**: 2026-11-12
> **Last Verified**: 2026-11-12
> **Implements Pillar**: P2 You direct the cut(核心系统),主题 Director's Cut

## Summary

每局出发前,玩家在营地的剪辑室把**片段卡**插进**胶片槽**,决定这一局的精英、尸潮、Boss、撤离点和时长。每张卡有风险值,风险总和决定本局材料的**收益倍率**(Box Office)。新卡在营地剪辑室里"恢复"出来。这是"导演剪辑版"主题在机制上的落点。

> **Quick reference** — Layer: `Feature` · Priority: `MVP` · Key deps: `camp-meta`, `enemies-spawning`, `extraction-death`

## Overview

剪辑室是一个面板:左边是卡库,中间是胶片槽,右边是本局预览(风险、倍率、时间表)。玩家可以全部留空(普通难度、倍率 ×1),也可以塞满高风险卡去博高倍率。卡组合在出发时锁定。

## Player Fantasy

"这一条我要拍得更狠。"我不是在被动接受随机难度,而是像导演一样剪辑这一场戏:加一段尸潮、请出 Boss、删掉一个出口,然后亲自上场演完它。

## Detailed Design

### Core Rules

**胶片槽**

1. 初始 2 个槽(Must)。营地剪辑室的 Film Slots 每级 +1,最多 5 个(Should,见 `camp-meta.md`)。
2. 槽可以留空。同一张卡不能插两次。
3. 时长类卡互斥:同时只能插一张。
4. 撤离类卡 Locked Exits 只有在本局撤离点 ≥ 2 个时才能插入(保证至少剩 1 个撤离点)。
5. 剪辑室记住上一次的卡组合,下次打开时自动填好,减少重复操作。
6. 出发("Roll Camera!")时锁定卡组合和收益倍率,局内无法修改。

**片段卡**

| 卡 | 类别 | 档位 | 风险 | 效果 | 解锁方式 |
|---|---|---|---|---|---|
| Horde Night | 怪物 | Must | 2 | 开启尸潮(规则见 `enemies-spawning.md`) | 初始拥有 |
| The Cut Knight | 高潮戏 | Must | 3 | 4:00 Boss 登场(Short Cut 时 3:00) | 初始拥有 |
| Elite Patrol | 怪物 | Should | 2 | 每 60 s 出现一只精英 | 剪辑室恢复 |
| Locked Exits | 撤离 | Should | 2 | 移除最早开放的撤离点(North Gate) | 剪辑室恢复 |
| Late Exits | 撤离 | Should | 1 | 所有撤离点推迟 2:00 开放 | 剪辑室恢复 |
| Short Cut | 时长 | Should | −1 | 本局时长 6:00,撤离点开放时间按比例提前 | 剪辑室恢复 |
| Extended Cut | 时长 | Should | 2 | 本局时长 14:00,撤离点开放时间按比例推后 | 剪辑室恢复 |

- 所有卡的效果都是**确定的**:没有随机成分(支柱 P2)。
- "移除最早开放的撤离点"按开放时间排序,固定移除 North Gate。
- 卡的解锁成本由 `camp-meta.md` 定义。

**本局预览**

面板右侧根据当前卡组合实时显示:
- 风险总和和收益倍率,例:"Risk 5 · Box Office ×2.25"
- 时间表(按时间排序的文字列表),例:
  - `1:30  Horde incoming`
  - `3:00  North Gate opens`
  - `4:00  The Cut Knight enters`
  - `5:00  East Docks opens`
- 本局时长(Must 档没有时长卡,显示 "No time limit")。

### States and Transitions

| 面板状态 | 说明 |
|---|---|
| Browsing | 浏览卡库、拖放或点击插卡 |
| Locked | 点击 Roll Camera! 之后,进入 Loading,卡组合不可再改 |

### Interactions with Other Systems

| 系统 | 交互 |
|---|---|
| `camp-meta.md` | 槽位数量和卡的解锁状态来自营地剪辑室 |
| `enemies-spawning.md` | 读取是否开启尸潮、精英巡逻、Boss,以及 Boss 登场时间 |
| `extraction-death.md` | 读取时长、撤离点开放时间和移除、收益倍率 M |
| `loot-inventory-heat.md` | 收益倍率只作用于材料,不作用于遗物 |
| `game-flow.md` | Roll Camera! 触发出发;加载卡显示卡组合名 |
| `ui-hud.md` | 剪辑室面板 |

## Formulas

**收益倍率(Box Office)**

```
M = clamp(1 + 0.25 × ΣRisk, 0.75, 3.0)
```

| 变量 | 含义 | 范围 |
|---|---|---|
| ΣRisk | 已插卡的风险值之和 | −1 到 11 |
| M | 本局材料收益倍率 | 0.75–3.0 |

例:
- 不插卡:ΣRisk = 0 → M = 1.0
- Horde Night + The Cut Knight(Must 档的满配):ΣRisk = 5 → M = 2.25
- 只插 Short Cut:ΣRisk = −1 → M = 0.75
- 五槽满配高风险:ΣRisk = 3 + 2 + 2 + 2 + 2 = 11 → `1 + 2.75 = 3.75`,被上限压到 3.0

**倍率设计依据**:Must 档满配(×2.25)意味着带出的材料超过两倍,足以让"打 Boss + 扛尸潮"值得冒险;同时 Boss 本身还掉一件遗物。不插卡(×1.0)是新手的安全选择。

**卡组合名(加载卡显示用)**

```
无卡 → "Theatrical Cut";有卡 → 按插槽顺序把卡名用 " + " 连接
```

## Edge Cases

| 情况 | 处理 |
|---|---|
| 想插第二张时长卡 | 不允许,提示 "Only one runtime card per take" |
| 插 Locked Exits 时只剩 1 个撤离点 | 不允许,提示 "At least one exit must remain" |
| 上次的卡组合里有卡现在不可用(例如重置了存档) | 自动移除那张卡 |
| 槽位升级后 | 新槽为空,不自动填卡 |
| 全部留空出发 | 允许,M = 1.0 |
| Must 档只有 2 个槽和 2 张卡 | 正常;评委第一局可以不插卡,第二局插上卡体验差别 |

## Dependencies

- 上游:`camp-meta`(槽位、解锁)。
- 下游:`enemies-spawning`、`extraction-death`、`loot-inventory-heat`(倍率)、`game-flow`(出发)、`ui-hud`(面板)。

## Tuning Knobs

| 参数 | 默认值 | 安全范围 | 影响 |
|---|---|---|---|
| 每点风险的倍率 | 0.25 | 0.1–0.5 | 冒险的回报 |
| 倍率下限 / 上限 | 0.75 / 3.0 | 0.5–1.0 / 2.0–5.0 | 收益波动 |
| 初始槽位 | 2 | 1–3 | 第一局的选择丰富度 |
| 各卡风险值 | 见卡表 | −2 到 5 | 每张卡的分量 |
| Late Exits 推迟时间 | 120 s | 60–180 s | 卡的难度 |
| Short Cut / Extended Cut 时长 | 360 / 840 s | 240–480 / 720–1080 | 局时 |

## Visual/Audio Requirements

- 胶片槽画成一段横向的胶片,卡片画成带缩略图的"片段"。插入时有胶片卡入的"咔哒"声。
- 风险高时倍率数字变成醒目的金色。
- Roll Camera! 按钮按下时:场记板打板音效 + 胶片放映机启动声。

## Game Feel

插卡要快:点击卡片自动插入第一个空槽,再点一次槽里的卡把它拿出来。不需要拖放(Should 档才做拖放)。

## UI Requirements

剪辑室面板:卡库(含已锁定的卡和解锁提示)、胶片槽、预览区、Roll Camera! 按钮。见 `ui-hud.md`。

## Cross-References

- `design/gdd/camp-meta.md`(剪辑室的升级和解锁成本)
- `design/gdd/enemies-spawning.md`(尸潮、精英、Boss 的具体规则)
- `design/gdd/extraction-death.md`(开放时间公式、入库公式)

## Acceptance Criteria

- **AC1(Must)**:新存档打开剪辑室,能看到 2 个空槽和 2 张可用卡(Horde Night、The Cut Knight)。
- **AC2(Must)**:插入两张卡后,预览显示 "Risk 5 · Box Office ×2.25",时间表包含 Horde 和 Boss 的时间。
- **AC3(Must)**:插了 The Cut Knight 出发,4:00 Boss 登场;没插,整局没有 Boss。
- **AC4(Must)**:倍率在撤离结算时作用于材料(见 `extraction-death.md` AC4)。
- **AC5(Must)**:关闭面板、重新打开,上次的卡组合仍在。
- **AC6(Should)**:插 Short Cut 后,时长显示 6:00,North Gate 开放时间显示 1:48。
- **AC7(Should)**:已插一张时长卡时,无法再插另一张,并显示提示。

## Open Questions

- 是否在第一局给评委一个"推荐剪辑"按钮(一键插入推荐卡)?默认不做;第一局的教程提示会建议 "Try adding a scene card for more Box Office"。
