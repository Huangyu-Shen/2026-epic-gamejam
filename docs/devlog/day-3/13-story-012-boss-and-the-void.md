# Day 3 (14 Nov) · The boss takes what you cut, and the void

[Dev log index](../README.md) · [← previous](12-extraction-sound-and-the-two-zones.md) · [next →](14-status-so-far.md)

## Committing first

Before more code I committed and merged everything so far as pull request 4 (57 changed
files became 95 with the new ones). The four free audio packs, about 690 MB, were
**left out of git on purpose**: they are large, and putting purchased or free
marketplace files into a public repository is probably redistribution under the
marketplace licence. They stay on my machine and in the package. The cost is that a
clone of the repository will have no sound.

## Story 012 started after its own deadline

The PRD said story 012 "cannot begin after Day 3 18:00; if it has not started by then
it is cut". It was past that time. I wrote the situation down and gave the author two
choices: build it (about 1.5 to 2 hours, before the 23:00 feature freeze), or change
the text on screen and in the documents so that it no longer promised the feature. The
interface was already telling the player "this now belongs to the Boss" while the boss
did nothing with it, which is the kind of empty claim a reviewer finds first. The author
said to build it. That is a decision to break the PRD rule, not a case of the rule
being met.

## What was built

A boss only exists when the Cut Knight film card is slotted. A new subsystem watches the
run's cut list and applies each new cut at once:

- **Cut weapon, boss attack.** Blade, Bolt, Chain and Gun give a slow ball flying at the
  hero (the speeds differ). Nova gives a red ring on the floor with a one-second
  wind-up. Orbit gives a ring of eight balls. Cutting the same weapon again adds 25% of
  that attack's base damage.
- **Cut passive, boss stat.** Swiftness: +8% speed. Might: +15% contact damage. Haste:
  attacks come 10% sooner (never faster than half). Vitality: +150 health. Everything
  else (Greed, Magnet, Reach, Armor, Regen, Crit) has no boss equivalent and gives +100
  health, so no cut is ignored.
- **Limits.** Speed and damage bonuses stop at double; bonus health stops at 1500. If
  the boss dies, its attacks vanish and later cuts do nothing. The numbers are in the
  tuning settings, as code defaults, not in a table.
- A line appears on the HUD for every cut: "THE BOSS TAKES GUN - it gets a new attack". (The first packaged build read "it a new attack",
a missing word I only saw in the screenshot afterwards; fixed in the source, not yet in a package.)

**Not done by design:** the boss does not use the player's own weapon component. I gave it
three simple attack kinds instead, which is much less work and is why this took about
an hour and not the full estimate. Boss attack damage is its own value, not the
weapon's.

## How it was checked

- A test flag spawned the boss next to the hero (in god mode), gave it cuts one by
  one and took screenshots: balls, a red warning ring, and a ring of balls. The log
  shows each gain and each hit. The images are kept in
  `production/qa/evidence/story-012-cut-knight/`.
- I packaged it and the author played it. Their words: the red ring and the balls can
  be seen and avoided, and the boss does not kill the hero at the start. They only
  collected one or two cuts per run, so **the criterion "a build that used all its cuts
  can still beat the boss" was not tested**. The story was closed with that gap written
  in its notes.

## The void

The author then found that the map has no walls and a void around it, so the hero falls
off the edge. The enemies and the boss do not fall (they move at a fixed height), but
they walked out over the void.

The fix is a small subsystem that finds the floor when the level starts and keeps the
hero, every enemy, the boss and new spawns inside it. It found one floor, 8000 by 8000
cm. A test flag pushed the hero to x=6000 and it stopped at x=3966; the farthest of 40
enemies and the boss was at 3970. In the packaged build the author confirmed that the
hero, the enemies and the boss all stop at the edge, and the feel is "ordinary". There
is still no visible wall or edge marker. The camp map was not changed.
