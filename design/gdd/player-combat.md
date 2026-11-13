# Player Control & Combat

> **Status**: Designed
> **Author**: The developer + Claude
> **Last Updated**: 2026-11-12
> **Last Verified**: 2026-11-12
> **Implements Pillar**: P1 Every pickup is a choice (provides the pressure), P3 Short takes

## Summary

The player moves with WASD under an angled top-down camera, weapons attack automatically, and there is a dodge plus one interact key (search, extraction prompts). This file defines the character's base stats, movement, dodge, weapon firing rules, damage calculation, and hit and death handling. The specific weapon list and upgrades are in `run-progression.md`.

> **Quick reference** — Layer: `Core` · Priority: `MVP` · Key deps: `game-flow`, `run-progression`, `camp-meta`

## Overview

The controls are only positioning and timing: the player decides where to stand, when to dodge, and when to stop and loot. All attacks happen automatically, so the player's attention can go to routes, Heat and bag trade-offs.

## Player Fantasy

"Standing alone in the middle of the horde, blade arcs sweeping outward in rings." Early on the player is occasionally pushed into a corner; later, they plow through everything. Dodge is the only active lifesaver, and it feels great when timed right.

## Detailed Design

### Core Rules

**Input (keyboard and mouse, Must)**

| Key | Action |
|---|---|
| W A S D | Eight-direction movement (relative to the screen) |
| Space | Dodge |
| E (hold) | Interact: search a supply point |
| Tab | Open/close the bag (time slows, see `loot-inventory-heat.md`) |
| 1 / 2 / 3 | Quick-use the 1st/2nd/3rd relic in the bag |
| Esc | Pause |
| Mouse | UI only; no aiming during a run |

Gamepad is Could tier: left stick to move, A to dodge, X to hold-interact, Y for the bag, D-pad for quick-using relics.

**Camera**

- Angled top-down, 55° pitch, 1800 cm camera distance, follows the player with slight smoothing (0.1 s).
- Cannot be rotated or zoomed (Must). Walls must not occlude the character: occluders fade to semi-transparent (Should).

**Movement**

- Base move speed 600 cm/s, instant acceleration, no inertia (mow-down games require responsive controls).
- The character faces the move direction; after stopping, it keeps its last facing.
- Collides with enemies, but the player can push small enemies aside a little (enemies have less mass), to prevent being stuck by a single small monster.

**Dodge**

- Dashes 400 cm in the current move direction over 0.2 s; when standing still, dashes in the facing direction.
- Invulnerable during the dash (0.2 s); can pass through enemies, cannot pass through walls.
- Cooldown 2.0 s; the HUD shows the cooldown.
- Dodging does not interrupt weapon firing.

**Weapon firing (general rules)**

- Up to 4 weapons held at once; each weapon has its own independent cooldown and fires automatically.
- Each weapon has a **targeting mode**:
  - `Facing`: toward the character's facing
  - `Nearest`: the nearest enemy
  - `Random`: a random enemy in range
  - `Around`: centered on the character
- When no enemy is in range, `Nearest` and `Random` weapons do not fire (no cooldown is wasted); `Facing` and `Around` fire as usual.
- Weapon stats: damage, cooldown, area, projectile count, duration, pierce. Specific values are in `run-progression.md`.

**Damage and getting hit**

- Player base HP is 100, with no armor (Must).
- Enemies deal contact damage or projectile damage to the player; after being hit, the player has 0.5 s of invulnerability frames, during which no further damage is taken.
- HP regeneration: base 0; sources are the camp Clinic's Recovery stat and relics.
- When HP drops to 0: if the player has a revive charge (the camp Clinic's Revival), they revive immediately; otherwise the run ends in death, handed to `extraction-death.md`.
- Revive effect: restores 50% HP, 2 s of invulnerability, and emits a knockback shockwave centered on the player (radius 600 cm, deals no damage).

### States and Transitions

| Character State | Can Move | Can Dodge | Weapons Fire | Can Be Damaged |
|---|---|---|---|---|
| Normal | Yes | Yes | Yes | Yes |
| Dodging (0.2 s) | Auto-dash | No | Yes | No |
| HitInvuln (0.5 s) | Yes | Yes | Yes | No |
| Searching (holding E) | Yes (interrupted if moving out of range) | Yes (interrupts the search) | Yes | Yes |
| Reviving (2 s) | Yes | Yes | Yes | No |
| Dead | No | No | No | No |

### Interactions with Other Systems

| System | Interaction |
|---|---|
| `run-progression.md` | Provides weapons and passive stats; in-run upgrades modify this system's stats |
| `camp-meta.md` | Meta stats are applied once at the start of a run: Might, Area, Amount, Max HP, Recovery, Revival |
| `enemies-spawning.md` | Enemies deal damage and receive weapon damage |
| `loot-inventory-heat.md` | The interact key searches supply points; quick keys use relics |
| `extraction-death.md` | When HP reaches zero with no revive charges, notifies it that the run ended in death |

## Formulas

**Final damage**

```
Damage = WeaponBase × (1 + MightBonus) × (1 + ItemDamageBonus)
```

| Variable | Meaning | Range |
|---|---|---|
| WeaponBase | Base damage of the weapon at its current level | 5–60 |
| MightBonus | Sum of camp Might (+0.05 per level) and in-run passive Might (+0.10 per level) | 0–0.75 |
| ItemDamageBonus | Temporary bonus from relics (e.g. Arcane Core +0.5) | 0–0.5 |

Example: Sword Arc level 3, base 16; camp Might level 2 = +0.10, in-run Might level 1 = +0.10 → `16 × 1.2 × 1.0 = 19.2`, rounded down to 19.

**Final cooldown**

```
Cooldown = WeaponBaseCooldown × (1 − HasteBonus), minimum 0.15 s
```

HasteBonus = in-run passive Haste (+0.08 per level) + temporary relic bonus (Arcane Core +0.30), range 0–0.7, and the result is bound by the 0.15 s floor. Example: base cooldown 1.0 s, Haste level 2 (+0.16) → 0.84 s.

**Final area**

```
Area = WeaponBaseArea × (1 + AreaBonus)
```

AreaBonus comes only from the camp Smithy's Area (+0.10 per level, capped at 3 levels), range 0–0.3.

**Max HP**

```
MaxHP = 100 + 10 × CampMaxHPLevel + 20 × RunVitalityLevel
```

Example: camp level 2, in-run level 1 → `100 + 20 + 20 = 140`.

## Edge Cases

| Situation | Handling |
|---|---|
| The dodge direction is a wall | The dash stops in front of the wall, and the cooldown is still counted |
| Multiple hits in the same frame | Only the first is resolved, then the 0.5 s invulnerability starts |
| Surrounded again right after reviving | 2 s of invulnerability + the knockback shockwave guarantee an escape window |
| Hit during a search | The search is not interrupted (otherwise it could never finish in a horde); only dodging or moving out of range interrupts it |
| A new weapon appears in the upgrade pool when the player already has 4 weapons | New weapons no longer enter the upgrade pool (see `run-progression.md`) |
| Cooldown pushed below the floor | Taken as the 0.15 s floor |

## Dependencies

- Upstream: `game-flow` (responds to input only in the InRun state), `run-progression` (weapon data), `camp-meta` (starting stats).
- Downstream: `enemies-spawning`, `loot-inventory-heat`, `extraction-death`, `ui-hud` depend on the HP, position and interact events this system provides.

## Tuning Knobs

| Parameter | Default | Safe Range | Affects |
|---|---|---|---|
| Base move speed | 600 cm/s | 450–750 | Too slow and the player gets surrounded; too fast and there is no pressure |
| Base HP | 100 | 60–150 | Early-game forgiveness |
| Dodge distance | 400 cm | 250–600 | Escape ability |
| Dodge cooldown | 2.0 s | 1.0–4.0 | Value of the dodge |
| Hit invulnerability | 0.5 s | 0.3–1.0 | Forgiveness when surrounded |
| Revive recovery ratio | 50% | 30–100% | Value of the revive |
| Cooldown floor | 0.15 s | 0.1–0.3 | Performance and effect density |
| Camera distance | 1800 cm | 1400–2400 | Balance between field of view and readability |

## Visual/Audio Requirements

- Getting hit: character flashes white for 0.1 s + slight camera shake + hit sound effect.
- Dodge: afterimage or trail + wind sound.
- Low HP (< 30%): red vignette at the screen edges + heartbeat sound.

## Game Feel

- Movement must be responsive: it starts the moment a key is pressed and stops the moment it is released.
- Weapon hits need feedback: enemies flash white when hit, get knocked back slightly, and shatter or fall on death.
- Hit-stop: used only when killing an elite or the Boss (0.05 s), not for small monsters, otherwise mowing down enemies will stutter.

## UI Requirements

The HUD shows the HP bar, the dodge cooldown, and weapon icons with their cooldowns. See `ui-hud.md` for details.

## Cross-References

- `design/gdd/run-progression.md` (weapon list)
- `design/gdd/camp-meta.md` (meta stats)
- `design/gdd/extraction-death.md` (death settlement)

## Acceptance Criteria

- **AC1 (Must)**: WASD moves in eight directions and stops immediately on release; move speed is 600 cm/s, with ±5% tolerance.
- **AC2 (Must)**: A dodge moves about 400 cm within 0.2 s, and touching an enemy during it costs no HP; pressing again within the 2 s cooldown has no effect.
- **AC3 (Must)**: The starting weapon attacks automatically and kills small monsters with no input at all.
- **AC4 (Must)**: Being touched again within 0.5 s of being hit costs no HP.
- **AC5 (Must)**: When HP reaches zero with no revive charges, the run enters death settlement.
- **AC6 (Should)**: With 1 revive charge, when HP reaches zero the player revives at 50% HP, surrounding enemies are pushed away, and the revive count becomes 0.
- **AC7 (Should)**: With 150 enemies on screen, there is no perceptible increase in input latency, and the frame rate is ≥ 60 FPS.

## Open Questions

- Should mouse aiming be allowed (the character faces the mouse)? Not done by default; if needed, it affects only `Facing` weapons.
