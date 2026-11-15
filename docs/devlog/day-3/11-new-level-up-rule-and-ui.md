# Day 3 (14 Nov) · The new level-up rule, a bigger pool and a cleaner UI

[Dev log index](../README.md) · [← previous](10-first-playtest-of-the-package.md) · [next →](12-extraction-sound-and-the-two-zones.md)

## The rule in code

- The level-up shows three cards. Keys 1 to 3 take an option; keys 4 to 6 cut one.
  Each card has a green TAKE button and a red Cut button.
- A cut puts the option on the "cutting-room floor", removes it from the pool for the
  run, and posts "CUT: X now belongs to the Boss". Only one cut per level-up, and
  only while permanent cuts remain.
- Options that are neither taken nor cut stay in the pool.

## More to choose from

The author's second point was that Cut is meaningless with few options. The pool went
from 7 to 13:

- Weapons: **Gun** (bullets with long range) and **Orbit** (balls that circle the
  hero). The author asked for a shooting weapon because two of the three weapons were
  lasers. The starting weapon's range was also too short: Blade range went from 220 to
  380 cm.
- Passives: **Reach** (longer weapon range), **Armor** (less contact damage, capped at
  75%), **Regen** (health per second), **Crit** (a chance to do double damage, capped
  at 90%).

The numbers for these are estimates. Crit, Armor and Regen were only checked to
compile and not crash. The frame rate with the new weapons and 150 enemies was not
measured (the earlier measurement had weapons off).

## The UI

The panels use rounded, outlined boxes, buttons come in three kinds (normal, primary
green, danger red) and the level-up has cards with a title, a level line and a
star-progress line. The camp's START RUN is green at the bottom left and QUIT (Esc)
is red at the top right. Esc also quits during a run.

After this I played it again. The author's words were that it felt fine.

## One thing the automated test cannot see

The test for the pool (`UnlockableOptionsTest.cpp`) uses a hand-written copy of the
pool list, not the real subsystem. I updated the copy to 13 options and the test
passes, but if the real list and the copy ever disagree the test will not notice.
That weakness was already there and is now written down here.
