# Day 4 (15 Nov) · Tests that were seen failing

[Dev log index](../README.md) · [← previous](06-first-refactor-batch.md)

The register says the project had only 12 tests, all on small helper functions. The refactor of
`RunProgressionSubsystem` that comes next needs more than that, so this entry adds some first.

## What was added

Seven tests, 19 in all:

- **Settlement (4).** An extracted bag is multiplied and rounded down; a death loses the whole
  bag, stone and reels included; no outcome settles nothing; a negative multiplier never gains
  less than zero.
- **Level-up offers (3).** An option a lost film reel just unlocked goes first and is marked NEW;
  with no new unlocks the order is left alone; two options never share an id.

To test the ordering I had to take it out of `GenerateOffer` into a small static function,
`MoveNewUnlocksToFront`. `GenerateOffer` does the same as before.

## Were they worth anything?

A test that has only ever passed is not known to test anything. I broke the code on purpose in
two places: rounding down became rounding up in the settlement, and the swap that moves an option
to the front was removed. I rebuilt and ran everything. Exactly two tests failed, the one for each
change. Then I put the code back, rebuilt, and 19 passed.

## What is still not tested

The whole of `GenerateOffer`, loading and migrating a save, building costs and the boss subsystem.
Those need either a world in a test, or more of the code taken out into plain functions, which
is the next refactor. The register item stays open with that written next to it.
