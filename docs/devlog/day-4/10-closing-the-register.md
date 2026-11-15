# Day 4 (15 Nov) · Closing the register

[Dev log index](../README.md) · [← previous](09-a-bad-save-file-crashed-the-game.md) · [next →](11-the-first-packages-since-the-refactor.md)

## The author asked a fair question

After the second refactor batch I told the author the refactor was "mostly done". The register still had 12
open items. The author asked why I called it done with 12 open, and why I did not just do them. The honest
answer was that I had sorted them in my head into "needs approval", "too big", "needs a package" and "I
would accept this", and then stopped without telling them what each pile held. I gave the list, the author
approved the two things that needed approval (deleting the stand-in files and changing `project.yaml`), and I
did them in order, one commit per group, testing each.

## What was done

- **Dead code (TD-003).** The stand-in character and game mode (four files) are deleted, with 11 functions
  nobody called.
- **Stale comments (TD-016).** Eighteen comments now describe the current rules.
- **`engine.language` (TD-020)** is C++.
- **Saves (TD-014, TD-021).** See entry 09.
- **Long functions (TD-006).** The enemy manager's tick is four passes, the boss's tick three, the hero's tick
  five, and the loot container's `Interact` two helpers. The UI layout builders and the camp's `BeginPlay` are
  left, because they are straight-line construction where a split would only move locals into members.
- **The HUD (TD-018).** It looked up the extraction points with an actor iterator over every actor in the
  world, every frame. It now finds them once. That was the one real cost; the loadout and bag strings built
  each frame are left, and nothing was measured.
- **Wood (TD-013).** `Iron` is `Wood` in 16 files. The save field and three data-table columns keep their
  names on purpose: renaming them would break old saves or reset the tuning tables. The comments there say so.
- **Tests (TD-011, in part).** The save checks and the building cost and bonus math have tests, 46 in all.
- **Plugins (TD-017).** Three development plugins are limited to the Editor target in the project file. Not
  yet confirmed in a package.

## What was accepted, and why

- **TD-012, the numbers still in code.** Three XP numbers moved into the Progression data row with the same
  defaults. The rest (weapon caps, colours, beam thicknesses, camp layout, forest counts, film card slots) are
  art and layout parameters tied to effect code and the map; moving a number that an existing table row
  already holds would change what the row gives unless the binary table is edited too.
- **TD-015, assets reached by a fixed path.** The packs are kept out of git on purpose and the deliverable is
  a package that holds them; making 33 loads happen at run time risks load hitches and missing assets in the
  package, for a benefit that only someone building from a clone would see.

## What is still open

Three items: tests for the boss subsystem and the glue code (TD-011), confirming in the final package that the
plugins are gone (TD-017), and the frame rate with the weapons on (TD-019).

## A slip

One of my edit scripts looked for a line in the boss file, matched the first copy of it, which was in another
function, and cut the file in the wrong place. The build failed straight away. The file was restored from
git and the script fixed to look inside the right function. Every script since then checks that what it
edits is where it expects.
