# Day 4 (15 Nov) · The first refactor batch

[Dev log index](../README.md) · [← previous](05-tech-debt-scan-and-register.md) · [next →](07-tests-that-were-seen-failing.md)

Done on a branch, merged after the author played it. Six items of the register, and a seventh in
part.

- **TD-001, the probes.** The nine test probes moved out of the game mode into their own world
  subsystem, one function per probe. The game mode went from 596 lines to 19. The subsystem is
  created only when a probe flag is on the command line, and never in a Shipping build.
- **TD-002, the debug keys.** The `K` key kills the hero. It is now bound only in the editor, or
  in a package started with `-DevCheats`, and never in Shipping. A package for players cannot
  kill the hero by key. The probe helpers `DebugGrantWeapon` and `DebugCut` stay public, but only
  the probes call them.
- **TD-007, the saves.** A change to the camp set off a save at once, and an upgrade or a run
  settlement could write the file three times. Changes in one frame now make one write on the
  next tick. The run probe showed the settlement writing the slot **once**, where the old code
  wrote it three times.
- **TD-008 and TD-009, small safety fixes.** Eleven places that fetched a game-instance
  subsystem now cope with a missing game instance, and ending a run with no bag returns with a
  warning. The progression subsystem now declares that it needs the enemy manager first.
- **TD-010, the soft pause (a change in behaviour).** While the level-up panel is open, the
  extraction bar stops filling, and a dash in progress waits. Before, a hero standing in the
  extraction zone could finish extracting behind the panel.
- **TD-003, part.** Removed the unused `EnhancedInput` module, two fields that were written and
  never read, and the code that cleared debug text that no longer exists. The old stand-in
  character files and the uncalled getters are still there; deleting files needs the author's yes.

## Checks

The Development editor build passed. All 12 existing tests passed. `-ReelProbe` settled a run, and
`-OfferProbe` played eight level-ups. The author played the editor build and found no problem
with the pause, the `K` key, the saves or an ordinary run.

## One thing that went wrong on the way

My first script for editing the files did not match, because the files use Windows line endings
and my patterns did not. One edit silently changed nothing and I only found out because an
assertion in the script failed. After that I wrote every edit to read, convert and write back
the line endings it found.
