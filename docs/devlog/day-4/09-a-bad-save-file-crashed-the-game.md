# Day 4 (15 Nov) · A bad save file crashed the game

[Dev log index](../README.md) · [← previous](08-splitting-the-progression-subsystem.md) · [next →](10-closing-the-register.md)

## What I was doing

The register item TD-014 said the save version was only checked against 2: a save written by a newer game
would be read as version 2 and then overwritten. The fix I planned was a backup before anything is replaced.

## What the check found

To see the backup work I wrote a garbage text file into a save slot and started the game. It did not start a
new camp. It crashed, with an assertion deep in the engine's name table, inside `LoadGameFromSlot`. That is
not caused by this change: the game has always handed whatever file is in the slot to the engine's loader.
So a save file that is empty or half written, for example after a power cut during a write, would have
crashed the game every time it started, and the player would have lost the camp.

## The fix

- Before loading, the first four bytes of the slot's file are read. A real save starts with the tag `GVAS`.
  A file without it is backed up (as `<slot>.sav.unreadable.bak`) and never given to the loader; a new camp
  starts and a warning is logged.
- A save written by a newer version is neither loaded nor overwritten: it is backed up as
  `<slot>.sav.newer-version.bak`.
- A save from an older version is backed up before it is migrated.
- The decisions are pure functions (`FSaveVersionCheck::Classify` and `LooksLikeSaveFile`) with tests.

## How it was checked

I ran the game with the garbage file again: a backup was written, a new camp started, and a run was played
through to the end. Forty-six tests pass. The new tests only cover the decisions; the backup itself was
checked by hand once.

## What is not covered

A file that starts with `GVAS` but is damaged further in can still crash the loader. Enums are still stored
by name in the save, so renaming an enumerator would drop old data. Both are written on the register.
