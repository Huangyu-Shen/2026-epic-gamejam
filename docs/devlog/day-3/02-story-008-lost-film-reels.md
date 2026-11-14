# Day 3 (14 Nov) · Story 008: relics become lost film reels

[Dev log index](../README.md) · [← previous](01-reviewing-the-new-stories.md) · [next →](03-a-bug-only-the-real-save-showed.md)

I answered "yes, start 008", and Claude built it. This is the first story of the
v2 design that changes how an old system behaves, so the behaviour change is stated
first.

## What changed for the player

- A relic can no longer be used during a run. The number keys that used relics,
  the buffs they gave and the heat drop on use are gone. Keys 1 to 3 only cut a
  level-up option.
- The warehouse in the camp can no longer dismantle relics for Iron.
- Relics are now **lost film reels**. Pick one up and heat rises, as before.
  Carry it out alive and a new upgrade option joins the pool for every later run.
  Die and the reel is lost and nothing is unlocked.
- The result screen names what was unlocked, and the camp shows a "STORAGE"
  list of unlocked options.

![A lost film reel in the bag: the bag panel names the reel, and heat has risen by one reel's worth.](../../../production/qa/evidence/story-008-lost-film-reels/1-reel-in-bag.png)

![Extracting with the reel: the result screen says which option was unlocked.](../../../production/qa/evidence/story-008-lost-film-reels/2-result-extracted.png)

![Dying with the reel: it is lost and nothing is unlocked.](../../../production/qa/evidence/story-008-lost-film-reels/3-result-died.png)

![A new launch of the camp: the unlocked option is still there, so it was saved.](../../../production/qa/evidence/story-008-lost-film-reels/4-camp-storage.png)

## Rules I wanted, and where they live

- **A reel only drops if it is useful.** It must unlock an option that exists, the
  option must not already be unlocked, and the bag must not already hold that reel
  type. With nothing useful left to find, containers give no reel. The vault still
  gives one whenever one can drop, so a first run can reach the experience without
  luck.
- **The mapping is data.** A new table says which reel unlocks which option and how
  often it drops. Until story 009 adds options, only one reel (Fast Cut, which
  unlocks "Haste") is mapped, so only that one drops.
- **Bag, heat and container numbers are data too.** They used to be C++ defaults
  with no way to tune them from the data tables that story 010 needs. Three new
  tables hold them, wired in through the same settings page as the others.
- **Unlocks are saved by name**, in a new version of the save file, and an old save
  is migrated on load.

## A way to make data tables without the editor open

The editor was not running, so Claude created the three new data tables by running
a Python script through the engine's command-line commandlet and then read the rows
back to check them. It is worth knowing that this works, because stories 009 and
010 are mostly table edits.

## How it was checked

1. **Nine automated tests pass.** Five are new: the unlocked set adds once and
   skips "none", reels map to their options, only useful reels drop, the random pick
   never returns a reel with no option, and an old save's relics become options.
2. **An in-game probe, in three modes.** A command-line flag puts a reel in the bag
   and ends the run by extraction (or by death), takes screenshots and logs the
   state; a second flag opens the camp and logs what it loaded. A third flag picks
   a different save slot, so the probes never touch my real save.
3. **The same probe in a packaged build**, which logs that the loot numbers came
   from the table rather than from the C++ defaults. That line exists because a
   missing table falls back to defaults without any error, and I did not want to
   trust that quietly again.

Probe log from the editor run (trimmed):

```
REELPROBE reel-in-bag reelsInBag=1 heat=15.0 unlocked=[] anotherReelCanDrop=0
Upgrade option unlocked: Haste (1 unlocked)
EXTRACTED: bag Iron 25 x 1.00 = +25 to camp, 1 lost reels carried out, 1 options unlocked
REELPROBE after-extraction reelsInBag=0 heat=0.0 unlocked=[Haste ] anotherReelCanDrop=0
```

## What is not checked

- I have not searched a real crate or vault with the E key and picked up a reel by
  hand. The probe puts the reel in the bag directly. The drop rules are covered by
  the unit tests and by the `anotherReelCanDrop` field in the log.
- The whole camp, run, camp loop inside the package, and audio (there is none yet).
- The enum is still called `ERelicType`, because renaming it would touch the save
  format. Only behaviour and text changed.
