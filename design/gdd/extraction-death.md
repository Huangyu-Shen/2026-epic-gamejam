# Extraction & Death

> **Status**: Designed
> **Author**: 开发者本人 + Claude
> **Last Updated**: 2026-11-12
> **Last Verified**: 2026-11-12
> **Implements Pillar**: P1 Every pickup is a choice(带出去才算数),P3 Short takes

## Summary

定义一局怎么结束:在已开放的撤离点站满时间 → 成功,背包材料乘以收益倍率后存入营地;生命归零或主动放弃 → 死亡,背包全部丢失,只拿到一点 NG 胶片补偿;时间耗尽 → 视为死亡。

> **Quick reference** — Layer: `Core` · Priority: `MVP` · Key deps: `loot-inventory-heat`, `directors-cut-editing`, `map-level`, `game-flow`

## Overview

地图上有固定的撤离点,它们按时间表陆续开放。玩家自己决定什么时候撤:早撤稳妥但拿得少,晚撤拿得多但怪越来越强。撤离需要在点内坚持一段时间,这段读条就是一局最紧张的时刻。

## Player Fantasy

"倒计时还剩 5 秒,怪已经围上来了。"带着一包宝贝冲向撤离点、硬撑到读条走完,听到 "Cut! That's a Wrap!",这一刻就是这款游戏的高潮。

## Detailed Design

### Core Rules

**撤离点**

| 撤离点 | 档位 | 默认开放时间 | 位置(见 `map-level.md`) |
|---|---|---|---|
| North Gate | Must | 3:00 | 北侧 |
| East Docks | Must | 5:00 | 东侧 |
| West Bridge | Should | 7:00 | 西侧 |

1. 撤离点开放前显示为关闭状态,并显示倒计时;开放后一直开放到本局结束。
2. **撤离**:玩家站在已开放撤离点的 400 cm 半径内,自动开始读条。读条 30 s(Golden Reel 生效时 15 s)。
3. 读条期间离开半径,进度清零。受伤和闪避(只要还在半径内)都不中断。
4. 读条完成 → 撤离成功,本局立即结束。
5. 撤离点的开放时间受片段卡影响(时长类和撤离类卡,定义在 `directors-cut-editing.md`;换算公式见下)。

**本局时长与超时(Should)**

- 默认时长(Theatrical Cut)10:00;时长类卡可以改为 6:00 或 14:00。
- 剩余 60 s 时,全屏提示 "Final Reel — 60 seconds!",背景音乐加速。
- 时间耗尽仍未撤离,视为死亡(MIA)。
- Must 档没有时长上限:撤离点开放后一直可用,直到玩家撤离或死亡。

**撤离成功结算**

1. 背包里每种材料的数量 × 本局收益倍率 M,四舍五入。
2. 遗物数量不变,不乘倍率。
3. 结算结果写入营地仓库,自动保存(见 `game-flow.md`)。
4. 结算画面 "That's a Wrap!" 显示:Take 编号、存活时间、击杀数、局内等级、每种物品的数量、收益倍率、最终入库数量。

**死亡结算**

1. 触发:生命归零且没有复活次数;在暂停菜单选 Abandon Take;超时。
2. 背包和地上的物品全部丢失。
3. **NG 胶片补偿(Should)**:直接给营地仓库一些 Arcane Dust,数量见公式。代表"这条虽然 NG 了,素材还能剪进别处"。
4. 结算画面 "NG — Take N":黑白画面,显示存活时间、击杀数、丢失的物品、获得的 NG 补偿。

### States and Transitions

| 撤离点状态 | 说明 |
|---|---|
| Locked | 未到开放时间,显示倒计时 |
| Open | 可撤离,有光柱 |
| Extracting | 玩家在点内,显示读条 |
| Removed | 被 Locked Exits 卡移除,本局不出现 |

局内结束的三种结果:`Extracted`、`Died`、`MIA`(超时)。后两种结算规则相同,只是画面文案不同。

### Interactions with Other Systems

| 系统 | 交互 |
|---|---|
| `loot-inventory-heat.md` | 读取背包内容;Golden Reel 修改开放时间和读条时间 |
| `directors-cut-editing.md` | 读取收益倍率 M、时长、撤离类卡 |
| `player-combat.md` | 接收"生命归零且没有复活"的死亡事件 |
| `camp-meta.md` | 写入仓库 |
| `game-flow.md` | 本局结束后进入 RunResult 状态 |
| `map-level.md` | 撤离点位置 |
| `ui-hud.md` | 撤离点方向标记、开放倒计时、读条、两种结算画面 |

## Formulas

**撤离点开放时间**

```
OpenTime_i = BaseOpenTime_i × (Duration / 600) + LateExitsDelay
```

| 变量 | 含义 | 默认 | 范围 |
|---|---|---|---|
| BaseOpenTime_i | 撤离点 i 的默认开放时间(秒) | 180 / 300 / 420 | — |
| Duration | 本局时长(秒) | 600 | 360 / 600 / 840 |
| LateExitsDelay | 插了 Late Exits 卡时为 120,否则为 0 | 0 | 0 或 120 |

例:插了 Short Cut(360 s)→ North Gate `180 × 0.6 = 108 s`(1:48);再加 Late Exits → 3:48。

**撤离成功的入库数量**

```
Stored_m = round(BagCount_m × M)
```

M 是本局收益倍率(定义在 `directors-cut-editing.md`,范围 0.75–3.0)。例:背包 20 Iron,M = 1.5 → 入库 30 Iron。

**NG 胶片补偿(Should)**

```
NGDust = min(1 + floor(SurvivedSeconds / 60), 5)
```

例:存活 3:40 → `min(1 + 3, 5) = 4` Arcane Dust。存活 10 s 也至少给 1。上限 5,不到一次补给箱的价值,所以死亡永远不划算,但不会一无所获。

## Edge Cases

| 情况 | 处理 |
|---|---|
| 读条完成的同一帧死亡 | 死亡优先(见 `game-flow.md`) |
| 读条中升级 | LevelUp 暂停读条,选完继续 |
| 背包为空时撤离 | 允许,结算显示 "Nothing in the can",仍算成功的一局 |
| 所有撤离点都被移除 | 不可能:Locked Exits 最多移除 1 个,且至少保留 1 个(见 `directors-cut-editing.md`) |
| Boss 站在撤离点上 | 不做特殊处理;玩家可以去别的撤离点 |
| 超时那一刻正在读条 | 视为 MIA,读条作废 |
| 有复活次数但选 Abandon Take | 直接死亡结算,复活不生效 |

## Dependencies

- 上游:`loot-inventory-heat`(背包)、`directors-cut-editing`(倍率和时间表)、`player-combat`(死亡事件)、`map-level`(位置)。
- 下游:`camp-meta`(入库)、`game-flow`(进入结算)、`ui-hud`(画面)。

## Tuning Knobs

| 参数 | 默认值 | 安全范围 | 影响 |
|---|---|---|---|
| 撤离读条时间 | 30 s | 10–45 s | 撤离的紧张程度 |
| 撤离半径 | 400 cm | 250–600 | 被逼出圈的难度 |
| 默认开放时间 | 3:00 / 5:00 / 7:00 | 1:30–9:00 | 最短局时 |
| 默认时长 | 10:00 | 6:00–15:00 | 最长局时 |
| Final Reel 提示 | 剩余 60 s | 30–120 s | 超时前的预警 |
| NG 补偿上限 | 5 | 0–10 | 死亡的安慰程度 |

## Visual/Audio Requirements

- 撤离点:关闭时是暗下来的片场灯,开放时亮起一束光柱,远处可见。
- 读条:撤离点周围一圈地面光环逐渐填满;最后 5 s 有场记员倒数音效("Five, four...")。
- 成功:"Cut! That's a Wrap!" 大字 + 掌声或号角。
- 死亡:画面转黑白、胶片划痕、"NG" 红色印章。

## Game Feel

读条最后几秒要"刺激":音乐加强,镜头微微推近。成功的一刻要有释放感。死亡画面要短,让玩家立刻想再来一局。

## UI Requirements

HUD 上撤离点的方向标记和开放倒计时、读条环、Final Reel 提示、两种结算画面。见 `ui-hud.md`。

## Cross-References

- `design/gdd/directors-cut-editing.md`(收益倍率 M、时长、撤离类卡)
- `design/gdd/loot-inventory-heat.md`(背包、Golden Reel)
- `design/gdd/camp-meta.md`(仓库)

## Acceptance Criteria

- **AC1(Must)**:North Gate 在 3:00 前显示倒计时且不可撤离,3:00 后亮起。
- **AC2(Must)**:站进开放的撤离点 30 s,撤离成功,进入 "That's a Wrap!" 结算。
- **AC3(Must)**:读条中离开半径再回来,读条从 0 开始。
- **AC4(Must)**:背包 20 Iron、M = 1.5 时撤离,营地仓库增加 30 Iron。
- **AC5(Must)**:死亡后背包物品全部消失,营地仓库没有增加任何材料(Should 档的 NG 补偿除外)。
- **AC6(Should)**:存活 3:40 后死亡,营地获得 4 Arcane Dust。
- **AC7(Should)**:10:00 时仍未撤离,按 MIA 结算,规则与死亡相同。

## Open Questions

- 读条中离开半径是"清零"还是"暂停"?默认清零(更紧张);如果试玩反馈太挫败,改为暂停。
