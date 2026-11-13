# Director's Cut Editing (Pre-Run)

> **Status**: Designed
> **Author**: The developer + Claude
> **Last Updated**: 2026-11-12
> **Last Verified**: 2026-11-12
> **Implements Pillar**: P2 You direct the cut (core system), Director's Cut theme

## Summary

Before each run, the player inserts **clip cards** into **film slots** in the camp's editing room, deciding this run's elites, hordes, Boss, extraction points and duration. Each card has a risk value, and the total risk determines the run's material **yield multiplier** (Box Office). New cards are "restored" in the camp's editing room. This is where the "director's cut" theme lands mechanically.

> **Quick reference** - Layer: `Feature` · Priority: `MVP` · Key deps: `camp-meta`, `enemies-spawning`, `extraction-death`

## Overview

The editing room is a panel: the card library on the left, the film slots in the middle, and the run preview on the right (risk, multiplier, timeline). The player can leave everything empty (normal difficulty, multiplier ×1), or stuff in high-risk cards to gamble for a high multiplier. The card combination is locked at departure.

## Player Fantasy

"I'm going to shoot this take harder." I am not passively accepting a random difficulty; I edit this scene like a director: add a horde, bring in the Boss, cut an exit, then step onto the set and play it out myself.

## Detailed Design

### Core Rules

**Film slots**

1. Start with 2 slots (Must). Each level of Film Slots in the camp editing room adds +1, up to 5 (Should, see `camp-meta.md`).
2. A slot can be left empty. The same card cannot be inserted twice.
3. Runtime cards are mutually exclusive: only one can be inserted at a time.
4. The extraction card Locked Exits can only be inserted when the run has >= 2 extraction points (guaranteeing at least 1 extraction point remains).
5. The editing room remembers the previous card combination and fills it in automatically the next time it opens, reducing repeated work.
6. Departing ("Roll Camera!") locks the card combination and the yield multiplier; they cannot be changed in the run.

**Clip cards**

| Card | Category | Tier | Risk | Effect | How to unlock |
|---|---|---|---|---|---|
| Horde Night | Monsters | Must | 2 | Enables the horde (rules in `enemies-spawning.md`) | Owned at start |
| The Cut Knight | Climax | Must | 3 | Boss enters at 4:00 (3:00 with Short Cut) | Owned at start |
| Elite Patrol | Monsters | Should | 2 | An elite appears every 60 s | Restored in the editing room |
| Locked Exits | Extraction | Should | 2 | Removes the earliest-opening extraction point (North Gate) | Restored in the editing room |
| Late Exits | Extraction | Should | 1 | All extraction points open 2:00 later | Restored in the editing room |
| Short Cut | Runtime | Should | -1 | Run lasts 6:00, extraction point open times move earlier proportionally | Restored in the editing room |
| Extended Cut | Runtime | Should | 2 | Run lasts 14:00, extraction point open times move later proportionally | Restored in the editing room |

- Every card's effect is **deterministic**: there is no random component (pillar P2).
- "Remove the earliest-opening extraction point" sorts by open time and always removes North Gate.
- Card unlock costs are defined in `camp-meta.md`.

**Run preview**

The right side of the panel shows, live for the current card combination:
- Total risk and yield multiplier, for example: "Risk 5 · Box Office ×2.25"
- A timeline (a text list sorted by time), for example:
  - `1:30  Horde incoming`
  - `3:00  North Gate opens`
  - `4:00  The Cut Knight enters`
  - `5:00  East Docks opens`
- The run duration (at Must tier there is no runtime card, so it shows "No time limit").

### States and Transitions

| Panel state | Description |
|---|---|
| Browsing | Browsing the card library, inserting cards by drag-and-drop or click |
| Locked | After clicking Roll Camera!, enters Loading and the card combination can no longer change |

### Interactions with Other Systems

| System | Interaction |
|---|---|
| `camp-meta.md` | Slot count and card unlock state come from the camp editing room |
| `enemies-spawning.md` | Reads whether the horde, elite patrol and Boss are enabled, and the Boss entrance time |
| `extraction-death.md` | Reads the duration, extraction point open times and removals, and the yield multiplier M |
| `loot-inventory-heat.md` | The yield multiplier applies only to materials, not to relics |
| `game-flow.md` | Roll Camera! triggers departure; the loading card shows the card combination name |
| `ui-hud.md` | Editing room panel |

## Formulas

**Yield multiplier (Box Office)**

```
M = clamp(1 + 0.25 × ΣRisk, 0.75, 3.0)
```

| Variable | Meaning | Range |
|---|---|---|
| ΣRisk | Sum of the risk values of inserted cards | -1 to 11 |
| M | This run's material yield multiplier | 0.75-3.0 |

Examples:
- No cards: ΣRisk = 0 -> M = 1.0
- Horde Night + The Cut Knight (the full Must-tier loadout): ΣRisk = 5 -> M = 2.25
- Only Short Cut: ΣRisk = -1 -> M = 0.75
- Five slots, all high risk: ΣRisk = 3 + 2 + 2 + 2 + 2 = 11 -> `1 + 2.75 = 3.75`, clamped to the cap of 3.0

**Rationale for the multiplier**: the full Must-tier loadout (×2.25) means the materials brought out are more than double, enough to make "fight the Boss + withstand the horde" worth the risk; the Boss itself also drops a relic. No cards (×1.0) is the safe choice for beginners.

**Card combination name (shown on the loading card)**

```
No cards -> "Theatrical Cut"; with cards -> card names joined with " + " in slot order
```

## Edge Cases

| Situation | Handling |
|---|---|
| Trying to insert a second runtime card | Not allowed; show "Only one runtime card per take" |
| Inserting Locked Exits when only 1 extraction point remains | Not allowed; show "At least one exit must remain" |
| The previous card combination contains a card that is now unavailable (for example the save was reset) | That card is removed automatically |
| After a slot upgrade | The new slot is empty and no card is filled in automatically |
| Departing with everything empty | Allowed, M = 1.0 |
| Must tier has only 2 slots and 2 cards | Works normally; a judge can insert no cards in the first run and insert cards in the second to feel the difference |

## Dependencies

- Upstream: `camp-meta` (slots, unlocks).
- Downstream: `enemies-spawning`, `extraction-death`, `loot-inventory-heat` (multiplier), `game-flow` (departure), `ui-hud` (panel).

## Tuning Knobs

| Parameter | Default | Safe range | Affects |
|---|---|---|---|
| Multiplier per point of risk | 0.25 | 0.1-0.5 | The reward for taking risk |
| Multiplier floor / cap | 0.75 / 3.0 | 0.5-1.0 / 2.0-5.0 | Yield variance |
| Starting slots | 2 | 1-3 | How rich the choice is in the first run |
| Risk value per card | See card table | -2 to 5 | The weight of each card |
| Late Exits delay | 120 s | 60-180 s | Card difficulty |
| Short Cut / Extended Cut duration | 360 / 840 s | 240-480 / 720-1080 | Run length |

## Visual/Audio Requirements

- Film slots are drawn as a horizontal strip of film, and cards are drawn as "clips" with thumbnails. Inserting plays a film-clicking-into-place "click" sound.
- When risk is high the multiplier number turns an eye-catching gold.
- When the Roll Camera! button is pressed: clapperboard clap sound + film projector start-up sound.

## Game Feel

Inserting cards must be quick: clicking a card inserts it into the first empty slot automatically, and clicking the card in the slot again takes it out. No drag-and-drop is needed (drag-and-drop is built only at Should tier).

## UI Requirements

Editing room panel: card library (including locked cards and unlock hints), film slots, preview area, Roll Camera! button. See `ui-hud.md`.

## Cross-References

- `design/gdd/camp-meta.md` (editing room upgrades and unlock costs)
- `design/gdd/enemies-spawning.md` (specific rules for hordes, elites and the Boss)
- `design/gdd/extraction-death.md` (open-time formula, storage intake formula)

## Acceptance Criteria

- **AC1 (Must)**: Opening the editing room on a new save shows 2 empty slots and 2 available cards (Horde Night, The Cut Knight).
- **AC2 (Must)**: After inserting both cards, the preview shows "Risk 5 · Box Office ×2.25", and the timeline includes the Horde and Boss times.
- **AC3 (Must)**: Departing with The Cut Knight inserted, the Boss enters at 4:00; without it, the whole run has no Boss.
- **AC4 (Must)**: The multiplier applies to materials at extraction settlement (see `extraction-death.md` AC4).
- **AC5 (Must)**: After closing and reopening the panel, the previous card combination is still there.
- **AC6 (Should)**: After inserting Short Cut, the duration shows 6:00 and North Gate's open time shows 1:48.
- **AC7 (Should)**: With one runtime card already inserted, another cannot be inserted, and a hint is shown.

## Open Questions

- Should the first run give judges a "recommended cut" button (one click inserts the recommended cards)? Not built by default; the first-run tutorial hint will suggest "Try adding a scene card for more Box Office".
