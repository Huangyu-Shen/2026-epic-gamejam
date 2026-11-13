# Day 1 — 12 November: from a theme to a playable loop

[← Dev log index](README.md) · [Day 2 →](day-2-2026-11-13.md)

Goal for the day: pick a design that fits the theme, plan it small, and get the
core loop running as grey boxes. Nothing pretty, only a loop that can be played.

## 1. Reading the theme

The theme is **Director's Cut**. The announcement text talks about the feature
you scoped out at 2 AM, deciding what makes the cut, and directing your AI
agents. I took a first reading from the film side: a director's cut is a
different edit of the same film.

I wanted that reading to be a mechanic, not decoration, so I planned two layers:

1. **Pre-run editing.** Before a run you slot "film cards". A card forces a horde
   or a boss and raises the payout. Difficulty comes from your edit, not from
   dice.
2. **An Editing Room** in the camp that "restores cut footage" and unlocks cards.

I rejected a draggable timeline editor on cost: the UI alone would eat a day.
Card slots give a similar decision for a fraction of the work.

The genre pairing is deliberately familiar: a survivors-style auto-attack game
plus extraction-style loot. The intended twist was **heat**: every rare item in
the bag makes enemies stronger, and relics could be burned on the spot for a
buff. So every pickup asked "keep it or use it?". (This is replaced on Day 2.)

## 2. Planning before code

Claude and I wrote, in this order:

- a requirements plan (21 numbered requirements) and a one-line "sticky note"
  for the game;
- a **master PRD** that sorts every system into Must, Should or Could, with
  cut rules written in advance ("if the core loop is not fun by tonight, change
  only heat, relics and the bag, and nothing else");
- one short design document per system, 11 in all.

The master PRD is the scope gate. Anything not in it is not built. I did this
first because the whole jam is four days and I work alone; deciding what *not*
to build is cheaper on paper than in code.

## 3. The project and the stories

I created a C++ project for Unreal Engine 5.8 after the jam started, with the
Unreal MCP plugin enabled so Claude can drive the editor. Each system became a
story with acceptance criteria. Claude implemented, compiled, played, and
captured evidence for each one.

| Story | Result |
|---|---|
| 001 Loot bag and heat | A bag with limited slots, search points, a heat meter, three relic types. |
| 002 Extraction and death | Stand in a zone to extract; die and lose the bag. |
| 003 Combat, enemies and map | Move and dodge, one auto-attack weapon, a chasing enemy, a spawn scheduler with hordes and a boss, a grey test map. |

All gameplay numbers (enemy health, spawn rate, caps, hero health) live in data
tables, not in code, so they can be tuned without recompiling.

![Extracting: the zone fills while the hero stands in it. The text at the top is a placeholder debug readout.](../../production/qa/evidence/story-002-extraction-death/01-extracting.png)

![Story 003: a horde of grey cubes chasing the hero. Still debug text for the HUD.](../../production/qa/evidence/story-003-combat-enemies-map/01-fight-horde-boss.png)

## 4. Performance, measured

The target was 60 FPS or better with 150 enemies on screen. I did not trust an
estimate, so Claude added a small command-line probe that spawns the horde,
samples frame time and logs one line. Result:

```
PERFPROBE enemies=152 avgFPS=132.3 minFPS=50.0 samples=1324
```

The average is far above target. The single worst frame is below it. I accepted
that for now and marked it advisory.

## 5. What went wrong

**The hero could not die.** Everything compiled and ran, but at zero health
nothing happened. The death handler was missing one `UFUNCTION()` macro, so the
event binding silently never connected. Claude found it only by playing until
health reached zero. The fix was one line; the lesson was that a successful build
says nothing about whether a feature works.

**A rename left stale paths.** The project folder was renamed from `Game_01` to
`Game_01 5.8` when the first stories were committed. Paths in the README and
`.gitignore` kept pointing at the old name, and Day 2 started with cleaning them
up.

## 6. What I did not do

- **No playtest with another person.** The plan's exit criterion for Day 1 was
  to test "keep it or use it" with one or two people. I did not. I put it off and
  kept building, which is exactly the risk the cut rules were meant to prevent.
- No art and no audio, on purpose, until the loop works.

## Evidence

- [Story 001](../../production/qa/evidence/story-001-loot-bag-heat/) ·
  [Story 002](../../production/qa/evidence/story-002-extraction-death/) ·
  [Story 003](../../production/qa/evidence/story-003-combat-enemies-map/)
