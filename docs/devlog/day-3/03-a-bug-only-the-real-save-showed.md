# Day 3 (14 Nov) · A bug that only my real save file showed

[Dev log index](../README.md) · [← previous](02-story-008-lost-film-reels.md) · [next →](04-keeping-the-docs-honest.md)

This is the best example so far of why "it compiled and the tests passed" is not
the same as "it works".

## The bug

To let the game tell an old save from a new one, the save file has a version
number. Claude's first version of the story set the new default for that number to
2. The unit tests passed. The editor probe passed. Then I had Claude load a copy of
my real save, which was written on Day 2 by the old code, with 8 relics in its
warehouse.

The game loaded it without a word:

```
LogCampSave: Loaded camp save: Iron 104, 0 unlocked options, 2 building entries
```

The line that should have come before it, "Migrated a v1 save", never appeared.

## Why

An Unreal save file stores only the values that differ from the class default. The
old file had version 1, which was the old default, so it stored no version at all.
When the new code loaded it, the missing value took the new default, 2, and the
game believed it was reading a current save. The old relics were never looked at.
Nothing failed. The player would simply have lost the relics.

## The fix, and a test that was seen failing

The rule is that the default must stay at 1 ("no version stored means the old
format"), and the game writes 2 explicitly when it saves, which differs from the
default and so is stored.

Before applying the fix, I had Claude change the test first and run it against the
unfixed code. It failed, for the right reason:

```
Error: Test Completed. Result={Fail} Name={V1SaveRelicsBecomeUnlockedOptions}
Expected 'a save with no stored version reads as version 1' to be 1, but it was 2.
```

After the fix, all nine tests pass, and loading the copy of the real save now says:

```
Migrated a v1 save: 8 relics [Warbrand Aegis Aegis Warbrand Aegis Aegis Aegis Warbrand ] -> 0 unlocked options
```

Zero unlocked options is correct here: none of those eight relics is the one type
that is mapped to an option, and the rule is that unmapped relics are dropped. The
migration of a mapped relic is covered by the unit test, not by a real save. My
actual save file was never changed; the loads used a copy in a separate save slot.

## What I take from it

- The first test of this behaviour asserted that the new save version is 2. It
  passed against the bug, because the bug made it 2. A test that asserts the
  intended answer is not a test of the behaviour.
- The only thing that exposed it was real data written by the old code. Fixtures
  written by the new code cannot show a compatibility bug.
- A failing log line is better than a silent one: "Migrated" is now printed with
  the list of relic names, so the next migration can be checked by reading one line.
