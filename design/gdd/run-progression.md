# Run Progression

> **Status**: Designed
> **Author**: The developer + Claude
> **Last Updated**: 2026-11-12
> **Last Verified**: 2026-11-12
> **Implements Pillar**: P3 Short takes (grow strong fast within a run)

## Summary

Vampire Survivors-style in-run growth: kills drop XP gems, filling XP triggers a level-up, and each level-up lets the player pick 1 of 3 options (a new weapon, a weapon upgrade, or a passive boost). This file defines the XP curve, the upgrade pool rules, and the per-level numbers for every weapon and passive. In-run growth resets to zero when the run ends.

> **Quick reference** — Layer: `Core` · Priority: `MVP` · Key deps: `player-combat`, `enemies-spawning`, `camp-meta`

## Overview

The run starts with only 1 starting weapon. Each level-up adds a choice, and a run reaches roughly level 10-15, assembling a build of the player's own. The camp Editing Room can add new weapons to the upgrade pool, which is how "restoring cut footage" shows up in the run.

## Player Fantasy

"From a broken sword to a screen full of blade arcs, orbs and fire rings." Every level-up is a small decision, and every run's build comes out different because of the choices made.

## Detailed Design

### Core Rules

1. **XP gems**: when an enemy dies, it drops an XP gem where it fell. Once the player enters the pickup radius, the gem is pulled toward the player, and touching it grants the XP.
2. A gem disappears after 60 s on the ground; when more than 300 gems are on screen, the oldest gems merge into one large gem, their XP added together, so none is lost.
3. **Level-up**: as soon as current XP >= the XP required, the player levels up and the overflow XP carries to the next level. Gaining a lot of XP at once can level up several times in a row, and each level pops up one choice.
4. **Upgrade choice**: enter the LevelUp state (game paused) and show 3 option cards; the player picks 1. No skipping, no rerolls (Must).
5. **Slots**: at most 4 weapons and at most 4 passives (the Must tier has exactly 4 passives, so passives are effectively unlimited).
6. **Upgrade pool composition** (recomputed at each level-up):
   - Weapons already held and not at max level -> "upgrade" options
   - When weapon slots are not full: weapons not held and already unlocked -> "new weapon" options
   - Passives not at max level -> "passive" options
7. **Draw rule**: draw 3 from the upgrade pool by weight, without replacement. When the pool has fewer than 3, fill with "Craft Services" (restores 30 HP); when the pool is empty, all 3 options are Craft Services.
8. **Weapon unlocks**: the 3 Must-tier weapons are in the pool from the start; the 2 Should-tier weapons enter the pool only after being unlocked in the camp Editing Room (see `camp-meta.md`).
9. When the run ends, all in-run levels, weapons and passives reset to zero.

### States and Transitions

Level-up flow: `InRun -> LevelUp (paused) -> choose -> recheck whether XP is enough for the next level -> InRun or LevelUp again`.

### Interactions with Other Systems

| System | Interaction |
|---|---|
| `enemies-spawning.md` | The XP drop value of each enemy type |
| `player-combat.md` | Weapons fire using this file's numbers; passives modify character stats |
| `camp-meta.md` | The Editing Room unlocks new weapons; camp stats and in-run passives stack additively |
| `loot-inventory-heat.md` | The relic Warlord's Sigil grants 2 level-ups directly; the Magnet passive only affects XP gem pickup, not supplies |
| `game-flow.md` | Requests entering and leaving the LevelUp state |

## Formulas

**XP required to level up**

```
XPToNext(L) = round(5 + 6 × (L − 1)^1.35)
```

| L | 1 | 2 | 3 | 5 | 8 | 10 | 15 |
|---|---|---|---|---|---|---|---|
| XPToNext | 5 | 11 | 20 | 44 | 85 | 121 | 228 |

Expectation: a 5-minute run kills about 500-700 monsters and reaches level 10-12; 10 minutes reaches level 14-16. This pace gives a level-up every 20-40 seconds.

**Option weights**

| Type | Weight |
|---|---|
| Upgrade of a held weapon | 1.2 |
| New weapon | 0.8 |
| Passive | 1.0 |

Example: holding Sword Arc (level 2), with 3 empty weapon slots, 2 passives not at max level, and 2 weapons not held -> the pool has 5 entries, total weight `1.2 + 0.8×2 + 1.0×2 = 4.8`; the probability that the Sword Arc upgrade is drawn first is `1.2 / 4.8 = 25%`.

**XP gem pickup radius**

```
PickupRadius = 300 cm × (1 + 0.3 × MagnetLevel)
```

MagnetLevel ranges 0-5, giving a radius of 300-750 cm.

## Weapon Data

All weapons cap at level 5. "Amount" is the number fired at once; the camp Smithy's Amount stat gives +1 to all projectile weapons (marked *).

**Sword Arc** (starting weapon, Must) — targeting: `Facing`; swings a fan-shaped slash in the facing direction.

| Lv | Damage | Cooldown | Arc angle | Radius | Notes |
|---|---|---|---|---|---|
| 1 | 10 | 1.0 s | 120° | 250 cm | |
| 2 | 13 | 1.0 s | 120° | 250 cm | |
| 3 | 16 | 0.9 s | 180° | 275 cm | |
| 4 | 20 | 0.9 s | 180° | 300 cm | |
| 5 | 25 | 0.8 s | 360° | 300 cm | Ring slash |

**Arcane Bolt** (Must) — targeting: `Nearest`; fires homing orbs. Range 1500 cm, speed 1500 cm/s. *

| Lv | Damage | Cooldown | Amount | Pierce |
|---|---|---|---|---|
| 1 | 8 | 0.9 s | 1 | 0 |
| 2 | 10 | 0.9 s | 1 | 0 |
| 3 | 12 | 0.9 s | 2 | 0 |
| 4 | 15 | 0.8 s | 2 | 1 |
| 5 | 18 | 0.8 s | 3 | 1 |

**Fire Ring** (Must) — targeting: `Around`; a permanent fire ring that damages enemies inside it every 0.5 s.

| Lv | Damage per tick | Radius |
|---|---|---|
| 1 | 4 | 300 cm |
| 2 | 5 | 330 cm |
| 3 | 6 | 360 cm |
| 4 | 8 | 400 cm |
| 5 | 10 | 450 cm |

**Throwing Axes** (Should, unlocked in the Editing Room) — targeting: `Random` (within 800 cm); thrown in an arc, infinite pierce. *

| Lv | Damage | Cooldown | Amount |
|---|---|---|---|
| 1 | 20 | 1.6 s | 1 |
| 2 | 25 | 1.6 s | 2 |
| 3 | 30 | 1.5 s | 2 |
| 4 | 38 | 1.5 s | 3 |
| 5 | 46 | 1.4 s | 3 |

**Lightning Rune** (Should, unlocked in the Editing Room) — targeting: `Random` (within 1200 cm); lightning strikes at a landing point, area damage radius 150 cm. *

| Lv | Damage | Cooldown | Strikes |
|---|---|---|---|
| 1 | 15 | 1.4 s | 2 |
| 2 | 19 | 1.4 s | 2 |
| 3 | 24 | 1.3 s | 3 |
| 4 | 30 | 1.3 s | 3 |
| 5 | 36 | 1.2 s | 4 |

## Passive Data (Must, all capped at level 5)

| Passive | Effect per level | At max |
|---|---|---|
| Might | Damage +10% | +50% |
| Haste | Cooldown −8% | −40% |
| Vitality | Max HP +20, and restore 20 immediately | +100 |
| Magnet | XP pickup radius +30% | +150% |

## Edge Cases

| Situation | Handling |
|---|---|
| A large XP gain causes 3 level-ups in a row | 3 choices pop up in a row, and the game does not resume in between |
| Level-up happens during the extraction channel | LevelUp pauses, and the channel pauses too; it continues after the choice |
| All weapons and passives are at max level | Only Craft Services appears |
| A gem drops somewhere the player cannot reach (inside a wall) | When spawned, the XP gem is projected to the nearest walkable point |
| Warlord's Sigil used while in the LevelUp state | Cannot happen: the bag cannot be opened in the LevelUp state |

## Dependencies

- Upstream: `enemies-spawning` (XP source), `camp-meta` (weapon unlocks).
- Downstream: `player-combat` (uses this system's weapons and passives), `loot-inventory-heat` (Warlord's Sigil calls this system's level-up interface), `ui-hud` (the level-up screen).

## Tuning Knobs

| Parameter | Default | Safe range | Affects |
|---|---|---|---|
| XP curve base | 5 | 3-10 | How fast the first few levels come |
| XP curve coefficient | 6 | 4-10 | Overall level-up frequency |
| XP curve exponent | 1.35 | 1.2-1.6 | How much level-ups slow down late |
| Option count | 3 | 3-4 | Richness of the choice |
| New weapon weight | 0.8 | 0.5-1.5 | How fast the build takes shape |
| Craft Services heal amount | 30 | 20-50 | The consolation prize after everything is maxed |
| Gem lifetime | 60 s | 30-120 s | Whether the player needs to go back and pick them up |

Every cell in each weapon's data table is a tunable parameter, and all of them live in a single weapon data table.

## Visual/Audio Requirements

- Level-up: the screen flashes briefly + a rising sound effect; option cards flip out one after another.
- Gems: three looks, small (1), medium (>= 5) and large (>= 25); a trail while being pulled in.
- Each weapon has its own hit sound and effect color, so they can be told apart in a melee.

## Game Feel

Upgrade options must be readable at a glance: icon + name + one line describing the change ("Damage 13 → 16"). The delay from popping up to being clickable is <= 0.3 s.

## UI Requirements

The level-up choice screen, the XP bar and level on the HUD, and the weapon and passive icon bar. See `ui-hud.md`.

## Cross-References

- `design/gdd/player-combat.md` (damage and cooldown formulas)
- `design/gdd/camp-meta.md` (Editing Room weapon unlocks)
- `design/gdd/loot-inventory-heat.md` (Warlord's Sigil)

## Acceptance Criteria

- **AC1 (Must)**: Starting from level 1, killing 5 small monsters worth 1 XP each reaches level 2 and pops up 3 options.
- **AC2 (Must)**: After choosing a new weapon, that weapon starts attacking automatically at once.
- **AC3 (Must)**: Once a weapon reaches level 5, it no longer appears in the options.
- **AC4 (Must)**: After holding 4 weapons, "new weapon" options no longer appear.
- **AC5 (Must)**: During LevelUp, enemies do not move and the timer does not run.
- **AC6 (Should)**: Before the Editing Room unlocks Throwing Axes, it never appears in the options; after unlocking, it can appear.
- **AC7 (Must)**: In a 5-minute test run, the median level-up interval is between 15 and 45 s.

## Open Questions

- Should weapon evolution be added (max-level weapon + matching passive -> evolved form)? Decided as Could tier, not doing it by default.
