# Player Control & Combat

> **Status**: Designed
> **Author**: 开发者本人 + Claude
> **Last Updated**: 2026-11-12
> **Last Verified**: 2026-11-12
> **Implements Pillar**: P1 Every pickup is a choice(提供压力),P3 Short takes

## Summary

玩家在斜俯视镜头下用 WASD 移动,武器自动攻击,另有闪避和一个交互键(搜索、撤离提示)。本文件定义角色基础属性、移动、闪避、武器开火规则、伤害计算、受击与死亡判定。武器的具体清单和升级在 `run-progression.md`。

> **Quick reference** — Layer: `Core` · Priority: `MVP` · Key deps: `game-flow`, `run-progression`, `camp-meta`

## Overview

操作只有走位和时机:玩家决定站在哪里、什么时候闪避、什么时候停下来搜刮。所有攻击都自动进行,所以玩家的注意力可以放在路线、热度和背包取舍上。

## Player Fantasy

"一个人站在尸潮中间,刀光一圈圈扫开。"前期偶尔会被逼到墙角,后期一路平推。闪避是唯一的主动救命手段,用对时机会很爽。

## Detailed Design

### Core Rules

**输入(键鼠,Must)**

| 键 | 动作 |
|---|---|
| W A S D | 八方向移动(相对屏幕) |
| Space | 闪避 |
| E(按住) | 交互:搜索物资点 |
| Tab | 打开/关闭背包(时间放慢,见 `loot-inventory-heat.md`) |
| 1 / 2 / 3 | 快捷使用背包里的第 1/2/3 件遗物 |
| Esc | 暂停 |
| 鼠标 | 只用于 UI,局内不瞄准 |

手柄为 Could 档:左摇杆移动,A 闪避,X 按住交互,Y 背包,方向键快捷使用遗物。

**镜头**

- 斜俯视,俯角 55°,镜头距离 1800 cm,跟随玩家,带轻微平滑(0.1 s)。
- 不能旋转,不能缩放(Must)。不允许墙体遮挡角色:遮挡物淡化为半透明(Should)。

**移动**

- 基础移速 600 cm/s,即时加速,无惯性(割草类要求操作跟手)。
- 角色朝向移动方向;停下后保持最后朝向。
- 与敌人有碰撞,但玩家可以把小怪推开一点(敌人质量更小),防止被一只小怪卡死。

**闪避**

- 向当前移动方向冲刺 400 cm,用时 0.2 s;静止时向朝向方向冲刺。
- 冲刺期间无敌(0.2 s),可以穿过敌人,不能穿墙。
- 冷却 2.0 s,HUD 显示冷却。
- 闪避不打断武器开火。

**武器开火(通用规则)**

- 最多同时持有 4 把武器,所有武器各自独立冷却、自动开火。
- 每把武器有一种**索敌方式**:
  - `Facing`:朝角色朝向
  - `Nearest`:最近的敌人
  - `Random`:范围内随机敌人
  - `Around`:以角色为中心
- 射程内没有敌人时,`Nearest` 和 `Random` 类武器不开火(不浪费冷却);`Facing` 和 `Around` 照常开火。
- 武器数值:伤害、冷却、范围、发射数量、持续时间、穿透。具体值见 `run-progression.md`。

**伤害与受击**

- 玩家基础生命 100,无护甲(Must)。
- 敌人对玩家造成接触伤害或投射物伤害;受击后玩家有 0.5 s 无敌帧,期间不再受伤。
- 生命回复:基础 0;来源是营地诊所的 Recovery 属性和遗物。
- 生命降到 0:如果有复活次数(营地诊所的 Revival),立刻复活;否则本局死亡,交给 `extraction-death.md`。
- 复活效果:恢复 50% 生命,2 s 无敌,同时以自身为中心发出一圈击退冲击波(半径 600 cm,不造成伤害)。

### States and Transitions

| 角色状态 | 可移动 | 可闪避 | 武器开火 | 可受伤 |
|---|---|---|---|---|
| Normal | 是 | 是 | 是 | 是 |
| Dodging(0.2 s) | 自动冲刺 | 否 | 是 | 否 |
| HitInvuln(0.5 s) | 是 | 是 | 是 | 否 |
| Searching(按住 E) | 是(移出范围则中断) | 是(中断搜索) | 是 | 是 |
| Reviving(2 s) | 是 | 是 | 是 | 否 |
| Dead | 否 | 否 | 否 | 否 |

### Interactions with Other Systems

| 系统 | 交互 |
|---|---|
| `run-progression.md` | 提供武器和被动属性;局内升级修改本系统的属性 |
| `camp-meta.md` | 局外属性在开局时一次性叠加:Might、Area、Amount、Max HP、Recovery、Revival |
| `enemies-spawning.md` | 敌人造成伤害,接收武器伤害 |
| `loot-inventory-heat.md` | 交互键搜索物资点;快捷键使用遗物 |
| `extraction-death.md` | 生命归零且没有复活次数时,通知它本局死亡 |

## Formulas

**最终伤害**

```
Damage = WeaponBase × (1 + MightBonus) × (1 + ItemDamageBonus)
```

| 变量 | 含义 | 范围 |
|---|---|---|
| WeaponBase | 武器当前等级的基础伤害 | 5–60 |
| MightBonus | 营地 Might(每级 +0.05)+ 局内被动 Might(每级 +0.10)之和 | 0–0.75 |
| ItemDamageBonus | 遗物的临时加成(如 Arcane Core +0.5) | 0–0.5 |

例:Sword Arc 3 级,基础 16;营地 Might 2 级 = +0.10,局内 Might 1 级 = +0.10 → `16 × 1.2 × 1.0 = 19.2`,向下取整为 19。

**最终冷却**

```
Cooldown = WeaponBaseCooldown × (1 − HasteBonus),最低 0.15 s
```

HasteBonus = 局内被动 Haste(每级 +0.08)+ 遗物临时加成(Arcane Core +0.30),范围 0–0.7,结果受 0.15 s 下限约束。例:基础冷却 1.0 s,Haste 2 级(+0.16)→ 0.84 s。

**最终范围**

```
Area = WeaponBaseArea × (1 + AreaBonus)
```

AreaBonus 只来自营地铁匠铺的 Area(每级 +0.10,3 级封顶),范围 0–0.3。

**生命上限**

```
MaxHP = 100 + 10 × CampMaxHPLevel + 20 × RunVitalityLevel
```

例:营地 2 级、局内 1 级 → `100 + 20 + 20 = 140`。

## Edge Cases

| 情况 | 处理 |
|---|---|
| 闪避方向是墙 | 冲刺在墙前停下,冷却照常计算 |
| 同一帧受到多次伤害 | 只结算第一次,然后进入 0.5 s 无敌 |
| 复活后立刻再次被围 | 2 s 无敌 + 击退冲击波保证有逃生窗口 |
| 搜索中被打 | 不中断搜索(否则尸潮里永远搜不完);闪避或移出范围才中断 |
| 武器满 4 把后升级池又出现新武器 | 新武器不再进入升级池(见 `run-progression.md`) |
| 冷却被压到下限以下 | 按 0.15 s 下限取值 |

## Dependencies

- 上游:`game-flow`(只在 InRun 状态响应输入)、`run-progression`(武器数据)、`camp-meta`(开局属性)。
- 下游:`enemies-spawning`、`loot-inventory-heat`、`extraction-death`、`ui-hud` 依赖本系统提供的生命、位置、交互事件。

## Tuning Knobs

| 参数 | 默认值 | 安全范围 | 影响 |
|---|---|---|---|
| 基础移速 | 600 cm/s | 450–750 | 太慢被围死,太快无压力 |
| 基础生命 | 100 | 60–150 | 前期容错 |
| 闪避距离 | 400 cm | 250–600 | 逃生能力 |
| 闪避冷却 | 2.0 s | 1.0–4.0 | 闪避的价值 |
| 受击无敌 | 0.5 s | 0.3–1.0 | 被围时的容错 |
| 复活恢复比例 | 50% | 30–100% | 复活的价值 |
| 冷却下限 | 0.15 s | 0.1–0.3 | 性能和特效密度 |
| 镜头距离 | 1800 cm | 1400–2400 | 视野与可读性的平衡 |

## Visual/Audio Requirements

- 受击:角色闪白 0.1 s + 轻微镜头抖动 + 受击音效。
- 闪避:残影或拖尾 + 风声。
- 低生命(< 30%):屏幕边缘红色暗角 + 心跳声。

## Game Feel

- 移动必须"跟手":按下就走、松开就停。
- 武器打中要有反馈:敌人受击闪白、击退一点点,死亡时碎裂或倒下。
- Hit-stop:只在击杀精英或 Boss 时用(0.05 s),小怪不用,否则割草会一卡一卡的。

## UI Requirements

HUD 上显示生命条、闪避冷却、武器图标及冷却。详见 `ui-hud.md`。

## Cross-References

- `design/gdd/run-progression.md`(武器清单)
- `design/gdd/camp-meta.md`(局外属性)
- `design/gdd/extraction-death.md`(死亡结算)

## Acceptance Criteria

- **AC1(Must)**:WASD 八方向移动,松开立刻停下;移速 600 cm/s,误差 ±5%。
- **AC2(Must)**:闪避 0.2 s 内移动约 400 cm,期间被敌人碰到不掉血;冷却 2 s 内再按无效。
- **AC3(Must)**:起始武器在没有任何输入的情况下自动攻击并击杀小怪。
- **AC4(Must)**:受击后 0.5 s 内再次被碰,不掉血。
- **AC5(Must)**:生命归零且没有复活次数时,本局进入死亡结算。
- **AC6(Should)**:有 1 次复活时,生命归零后以 50% 生命复活,周围敌人被推开,复活次数变为 0。
- **AC7(Must)**:150 只敌人同屏时,输入延迟没有可察觉的增加,帧率 ≥ 60 FPS。

## Open Questions

- 是否允许鼠标朝向(角色面朝鼠标)?默认不做;需要时只影响 `Facing` 类武器。
