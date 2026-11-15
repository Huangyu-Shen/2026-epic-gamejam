# Day 3 (14 Nov) · Story 009: three options that only a film reel can unlock

[Dev log index](../README.md) · [← previous](05-checking-the-code-wiki-against-the-source.md) · [next →](07-when-the-agent-was-blocked.md)

Story 008 made a lost film reel unlock an option, but there was nothing to unlock.
Story 009 adds the three options, and with them the first moment where the whole
idea can be seen: carry a reel out, and next time something new is in the level-up.

## What was added

| Option | Kind | Unlocked by | What it does |
|---|---|---|---|
| **Haste** | passive | Fast Cut reel | Every weapon attacks faster: the interval is divided by 1 plus 0.12 per stack. |
| **Magnet** | passive | Wide Shot reel | Experience gems fly to you from further away: +150 cm per stack on a 600 cm base. That base used to be a hard-coded number; it is now in the progression table. |
| **Chain** | weapon | Chain Scene reel | Hits the nearest enemy, then jumps to the nearest enemy not yet hit, up to three jumps. Stars widen its reach. |

I chose these three in the story review, as defaults to change before coding if I
preferred others; I used them as they were. Each starts locked. On a new save the
level-up pool is the same seven options as before, with three permanent cuts. Every
unlock adds one option and one cut. An option's id is its data-table row name, which
is also the name a reel unlocks and the save stores.

## "NEW!" in the first offer of the next run

The brief's promise is that you carry a reel out and see the new option in your next
run. The offers are a random shuffle, so a short second run could simply never show
it. So the newly unlocked options go to the front of the pool in the next run's first
offer, marked NEW!

![The first level-up of a run after three reels were carried out: all three new options, each marked NEW!](../../../production/qa/evidence/story-009-unlockable-upgrades/1-first-offer.png)

![A Chain attack in the same game: the green line runs through enemies, and the loadout line at the top left lists the new options with their stars.](../../../production/qa/evidence/story-009-unlockable-upgrades/4-chain-fire.png)

## How it was checked

- **Three new automated tests** (suite: 12, all pass): a locked option is not
  offered until unlocked; the pool and the permanent cuts are 7 and 3 with no unlock,
  8 and 4 with one, and 10 and 6 with all three; an unlocked id that matches no option
  changes nothing.
- **The probes, run as the real flow.** One run carries three reels out. A second,
  separate launch on the same save starts a level-up probe. The log reads pool 10,
  cuts 6, and a first offer of Haste, Chain and Magnet, each NEW. A fresh save as a
  control shows pool 7, cuts 3 and none of them.
- **The numbers show the effects.** Haste at three copies and two stars gives attack
  speed 1.54 (that is 1 plus 3 x 0.12 x 1.5). One copy of Magnet gives a pickup
  radius of 750 cm.
- **A hook for the weapon probe.** The weapon component now announces every attack and
  how many enemies it hit. The probe uses it to take a screenshot in the very frame of
  the first Chain attack that hits two or more enemies. Story 011's audio and hit
  feedback can use the same hook.
- **The same two runs in a packaged build**, where a Chain attack hit three enemies.

## Two problems the runs showed

1. **The NEW marker did not survive a restart.** My first version kept it in memory.
   The level-up probe on a relaunched game showed Greed, Might and Swiftness in the
   first offer, not the new options. The marker is now saved in the save file. The
   mistake was invisible to the unit tests, which have no restart.
2. **A crash in a tool, from a null.** The first build crashed the editor's
   command-line tool: the progression code looked up the camp stock while an editor
   world, which has no game instance, was starting up. The lookup now tolerates it.
   It would also have broken the editor's preview worlds.

## What was not checked

- The loadout line with all ten options owned at once. The longest line I saw had six
  entries, and it already wraps to two lines.
- Chain's reach at two and three stars. The code scales it; nothing measured it.
- Playing any of it by hand in a normal run.
- All three options can never be kept in one run, because the first offer holds exactly
  the three new options and one of them is cut for good. That follows from the
  rules, but it means Haste and Magnet were seen working in different runs.
- Chain's numbers (damage 12, interval 1.1 s, three jumps) are a first guess and
  probably too weak. I wanted a stronger row, and the next entry says why it stayed.
