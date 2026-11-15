# Day 3 (14 Nov) · Status so far

[Dev log index](../README.md) · [← previous](09-closing-the-stories.md)

This entry is a snapshot. It replaces the earlier one, which was written after
story 008, and it will be rewritten as the day goes on.

## Done

- The five new stories reviewed and corrected; a submission story added.
- Story 008: lost film reels, with data tables, a new save version and probe evidence
  from the editor and the package.
- Story 009: Haste, Magnet and Chain, unlocked by reels and shown as NEW in the next
  run's first level-up; checked in the editor and the package; 12 automated tests pass.
- The whole code wiki re-checked against the source, with the defects found logged in
  its known-issues page.
- Story 010: the pace of a run measured with a bot, and four numbers changed in
  three data tables (spawn rate and ramp, grunt contact damage, hero health). Closed
  as complete with notes: 4 of 12 criteria passing, 8 deferred to a human playtest.
- This dev log split by day and topic.

## Not done

- Story 010's human playthrough: two runs from a deleted save, a first-time player
  extracting, two building upgrades from two runs, the Cut Knight fight, heat on the
  trip to extraction.
- Story 011 (art, audio, feel). Weapon attacks are still drawn as debug lines, which
  a Shipping package would not show. There is no audio code. The hero and enemies are
  still grey. Its code part is the next piece of work.
- Story 012 (the boss with the rejects) and story 013 (the submission).
- None of the findings in the wiki's known-issues page was fixed.

## Not checked

- Whether the game is fun. No other person has played it yet.
- A reel picked up from a real crate by hand; the camp, run, camp loop in the package
  by hand; the level-up loadout line with all ten options owned; Chain's reach at
  higher stars.
- An odd bot run in the editor: the hero at a constant 24 of 150 health among 200
  enemies for two game minutes.
- A frame-rate hitch: average 118 frames a second in the package, but the worst single
  frame was 13.

## The clock

The 18:00 gate cuts every Should item that has not started, and the 23:00 gate freezes
features. Stories 008 to 010 are closed; the code part of story 011 has to be in before the
freeze.
