# Day 3 (14 Nov) · Reviewing the new stories before building them

[Dev log index](../README.md) · [next →](02-story-008-lost-film-reels.md)

Day 3 is the day with the two hard gates from my own cut rules: at 18:00 every
Should item that has not started is cut, and at 23:00 features freeze. Before
building anything I asked Claude to review the five stories it had just written
for the rest of the v2 design (stories 008 to 012) and to find the holes.

## How the review worked

The stories were written from the v2 brief. The review read each one against two
things: the master PRD (the tiers, the cut rules and the success criteria) and the
actual source tree, using searches to check every claim of the form "this
function exists" or "this value is set here". Anything a story took for granted
was either confirmed in the code or turned into a new acceptance criterion. Every
addition is marked "Review 2" in the story file, so the original text and the
added text can be told apart.

Everything below was found by reading. None of it was found by running the game.

## What it found

| Story | The hole | What was added |
|---|---|---|
| 008 Lost film reels | A container could roll the same reel type twice, so the second reel cost a bag slot and heat and unlocked nothing. | A reel type already in the bag, or whose option is already unlocked, never drops. A reel with no option mapped never drops. |
| 008 | The v1 save stores relics by enum number. Story 009 adds enum values, so a number saved today could point at a different option tomorrow. | Save unlocked options by name. |
| 008 | The relic-use code was not the only dead code: the dismantle path, the heat drop and the buff struct would also have been left behind. | A list of everything to remove together. |
| 009 Unlockable options | Carrying a reel out is only half of the judge's experience. Level-up offers are a random shuffle, so a short second run might never show the new option. | The new option is guaranteed in the first offer of the next run and marked "NEW". |
| 009 | "Decide the effects when the story starts" would have blocked the start. | A default set of three effects (two passives and a weapon), clearly marked as defaults to change before coding if I prefer others. |
| 009 | The number of permanent cuts is computed from the table sizes. Once the tables hold locked rows, a new save would get 6 cuts instead of 3. | Compute it from the filtered pool. |
| 010 Number retune | The 150-enemy frame-rate result was measured in the editor on Day 1, before any v2 work. | Re-measure on the packaged build, after the retune. |
| 010 | Every criterion assumes the player extracts, but nothing said a first-time player can. | A criterion that the first run of a new save can be extracted by someone who has not seen the game. |
| 011 Art, audio, feel | Sounds referenced only through settings are skipped by the cook without an error, which is exactly what happened to the run map on Day 2. | Reference them hard or cook their folder, and check by ear in the package. |
| 011 | A Shipping build writes no log by default, and the probes report through the log. | Take the evidence from a Development package of the same content first. |
| 011 | The free audio source I had in mind is limited to non-commercial use, and nobody has checked whether a jam with cash prizes counts. | A criterion that I confirm the licence before any file is imported. |
| 012 Cut Knight | The boss spawns at second zero, but the first cut comes at level 2. If the boss ends the run first, nobody sees the feature. | Check this by playing before building anything. |
| 012 | The boss's weapon attacks need the real weapon visuals from story 011; drawn with the debug-line code they would be invisible in a Shipping package. | A dependency on the code part of 011. |
| All | Nothing owned Day 4: packaging on a clean machine, the zip size, the video, the dev log, the public link. | A new story, 013, for the submission. |
| Epic page | Stories 001 to 007 still said "Ready" and the goal was the v1 description. | Statuses corrected, goal rewritten, and a day plan added. |

## The order, because the gates will not wait

Adding up the estimates, the five open feature stories come to two medium, one
small and two large pieces of work, which is more than one day. So the order is
the decision:

1. Story 008 in full.
2. Story 009, minimum version: one passive, wired end to end to its reel.
3. The code part of story 011: real weapon visuals, one audio entry point, hit
   feedback.
4. A first number pass for story 010 on the packaged build.
5. The rest of story 009 and story 012, only if started before 18:00.
6. After the freeze: asset swaps, the final number pass, then the submission story.

## What I had to decide, and what Claude did

Claude proposed the defaults and the order. Two things are mine to confirm: the
three default upgrade effects in story 009, and whether the audio licence allows
use in a prize jam. Neither is settled yet.
