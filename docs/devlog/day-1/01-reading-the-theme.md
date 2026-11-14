# Day 1 (12 Nov) · Reading the theme

[Dev log index](../README.md) · [next →](02-planning-before-code.md)

Goal for the day: pick a design that fits the theme, plan it small, and get the
core loop running as grey boxes. Nothing pretty, only a loop that can be played.

## Reading the theme

The theme is **Director's Cut**. The announcement text talks about the feature
you scoped out at 2 AM, deciding what makes the cut, and directing your AI
agents. I took a first reading from the film side: a director's cut is a
different edit of the same film.

I wanted that reading to be a mechanic, not decoration, so I planned two layers:

1. **Pre-run editing.** Before a run you slot "film cards". A card forces a horde
   or a boss and raises the payout. Difficulty comes from your edit, not from
   dice.
2. **An Editing Room** in the camp that "restores cut footage" and unlocks cards.

I rejected a draggable timeline editor on cost: the UI alone would eat a day.
Card slots give a similar decision for a fraction of the work.

The genre pairing is deliberately familiar: a survivors-style auto-attack game
plus extraction-style loot. The intended twist was **heat**: every rare item in
the bag makes enemies stronger, and relics could be burned on the spot for a
buff. So every pickup asked "keep it or use it?". (This is replaced on Day 2.)
