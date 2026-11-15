# Day 4 (15 Nov) · The hero and the enemies faced the wrong way

[Dev log index](../README.md) · [← previous](02-preparing-the-submission-materials.md) · [next →](04-housekeeping-config-and-unused-packs.md)

## What the author saw

Playing the editor build, the author pressed D and the hero walked right but looked down. All
four directions were the same: the hero was turned a quarter turn too far.

## Why

Both pack models (the hero and the monsters, from the same author) face +Y in their own space.
The game turns the actor to face where it moves, which is +X for "forward", and the model was
attached with a yaw offset of 0. So every model was a quarter turn off.

The enemies had the same offset of 0 for all five grunt types and the boss. Day 3's entry on the
first real models had already said so: "I did not check from the models' own forward direction
that they look where they walk." The old boss screenshot showed it: the chest monster's mouth
points to the bottom of the screen while it walks right, towards the hero.

## The fix

One number each: the hero's `HeroModelYaw` default from 0 to -90, and the yaw offset of the five
grunts and the boss to -90. The hero's value is read only in the constructor, so changing the C++
default is what counts; editing the property in the editor does not.

## What was checked

- It compiles (the Development editor target).
- The author played it: the hero, the grunts and the boss look where they walk. That is the only
  check of the facing. I could not tell from the code alone whether all five grunts needed the
  same -90, and I did not take a new screenshot.

## What is not done

The screenshots in `production/qa/evidence/` from before this fix show the old facing. If one of
them goes into the submission, retake it.
