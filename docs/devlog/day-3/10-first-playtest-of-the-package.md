# Day 3 (14 Nov) · The first playtest of the package, and the rule I had backwards

[Dev log index](../README.md) · [← previous](09-closing-the-stories.md) · [next →](11-new-level-up-rule-and-ui.md)

Story 011 was packaged first with sound, music and real weapon effects. I played it
and wrote down what I found. The notes below are the person's own words, shortened.

## What the player (the author) reported

- The music is fine.
- Upgrading a building in the camp makes no sound.
- There is no quit button.
- The extraction point is hard to tell apart. Standing in it shows no progress and
  makes no sound, and extracting successfully makes no sound either.
- The most serious point: **the level-up choice is the wrong way round.** The game
  asked which option to cut. Most players, the author included, expect to choose
  which one to take.
- Later, from a screenshot of the level-up panel: too much information, too little
  contrast, unclear boundaries between parts. The camp's start and quit buttons are
  also hard to see.
- There are too few weapons and passives, so Cut has nothing to show. Cut is useful
  when there are many options and you remove the ones that get in the way of the
  build you want. With a handful of options, cutting leaves you unable to build
  anything strong.

## What I did about it

I did not start coding. I asked the author to talk it through, in Chinese, because
this was a design problem and the wording mattered. The author's answer:

> Let the player pick one of three, and put a Cut button beside it so the player may
> cut one of the three. If all three are wanted, no cut at all. What is cut still
> goes to the boss. The player's choice is the main thing.

One question was left open: what happens to the options that are neither taken nor
cut? I offered three answers. The author chose: **they go back into the pool.**

## What this changed in the design

- Before: three options, cut one, keep the other two.
- After: three options, take one, and optionally cut one first. The cut one leaves the
  pool for the run and goes to the boss. One cut per level-up, while permanent cuts
  remain.

This contradicts the rule written in the game brief and the master PRD (brief v2).
The design documents still described the old rule at this point; they were corrected
later the same night (see [entry 16](16-status-so-far.md)).

## What this shows about the process

The stories that built the old rule were closed against it and
passed their own criteria. The rule itself had never been played by a person.
The playtest found in a few minutes what the checks could not.
