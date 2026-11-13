# Camp & Meta Progression

> **Status**: Designed
> **Author**: The developer + Claude
> **Last Updated**: 2026-11-12
> **Last Verified**: 2026-11-12
> **Implements Pillar**: P3 Short takes, visible growth; Director's Cut theme (the editing room)

## Summary

Meta progression is kept "thin": a fixed-camera 3D camp with four buildings, each governing a few Vampire Survivors-style stat upgrades. Upgrades cost only generic materials, and the cost grows exponentially. Relics brought back are salvaged into materials here, or kept in the storehouse as a hoard. The editing room unlocks new clip cards, new weapons and more film slots, that is, "restoring the cut clips".

> **Quick reference** - Layer: `Feature` · Priority: `MVP` · Key deps: `extraction-death`, `loot-inventory-heat`, `game-flow`

## Overview

The camp is the "backstage of the set". After coming back, the player sees materials go into storage, clicks a building to open its upgrade panel, spends materials to raise a stat one level, then goes into the editing room to prepare the next take. Meta progression does not aim to be rich; it has one job: make "bringing things out" worthwhile.

## Player Fantasy

"The set is slowly starting to look like something." After every successful extraction the player can spend the materials right away and watch the numbers grow; a new card lights up in the editing room, and the next run has new gameplay.

## Detailed Design

### Core Rules

**Camp scene**

1. A fixed top-down camera looks over the camp and cannot be moved or rotated. The scene has 4 buildings and one exit (the "Set Gate", decorative).
2. Mouse hover over a building: outline highlight + building name; click: open that building's panel.
3. The top right permanently shows the material counts in storage.
4. Departing is only possible through the editing room panel (see `directors-cut-editing.md`).

**Four buildings and their stats**

Each stat has its own level and is bought level by level using the `camp-meta` cost formula. Stat bonuses take effect once at the start of each run.

| Building | Stat | Tier | Effect per level | Max level | Base cost |
|---|---|---|---|---|---|
| Blacksmith | Might | Must | Damage +5% | 5 | 10 |
| | Area | Must | Area +10% | 3 | 15 |
| | Amount | Should | Projectile weapon count +1 | 1 | 60 |
| Clinic | Max HP | Must | Max health +10 | 5 | 10 |
| | Recovery | Must | Regenerate +0.2 health per second | 3 | 15 |
| | Revival | Should | Revives +1 | 1 | 60 |
| Storehouse | Bag Capacity | Should | Bag +2 slots | 4 | 12 |
| | Greed | Should | Supply point yield +10% | 5 | 12 |
| Editing Room | Film Slots | Should | Film slots +1 | 3 | 15 |
| | Luck | Should | Stash relic chance +5% | 3 | 15 |

**One-time unlocks in the editing room (Should)**

| Unlock | Type | Cost |
|---|---|---|
| Short Cut | Clip card | 8 |
| Elite Patrol | Clip card | 10 |
| Late Exits | Clip card | 10 |
| Locked Exits | Clip card | 15 |
| Extended Cut | Clip card | 15 |
| Throwing Axes | Weapon | 20 |
| Lightning Rune | Weapon | 30 |

Unlocks take effect immediately: cards appear in the editing room's card library, and weapons enter the in-run upgrade pool (see `run-progression.md`).

**Material recipes**

- Must tier: all costs are paid in Iron.
- Should tier:
  - Blacksmith: Iron
  - Clinic: Herbs
  - Storehouse: half Iron and half Herbs (each rounded up)
  - Editing room (stats and unlocks): Arcane Dust

**Storehouse and salvage (Must)**

1. The storehouse panel lists all material counts and the hoarded relics.
2. Relics can be "Salvaged" and converted into materials according to the salvage yield table in `loot-inventory-heat.md`. Salvage asks for one confirmation first and cannot be undone.
3. Relics can also be kept forever, with no cap. Relics in the storehouse **cannot** be taken into a run; they can only be hoarded or salvaged (taking them into a run is "to be done later").
4. At Must tier the storehouse panel is attached to a chest in the camp; at Should tier it becomes the storehouse building's panel.

**Building appearance (Could)**

Each building's appearance has 3 stages based on the sum of all its stat levels: levels 0-2, levels 3-6, and level 7 and above. When the stage changes, a short construction effect plays once.

### States and Transitions

| Camp state | Description |
|---|---|
| Idle | Browsing the camp, hover highlight |
| PanelOpen | A building's panel is open, the background is dimmed, other buildings cannot be clicked |
| EditingRoom | The editing room panel (see `directors-cut-editing.md`) |

Upgrade flow: click "Upgrade" -> check materials -> deduct materials -> level +1 -> refresh the panel -> auto-save. When materials are insufficient the button is grayed out and shows how many more are needed.

### Interactions with Other Systems

| System | Interaction |
|---|---|
| `extraction-death.md` | Successful extraction and NG compensation are written to storage |
| `loot-inventory-heat.md` | Salvage yield table; bag slots, Greed and Luck are provided by this system |
| `player-combat.md` | At the start of a run, Might, Area, Amount, Max HP, Recovery and Revival are applied |
| `run-progression.md` | Weapon unlocks |
| `directors-cut-editing.md` | Film slot count and card unlock state |
| `game-flow.md` | Auto-save timing; save contents |
| `ui-hud.md` | Camp HUD, building panels, storehouse panel |

## Formulas

**Upgrade cost**

```
Cost(L) = round(BaseCost × g^(L − 1))
```

| Variable | Meaning | Default | Range |
|---|---|---|---|
| L | The level being purchased | - | 1-5 |
| BaseCost | The stat's base cost (see table) | 10-60 | - |
| g | Growth base | 1.8 | 1.5-2.2 |

| Base cost | L1 | L2 | L3 | L4 | L5 | Total at max |
|---|---|---|---|---|---|---|
| 10 | 10 | 18 | 32 | 58 | 105 | 223 |
| 12 | 12 | 22 | 39 | 70 | 126 | 269 |
| 15 | 15 | 27 | 49 | - | - | 91 (capped at level 3) |

**Early-pacing check (for requirement R19: 2-3 upgrades within the first 2-3 runs)**

- Run 1: no cards (M = 1.0), extract at 5 minutes, about 20 Iron + 1 relic (about 8-10 when salvaged) ≈ 28 Iron (see the expected yield in `loot-inventory-heat.md`).
- Can buy: Might L1 (10) + Max HP L1 (10) = 20, 8 left. -> **2 upgrades after run 1**.
- Run 2: all Must cards inserted (M = 2.25), about 20 × 2.25 = 45 + Boss relic ≈ 55 Iron.
- Can buy next: Might L2 (18) + Max HP L2 (18) + Area L1 (15). -> **5 upgrades in total after run 2**, which satisfies R19.
- Maxing every Must stat takes about `223 + 91 + 223 + 91 = 628` Iron, roughly 12-20 runs. Jam judges will not buy everything; this is a long tail for people who want to keep playing.

**Start-of-run stat summary**

```
MightBonus(camp) = 0.05 × MightLevel
AreaBonus(camp) = 0.10 × AreaLevel
MaxHP increase = 10 × MaxHPLevel
Recovery = 0.2 × RecoveryLevel (per second)
```

Example: Might level 2 -> +10% damage, consistent with the example in `player-combat.md`.

## Edge Cases

| Situation | Handling |
|---|---|
| Materials exactly enough | Purchase is allowed, leaving 0 |
| Already at max level | The button shows "MAX" and the cost is no longer shown |
| Opening salvage when storage is empty | Show "No relics in storage" |
| Quitting the game partway through a salvage | Salvage completes instantly and is saved immediately, so there is no intermediate state |
| Unlocking a weapon while in a run | Impossible: unlocks can only happen in the camp |
| After a film slot upgrade in the editing room | The new slot is empty |
| The save contains data beyond Must tier, but only Must was finished | Cannot happen: the data structure is designed for all tiers from the start, and unimplemented stats are shown as locked |

## Dependencies

- Upstream: `extraction-death` (storage intake), `loot-inventory-heat` (salvage yield), `game-flow` (saving).
- Downstream: `player-combat` (start-of-run stats), `run-progression` (weapon unlocks), `directors-cut-editing` (slots and cards), `loot-inventory-heat` (bag, Greed, Luck), `ui-hud` (panels).

## Tuning Knobs

| Parameter | Default | Safe range | Affects |
|---|---|---|---|
| Growth base g | 1.8 | 1.5-2.2 | How slow late-game growth is |
| Base cost per stat | See table | 5-80 | How fast early upgrades come |
| Effect per level per stat | See table | ±50% | How much meta progression influences in-run play |
| Unlock cost | 8-30 | 5-60 | How fast new content appears |

## Visual/Audio Requirements

- Camp: backstage of a night-time film set, warm set lights shining on a few tents and wooden huts; the editing room is a wagon or hut with a projector light on.
- Upgrade: after the button is pressed, a stat icon floats up above the building + a "ding" confirmation sound.
- Salvage: the relic shatters into points of light that flow into the material icon.

## Game Feel

Camp interactions get it done in one step: click the building -> click upgrade, two clicks per upgrade. No confirmation popups (except for salvage).

## UI Requirements

Camp HUD (material counts), building panel (stat list, level pips, current value -> next level value, cost, upgrade button), storehouse panel (materials, relics, salvage button). See `ui-hud.md`.

## Cross-References

- `design/gdd/loot-inventory-heat.md` (expected yield per run, salvage yield)
- `design/gdd/directors-cut-editing.md` (slots and cards)
- `design/gdd/run-progression.md` (weapon unlocks)
- `design/gdd/player-combat.md` (stat application formulas)

## Acceptance Criteria

- **AC1 (Must)**: Hovering a building shows an outline and its name, and clicking opens the matching panel.
- **AC2 (Must)**: Holding 28 Iron, the player can buy Might L1 (10) and Max HP L1 (10), leaving 8 Iron; Might L2 shows a cost of 18 and its button is grayed out.
- **AC3 (Must)**: After buying Max HP L1, max health at the start of the next run is 110.
- **AC4 (Must)**: Salvaging one Warlord's Sigil adds 10 Iron to storage and the relic disappears.
- **AC5 (Must)**: After upgrading, quitting and re-entering, levels and materials are unchanged.
- **AC6 (Should)**: After unlocking Elite Patrol in the editing room, it appears in the editing room's card library and can be inserted.
- **AC7 (Should)**: After buying Film Slots L1, the editing room shows 3 slots.
- **AC8 (Must)**: After the first 2 runs of a new save (run 1 with no cards, run 2 with all Must cards inserted, both extracting successfully), at least 2 upgrades have been completed.

## Open Questions

- Can relics in the storehouse be taken into a run (an equipment slot)? Already decided as "to be done later".
