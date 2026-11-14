# Day 1 (12 Nov) · The first three stories, and a frame-rate probe

[Dev log index](../README.md) · [← previous](02-planning-before-code.md) · [next →](04-what-went-wrong-and-what-was-skipped.md)

## The project and the stories

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

![Extracting: the zone fills while the hero stands in it. The text at the top is a placeholder debug readout.](../../../production/qa/evidence/story-002-extraction-death/01-extracting.png)

![Story 003: a horde of grey cubes chasing the hero. Still debug text for the HUD.](../../../production/qa/evidence/story-003-combat-enemies-map/01-fight-horde-boss.png)

## Performance, measured

The target was 60 FPS or better with 150 enemies on screen. I did not trust an
estimate, so Claude added a small command-line probe that spawns the horde,
samples frame time and logs one line. Result:

```
PERFPROBE enemies=152 avgFPS=132.3 minFPS=50.0 samples=1324
```

The average is far above target. The single worst frame is below it. I accepted
that for now and marked it advisory.
