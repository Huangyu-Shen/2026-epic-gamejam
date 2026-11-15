# Day 4 (15 Nov) · Splitting the progression subsystem

[Dev log index](../README.md) · [← previous](07-tests-that-were-seen-failing.md) · [next →](09-a-bad-save-file-crashed-the-game.md)

`URunProgressionSubsystem` was the biggest debt on the register: 684 lines, about 45 public methods, used by 14
files. It held three different things: what the hero owns and what it is worth, the rules of a level-up
offer, and the words on the level-up cards.

## The design

I kept its interface exactly as it was, so none of the 14 callers changed, and moved the three things into
plain C++ structs that need no world:

- `FRunLoadout`: copies and stars of every weapon and passive, the default tuning rows, and every passive's effect.
- `FOfferRules`: which options may be offered, the three that are shown, new unlocks first, the cut (one per
  offer, only while permanent cuts remain) and the choice.
- `FOfferText`: option names, the one-line description, the card text and the HUD line of what the hero owns.

The subsystem is left with the parts that need the game world: sounds, messages, XP pickups and the hero.
It ends at 346 lines.

## How each step was done

Each step was the same: write tests first with numbers or strings worked out by hand from the old behaviour,
move the code, build, run all tests, run the level-up probe, then break the new code on purpose and see the
matching tests fail. Tests went from 19 to 41. For example, three Swiftness copies make two stars and are
worth `1 + 3 x 0.10 x 1.5 = 1.45`; that number is in a test, not copied from the code.

## One slip in the check

On the third step my first "break it on purpose" changed two things in one command, but only one change was
applied (a quoting mistake in the command). Only one test failed. I noticed because the test for the other
change stayed green, redid it with that change alone, and it failed as it should. A check that passes tells
you nothing until you have seen it fail for the right reason.

## FireWeapon

`FireWeapon` was a 151-line `switch` with all six weapons. It is now a short dispatcher over `FireBlade`,
`FireBolt`, `FireNova`, `FireChain` and `FireGun`; Bolt and Nova share a `DamageAll`. A pure move, no number
changed. This one has no test, since firing needs enemies in a world. The level-up probe saw the Gun and the
Bolt fire and hit; Blade, Nova, Chain and Orbit were not seen in a probe log. The author played a run after
the change and found nothing wrong.
