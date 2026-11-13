# Enemies & Spawning

> **Status**: Designed
> **Author**: The developer + Claude
> **Last Updated**: 2026-11-12
> **Last Verified**: 2026-11-12
> **Implements Pillar**: P1 Every pickup is a choice (heat makes more enemies spawn), P2 You direct the cut (footage cards decide elites/hordes/Boss)

## Summary

Defines the stats and behavior of every enemy, and how a "spawn director" decides when, where, and how many enemies to spawn. Spawn volume is driven by three things together: elapsed time, bag heat, and the footage cards inserted before departure.

> **Quick reference** — Layer: `Core` · Priority: `MVP` · Key deps: `player-combat`, `loot-inventory-heat`, `directors-cut-editing`

## Overview

Enemies pour in continuously from outside the player's view, and their numbers grow over time. The more valuable the bag, the higher the heat, the faster enemies spawn and the tougher they are. Footage cards can add periodic hordes, elite patrols, and the Boss "The Cut Knight". All enemy behavior is simple: chase the player, deal collision damage, and a few ranged attackers. Difficulty comes from numbers, not from complex AI.

## Player Fantasy

"The entire movie's extras are rushing me." The pressure is visible and tangible, and it is self-chosen: if I slotted a horde card and stuffed my bag with treasure, I live with the consequences.

## Detailed Design

### Core Rules

**Spawn director**

1. For the whole run, a single spawn director resolves spawning at a fixed frequency (every 0.25 s).
2. Each resolution accumulates "spawn points" at the current spawn rate (formula below). When points >= 1, one enemy spawns and 1 point is deducted.
3. **Spawn position**: pick a random point on a ring 1600-2200 cm around the player. The point must be outside the camera view, on walkable ground, and not inside an enclosed building. After 5 consecutive failures to find a valid point, fall back to the nearest preset spawn point (see `map-level.md`).
4. **On-screen cap**: 150 in normal conditions, 200 during a horde. Once the cap is reached, spawning pauses and points keep accumulating, up to a maximum of 20 points stored.
5. **Recycling**: a normal enemy farther than 3500 cm from the player is teleported onto the spawn ring around the player (health unchanged). Elites and the Boss are not recycled.
6. **Enemy type selection**: draw from the currently unlocked enemy weight table (the Must tier has only Skeleton Grunt).
7. **Scaling**: enemy health scales with time and heat (formula below); move speed and damage do not grow with time and are affected only by heat.

**Enemy behavior (general)**

- Melee enemies: move straight toward the player, route around obstacles, and push against each other to avoid overlapping; deal contact damage on touching the player.
- Ranged enemies: keep a set distance from the player and fire straight-line projectiles at intervals; projectiles can be dodged through.
- All enemies are knocked back slightly when hit (the Boss is immune) and drop experience gems on death.

**Enemy roster**

| Enemy | Tier | Health | Move speed cm/s | Damage | XP | Behavior |
|---|---|---|---|---|---|---|
| Skeleton Grunt | Must | 20 | 280 | Contact 8 | 1 | Melee chase |
| Ghoul Runner | Should | 12 | 450 | Contact 6 | 1 | Melee chase, spawns in groups of 5 |
| Bone Archer | Should | 15 | 250 | Arrow 6 | 2 | Keeps 800 cm distance, fires an arrow every 2.5 s, arrow speed 900 cm/s |

**Enemy type weights (with the Should tier fully enabled)**

| Time | Grunt | Runner | Archer |
|---|---|---|---|
| 0:00-2:00 | 100 | 0 | 0 |
| 2:00-5:00 | 60 | 30 | 10 |
| After 5:00 | 45 | 35 | 20 |

**Elites (Should)**

- A stronger version of any enemy type: health x6, size x1.5, damage x1.3, with a gold outline.
- Drops 10 XP, plus an "elite chest" (equivalent to the drop of one stash chest, see `loot-inventory-heat.md`).
- Sources: the Elite Patrol footage card (one every 60 s), and the highest heat tier, Infamous (one every 45 s). The two sources are timed independently.

**Horde (Must, from the Horde Night footage card)**

- Starts at 1:30, then every 90 s, each lasting 15 s.
- During a horde the spawn rate is x3, and all enemies come from the same direction (a 90-degree sector centered on the player), with the direction random each time.
- A warning showing the incoming direction appears at the screen edge 3 s before it starts.

**Boss: The Cut Knight (Must, from The Cut Knight footage card)**

- Appears at 4:00 (3:00 if a Short Cut card is slotted). A full-screen message "The Cut Knight has entered the scene." shows 5 s before the appearance.
- Starts from the Boss arena (the graveyard) and walks toward the player.
- Health 2500 (not scaled by time or heat), move speed 300 cm/s, contact damage 15, immune to knockback.
- Skill cycle (2 s between skills):
  1. **Charge**: the ground shows a 1.0 s straight-line telegraph, then it dashes 1200 cm, dealing 25 damage on hit.
  2. **Ground Slam**: the ground shows a 1.2 s circular telegraph (radius 500 cm), then it slams down, dealing 30 damage.
  3. **Rally the Extras** (at most once every 20 s): summons 10 Skeleton Grunts around the Boss.
- On defeat it drops a pile of "vault-grade" loot (1 guaranteed relic + 8 materials, see `loot-inventory-heat.md`) and 50 XP.
- The Boss does not block extraction: the player can skip the Boss and extract directly.
- The second phase is Could tier and is not built by default.

### States and Transitions

| Enemy state | Description |
|---|---|
| Spawning (0.3 s) | Rise-from-the-ground spawn animation; cannot be damaged and deals no damage |
| Chasing / Kiting | Normal behavior |
| Attacking | Ranged enemies shooting, Boss casting skills |
| HitReact (0.1 s) | White flash + knockback |
| Dying (0.5 s) | Death animation, drops spawn, then returned to the object pool |

### Interactions with Other Systems

| System | Interaction |
|---|---|
| `loot-inventory-heat.md` | Reads the current heat tier and converts it into spawn and health multipliers; elite and Boss drops |
| `directors-cut-editing.md` | Reads this run's card combination to decide whether to enable hordes, elite patrols, and the Boss, as well as duration-related schedules |
| `run-progression.md` | Death drops experience gems |
| `player-combat.md` | Receives weapon damage, deals damage to the player |
| `map-level.md` | Preset spawn points, Boss arena, no-spawn zones |
| `ui-hud.md` | Horde direction warning, Boss health bar, Boss entrance message |

## Formulas

**Spawn rate (enemies/second)**

```
SpawnRate(t) = S0 × (1 + t / T_ramp) × HeatSpawnMult × HordeMult
```

| Variable | Meaning | Default | Range |
|---|---|---|---|
| t | Seconds elapsed in this run | — | 0-840 |
| S0 | Spawn rate at the start of the run | 1.5 enemies/s | 0.8-3.0 |
| T_ramp | Time needed for the rate to double | 120 s | 60-240 |
| HeatSpawnMult | Spawn multiplier of the heat tier, defined in `loot-inventory-heat.md` | 1.0-2.2 | — |
| HordeMult | 3 during a horde, otherwise 1 | 1 or 3 | — |

Example: t = 300 s (5:00), heat tier Hunted (x1.7), not a horde → `1.5 × (1 + 2.5) × 1.7 = 8.9 enemies/s`. In practice this is held down by the 150 on-screen cap.

**Enemy health**

```
EnemyHP = BaseHP × (1 + 0.15 × t / 60) × HeatHPMult
```

| Variable | Meaning | Range |
|---|---|---|
| BaseHP | Base health of the enemy type (the value in the table; elites are further x6) | 12-120 |
| HeatHPMult | Health multiplier of the heat tier, defined in `loot-inventory-heat.md` | 1.0-1.6 |

Example: at 5:00, a Skeleton Grunt with heat Noticed (x1.15) → `20 × 1.75 × 1.15 = 40.25`, rounded to 40. At this point the starting weapon Sword Arc at level 3 (16 damage) needs 3 slashes, so the player feels clear pressure, which pushes them toward upgrading or extracting.

**Enemy damage**

```
EnemyDamage = BaseDamage × (Elite ? 1.3 : 1.0)
```

Does not grow over time; the pressure comes from numbers.

## Edge Cases

| Situation | Handling |
|---|---|
| Player stands in a map corner, most of the ring falls outside the map | Spawn only on valid points; on consecutive failures use preset spawn points |
| Player has already extracted when the Boss would appear | The run is over; the Boss does not spawn |
| Boss gets stuck in terrain | Check every 5 s; if it has not moved within 3 s, teleport it to a valid point 1500 cm from the player |
| Heat tier changes mid-horde | New multipliers apply immediately, without waiting for the horde to end |
| Boss spawn comes up while the on-screen cap is reached | The Boss is not limited by the cap and always appears |
| Both elite sources trigger at the same time | Spawn two; they are not merged |

## Dependencies

- Upstream: `loot-inventory-heat` (heat multipliers), `directors-cut-editing` (card combination), `map-level` (spawn points and Boss arena), `player-combat` (player position and damage).
- Downstream: `run-progression` (experience), `loot-inventory-heat` (elite and Boss drops), `ui-hud` (warnings and Boss health bar).

## Tuning Knobs

| Parameter | Default | Safe range | Effect |
|---|---|---|---|
| S0 | 1.5 enemies/s | 0.8-3.0 | Early-run pressure |
| T_ramp | 120 s | 60-240 | How fast pressure grows |
| Health time growth | 0.15 / minute | 0.05-0.3 | How tough late-run enemies are |
| On-screen cap | 150 | 100-250 | Performance vs. pressure |
| Spawn ring | 1600-2200 cm | 1200-3000 | Where enemies appear from |
| Recycle distance | 3500 cm | 2500-5000 | Whether enemies are always nearby |
| Horde interval / duration / multiplier | 90 s / 15 s / ×3 | 60-150 / 10-25 / ×2-4 | Risk of the horde card |
| Elite interval | 60 s | 30-120 | Risk of the elite patrol card |
| Boss health | 2500 | 1500-4000 | Boss fight length (target 40-70 s) |
| Boss appearance time | 4:00 | 2:30-6:00 | Player's preparation time |

## Visual/Audio Requirements

- Each enemy's silhouette and color are distinguishable at a glance: Grunt bone-white, Runner sickly green, Archer deep purple; elites have a gold border.
- Ranged attacks and Boss skills must have a ground telegraph (a red-outlined area).
- Boss entrance: the camera pulls back briefly for 1 s, and a low drumbeat comes in.

## Game Feel

Small enemies die "one slash each", with crisp death feedback. From mid-run they take two or three hits, making the player feel the situation deteriorating. The Boss's skill telegraphs are clear, and dodging them should give an "I dodged it" thrill.

## UI Requirements

Horde direction warning, Boss entrance caption, Boss health bar. See `ui-hud.md`.

## Cross-References

- `design/gdd/loot-inventory-heat.md` (heat tier multiplier table)
- `design/gdd/directors-cut-editing.md` (card risks and schedules)
- `design/gdd/map-level.md` (spawn points, Boss arena)

## Acceptance Criteria

- **AC1 (Must)**: Within 10 s of the run starting, Skeleton Grunts keep appearing off-screen and chasing the player.
- **AC2 (Must)**: After heat rises from Calm to Hunted, the spawn count over the next 30 s rises noticeably (log count >= 1.5x).
- **AC3 (Must)**: With the Horde Night card slotted, a direction warning appears at 1:30, and over the following 15 s enemies pour in from the same direction in large numbers.
- **AC4 (Must)**: With The Cut Knight card slotted, the Boss appears at 4:00; all three of its skills have telegraphs, and on defeat it drops a loot pile containing a relic.
- **AC5 (Must)**: With no Boss card slotted, no Boss appears for the whole run.
- **AC6 (Must)**: The on-screen enemy count never exceeds 150 (200 during a horde).
- **AC7 (Should)**: With the Elite Patrol card slotted, a gold-bordered elite appears every 60 s, and on defeat it drops an elite chest.
- **AC8 (Must)**: With 150 enemies on screen, frame rate >= 60 FPS (dev machine, 1080p).

## Open Questions

- Should enemy pathfinding use a navigation mesh or direct chase plus obstacle avoidance? Left to the implementation plan, decided by performance.
