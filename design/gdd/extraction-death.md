# Extraction & Death

> **Status**: Designed
> **Author**: The developer + Claude
> **Last Updated**: 2026-11-12
> **Last Verified**: 2026-11-12
> **Implements Pillar**: P1 Every pickup is a choice (it only counts if you bring it out), P3 Short takes

## Summary

Defines how a run ends: stand in an open extraction point for the full duration → success, and the bag's materials are multiplied by the payout multiplier and stored in camp; health reaching zero or voluntarily giving up → death, the entire bag is lost and only a small NG Film compensation is granted; time running out → counts as death.

> **Quick reference** — Layer: `Core` · Priority: `MVP` · Key deps: `loot-inventory-heat`, `directors-cut-editing`, `map-level`, `game-flow`

## Overview

The map has fixed extraction points, which open one after another on a schedule. The player decides when to extract: extracting early is safe but yields little, extracting late yields more but enemies keep getting stronger. Extraction requires holding out inside the point for a period of time, and this progress bar is the most tense moment of a run.

## Player Fantasy

"Five seconds left on the countdown, and the enemies have already surrounded me." Rushing to the extraction point with a bag of treasure, holding out until the bar fills, and hearing "Cut! That's a Wrap!" — this moment is the climax of the game.

## Detailed Design

### Core Rules

**Extraction points**

| Extraction point | Tier | Default open time | Location (see `map-level.md`) |
|---|---|---|---|
| North Gate | Must | 3:00 | North side |
| East Docks | Must | 5:00 | East side |
| West Bridge | Should | 7:00 | West side |

1. Before opening, an extraction point is shown as closed with a countdown; once open it stays open until the run ends.
2. **Extraction**: when the player stands within a 400 cm radius of an open extraction point, the progress bar starts automatically. The bar takes 30 s (15 s when Golden Reel is active).
3. Leaving the radius during the bar resets progress to zero. Taking damage and dodging (as long as the player remains within the radius) do not interrupt it.
4. When the bar completes → extraction succeeds and the run ends immediately.
5. The open time of extraction points is affected by footage cards (duration cards and extraction cards, defined in `directors-cut-editing.md`; see the formula below for the conversion).

**Run duration and timeout (Should)**

- Default duration (Theatrical Cut) is 10:00; duration cards can change it to 6:00 or 14:00.
- With 60 s remaining, a full-screen message "Final Reel — 60 seconds!" appears and the background music speeds up.
- If time runs out without extracting, it counts as death (MIA).
- The Must tier has no duration cap: once an extraction point opens it stays available until the player extracts or dies.

**Extraction success settlement**

1. The count of each material in the bag x this run's payout multiplier M, rounded to the nearest integer.
2. Relic counts are unchanged and are not multiplied.
3. The settlement result is written to the camp stash and auto-saved (see `game-flow.md`).
4. The "That's a Wrap!" results screen shows: Take number, survival time, kill count, in-run level, count of each item, payout multiplier, and final stored amount.

**Death settlement**

1. Triggers: health reaches zero with no revives left; choosing Abandon Take in the pause menu; timeout.
2. The bag and all items on the ground are lost.
3. **NG Film compensation (Should)**: grants some Arcane Dust directly to the camp stash; amount per the formula. It represents "this take was an NG, but the footage can still be cut into something else".
4. The "NG — Take N" results screen is black and white and shows survival time, kill count, lost items, and the NG compensation received.

### States and Transitions

| Extraction point state | Description |
|---|---|
| Locked | Not yet open time, shows a countdown |
| Open | Can extract, has a light beam |
| Extracting | Player is inside the point, shows the progress bar |
| Removed | Removed by the Locked Exits card, does not appear this run |

The three in-run end results: `Extracted`, `Died`, `MIA` (timeout). The latter two share the same settlement rules, differing only in the screen text.

### Interactions with Other Systems

| System | Interaction |
|---|---|
| `loot-inventory-heat.md` | Reads bag contents; Golden Reel modifies open time and bar duration |
| `directors-cut-editing.md` | Reads the payout multiplier M, duration, and extraction cards |
| `player-combat.md` | Receives the death event "health reached zero with no revive" |
| `camp-meta.md` | Writes to the stash |
| `game-flow.md` | After the run ends, enters the RunResult state |
| `map-level.md` | Extraction point locations |
| `ui-hud.md` | Extraction point direction markers, open countdown, progress bar, the two results screens |

## Formulas

**Extraction point open time**

```
OpenTime_i = BaseOpenTime_i × (Duration / 600) + LateExitsDelay
```

| Variable | Meaning | Default | Range |
|---|---|---|---|
| BaseOpenTime_i | Default open time of extraction point i (seconds) | 180 / 300 / 420 | — |
| Duration | Run duration (seconds) | 600 | 360 / 600 / 840 |
| LateExitsDelay | 120 when the Late Exits card is slotted, otherwise 0 | 0 | 0 or 120 |

Example: with Short Cut slotted (360 s) → North Gate `180 × 0.6 = 108 s` (1:48); adding Late Exits → 3:48.

**Stored amount on successful extraction**

```
Stored_m = round(BagCount_m × M)
```

M is this run's payout multiplier (defined in `directors-cut-editing.md`, range 0.75-3.0). Example: bag has 20 Iron, M = 1.5 → 30 Iron stored.

**NG Film compensation (Should)**

```
NGDust = min(1 + floor(SurvivedSeconds / 60), 5)
```

Example: survived 3:40 → `min(1 + 3, 5) = 4` Arcane Dust. Even surviving 10 s grants at least 1. The cap of 5 is less than the value of one supply crate, so dying is never worthwhile, but the player does not leave empty-handed.

## Edge Cases

| Situation | Handling |
|---|---|
| Death on the same frame the bar completes | Death takes priority (see `game-flow.md`) |
| Level-up during the bar | LevelUp pauses the bar; it resumes after the choice |
| Extracting with an empty bag | Allowed; the results screen shows "Nothing in the can", still counted as a successful run |
| All extraction points removed | Impossible: Locked Exits removes at most 1, and at least 1 is always kept (see `directors-cut-editing.md`) |
| Boss standing on the extraction point | No special handling; the player can go to another extraction point |
| Mid-bar at the moment of timeout | Counts as MIA; the bar is void |
| Has a revive but chooses Abandon Take | Goes straight to death settlement; the revive does not take effect |

## Dependencies

- Upstream: `loot-inventory-heat` (bag), `directors-cut-editing` (multiplier and schedule), `player-combat` (death event), `map-level` (locations).
- Downstream: `camp-meta` (storing), `game-flow` (entering settlement), `ui-hud` (screens).

## Tuning Knobs

| Parameter | Default | Safe range | Effect |
|---|---|---|---|
| Extraction bar duration | 30 s | 10-45 s | Tension of extraction |
| Extraction radius | 400 cm | 250-600 | How easily the player is forced out of the circle |
| Default open times | 3:00 / 5:00 / 7:00 | 1:30-9:00 | Shortest run length |
| Default duration | 10:00 | 6:00-15:00 | Longest run length |
| Final Reel prompt | 60 s remaining | 30-120 s | Warning before timeout |
| NG compensation cap | 5 | 0-10 | How consoling death is |

## Visual/Audio Requirements

- Extraction point: when closed, a dimmed set light; when open, a light beam lights up, visible from afar.
- Bar: a ring of ground light around the extraction point gradually fills; in the final 5 s a clapper-loader countdown sound plays ("Five, four...").
- Success: "Cut! That's a Wrap!" in large text + applause or a fanfare.
- Death: the screen turns black and white, with film scratches and a red "NG" stamp.

## Game Feel

The last few seconds of the bar should be "thrilling": the music intensifies and the camera pushes in slightly. The moment of success should feel like a release. The death screen should be short, so the player immediately wants another run.

## UI Requirements

On the HUD: the extraction point direction markers and open countdown, the progress ring, the Final Reel prompt, and the two results screens. See `ui-hud.md`.

## Cross-References

- `design/gdd/directors-cut-editing.md` (payout multiplier M, duration, extraction cards)
- `design/gdd/loot-inventory-heat.md` (bag, Golden Reel)
- `design/gdd/camp-meta.md` (stash)

## Acceptance Criteria

- **AC1 (Must)**: North Gate shows a countdown and cannot be used before 3:00, and lights up after 3:00.
- **AC2 (Must)**: Standing in an open extraction point for 30 s succeeds in extracting and enters the "That's a Wrap!" results screen.
- **AC3 (Must)**: Leaving the radius mid-bar and coming back restarts the bar from 0.
- **AC4 (Must)**: Extracting with 20 Iron in the bag and M = 1.5 increases the camp stash by 30 Iron.
- **AC5 (Must)**: After death all bag items vanish and the camp stash gains no materials (except the Should-tier NG compensation).
- **AC6 (Should)**: Dying after surviving 3:40 grants the camp 4 Arcane Dust.
- **AC7 (Should)**: If still not extracted at 10:00, the run settles as MIA, with the same rules as death.

## Open Questions

- Should leaving the radius mid-bar "reset" or "pause" progress? Default is reset (more tense); if playtest feedback says it is too frustrating, change it to pause.
