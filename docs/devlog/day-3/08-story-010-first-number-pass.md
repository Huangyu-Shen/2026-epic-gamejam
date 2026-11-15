# Day 3 (14 Nov) · Story 010: a bot to measure the pace, and the first number pass

[Dev log index](../README.md) · [← previous](07-when-the-agent-was-blocked.md) · [next →](09-closing-the-stories.md)

Story 010 is about numbers: how fast you level, how many enemies come, how much it hurts.
Its acceptance criteria say "measured in one timed playthrough by someone playing
normally". This is a solo project and nobody has played two whole runs yet, so the
first question was a different one: can the pace be checked, at least roughly,
without a person?

## A bot that plays a run

A new command-line flag makes a bot play a whole run in the packaged game or the
editor: the hero walks in a circle while the weapons fire on their own, the bot always
cuts the first level-up option, and time is sped up. Every 30 game seconds it logs
level, health, enemies alive, spawn rate and loadout, and it stops on death or after
ten minutes. It also prints the numbers the game is actually using, so two builds
can be compared line for line. It is a poor player (it never dodges), so it
under-estimates what a human can do. For pace that is a useful floor.

## The old numbers

| Spawn base | Ramp per minute | Grunt contact damage | Hero health | Bot lifetime |
|---|---|---|---|---|
| 0.8 per second | +1.5 per second | 10 | 100 | died at 33, 91 and 97 seconds |

Nothing in the game heals the hero, and by 97 seconds the spawn rate was already above
three grunts a second. A run of three to ten minutes, which the design needs, was not
possible with these numbers. This had not been measured before.

## The sweep

Probe-only overrides let the bot try numbers without touching the data tables. Five
sets, two runs each, in game seconds:

| Set | Spawn base | Ramp | Contact | Health | Bot result |
|---|---|---|---|---|---|
| A (old) | 0.8 | 1.5 | 10 | 100 | died at 91 s and 33 s |
| B | 0.6 | 0.8 | 8 | 150 | died at 181 s; lived 600 s |
| **C** | **0.5** | **0.6** | **6** | **150** | died at 283 s; lived 600 s |
| D | 0.5 | 0.4 | 6 | 200 | lived 600 s; lived 600 s |
| E | 0.6 | 0.5 | 7 | 150 | lived 600 s; lived 600 s |

D and E would leave a human with nothing to fear. A is hopeless. I took C. The variance
is large: the same numbers gave a death at 283 s and a full ten minutes. I read
that as "about right for a bot that never dodges", not as a result to three digits.

## What was written

Claude proposed putting the four numbers of set C into three data tables and asked
me first, listing each as "from, to". I said yes.

| Table, row | Value | From | To |
|---|---|---|---|
| spawn tuning | base rate | 0.8 | 0.5 |
| spawn tuning | ramp per minute | 1.5 | 0.6 |
| enemy tuning, Grunt | contact damage | 10 | 6 |
| health tuning, Hero | max health | 100 | 150 |

Nothing else was changed: the experience curve, the weapons, the boss and the building
costs are as they were.

## After the change

- **Packaged build, new save:** the bot lived ten minutes. Its health fell from 600 to
  240 over the last five, with 42 grunts alive and a spawn rate above six a second, so
  enemies are still a threat at the end. It had its first star by level 5.
- **Editor, three runs:** one death at 566 s, one ten-minute survival, and one run
  that never finished: its log stops at 360 s with the hero at 24 of 150 health, the
  same value for two game minutes, among 200 enemies. Either the bot got stuck or the
  game was running very slowly under that load, and I do not know which. A hero at a
  constant health among 200 enemies is odd and I have left it as an open question.
- **Same numbers in both builds.** The printed numbers match exactly in the editor and
  the package. The old puzzle, hero health 100 in the package and 150 in the editor,
  was my editor save having Clinic upgrades; on a fresh save both now read 150.
- **Frame rate**, packaged, 1920x1080, 151 enemies: average 118 frames a second, but
  the single worst frame was 13 (it was 50 on Day 1, in the editor). I think it is a
  hitch when the probe spawns the whole horde at once, and I have not checked. The
  design says "holds 60 or more", so I left that criterion unticked.

## What is not done

Everything the story says needs a person: two runs inside fifteen minutes from a
deleted save, a first-time player extracting, two building upgrades paid from two
runs, the Cut Knight fight, and whether a reel in the bag makes the trip visibly
harder. The bot never extracts and never meets the boss. The report is in
`production/qa/smoke-2026-11-14.md`.

## One more thing the story asked for

The story says "only data values change". I added harness code (the bot and its
overrides) so that I could see the numbers, which is not a data value. It changes no
game rule, and the story's notes say so.
