# Day 2 (13 Nov) · Seven ideas, and asking the agent to attack its own plan

[Dev log index](../README.md) · [← previous](02-the-moment-i-stopped.md) · [next →](04-the-redesign-and-how-it-was-checked.md)

## Seven ideas, six thrown away

The redesign did not arrive in one step. This is the real sequence, including
who threw each idea out.

| # | Idea | Outcome |
|---|---|---|
| 1 | **A "CUT!" button during the run.** A film strip on the HUD; one press clears the screen and deletes the current scene's payout. | Dropped by Claude's own review: see the second half of this entry. |
| 2 | **An "unfinished build".** Start grey and silent; restore colour, sound and features in the camp. | Dropped by the same review: the judge's first minute would be the worst version of the game. |
| 3 | **Reel-order multiplier.** Weapons fire in film order, slot *n* deals ×*n*. | Dropped by me. |
| 4 | **A scene-number chain.** Upgrades carry numbers 0 to 4 and must be played in order. | Dropped by me. |
| 5 | **Waves plus a gold shop**, as in Brotato. | Dropped by me, for time. |
| 6 | **Remove heat, relics and extraction** to simplify. | Dropped by me. They work and were verified by playing. |
| 7 | **Cut one, keep two.** | Kept. |

My reasons for the two I rejected on design grounds, translated:

- On #3: *if the order is fixed, it triggers its multiplier every time. A card
  game is fun because you have to decide what to play; here there is nothing to
  decide.*
- On #4: *that only fits a card game. In a survivors-like the attacks are
  automatic, so look at Brotato instead: free upgrades in the level, a shop
  between levels, and merging duplicates.*

On #5 and #6 my answer was about the project, not the idea:

> We do not need to start from scratch. Fuse the new idea with what we already
> have. Ideally 1+1 is more than 2; at worst it is 2; it must never be less
> than 2.

## Asking the agent to attack its own plan

Before building ideas 1 and 2 I asked Claude to check its own draft brief. It did
it against the actual code, and found six problems. The three that sank the
draft:

1. **"The current scene" did not exist.** The film cards fire at second zero and
   the code has no run timer. A film-strip HUD and a "cut the current scene"
   button had nothing to attach to.
2. **A bad first minute.** If the game started grey, silent and missing its
   dodge, and a death on the first run loses everything, a judge could play
   fifteen minutes and never see the game get better.
3. **The button had a loophole.** With no scene playing it cost nothing, so it was
   a free screen-clear, and "clear only non-boss enemies" contradicted "cut the
   boss scene".

It also noticed that the draft had no audio work of its own, and that the code
contains no line that plays a sound. Reading the document would not have shown
any of that; reading the code did.
