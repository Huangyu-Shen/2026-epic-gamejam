# Day 3 (14 Nov) · Status so far

[Dev log index](../README.md) · [← previous](16-the-forest-floor.md)

This entry is a snapshot. It replaces the earlier one, written after story 010, and it
will be rewritten as the work goes on.

## Done

- Stories 008, 009 and 010 closed (010 as complete with notes: 4 of 12 criteria
  passing, 8 deferred to a human playthrough).
- Story 011 code part: audio and music, visible weapon effects, extraction beacon and
  progress, quit button, restyled camp and level-up UI, new weapons and passives
  (13 options). The author played the package and had no further complaints.
- The level-up rule changed after the first playtest: take one of three, optional cut.
- Story 012 closed as complete: the Cut Knight boss uses what you cut. Played by the
  author with 1 or 2 cuts.
- The void around the map: hero, enemies and boss stay on the floor. Confirmed in the
  package.
- The boss, the void fix, the design and wiki corrections and the dev log are merged to
  main (pull requests 4 and 5).
- The floor (a meadow with a rim of trees and rocks), the loot containers (stumps and boulders)
  and the extraction point (a portal and the green beam) use the forest pack
  ([entry 16](16-the-forest-floor.md)). Checked in the editor and in a package.
  **Not committed.**
- The hero, the five grunt types and the boss use Fab models ([entry 15](15-the-first-real-models.md)).
  Checked in the editor and in a package with the 150-enemy test (about 130 FPS on warm runs, weapons off).
  **Not committed**; the model packs are deliberately not in git.
- Leaving a run: an Esc menu in the camp and in a run (Resume, Settings with three
  volume levels, Return to camp in a run, Quit game). Built after the author described it
  ([entry 14](14-esc-menu-and-leaving-a-run.md)). Checked in the editor with a test flag;
  **not committed**, and the packaging of it was started but not tried by hand.

## Not done

- Story 011 is still In Progress: the camp scene is still plain shapes, the boss attacks are
  orange balls and a red ring, and star-ups only make beams thicker.
- Story 013 (package, texts, video, final checks).
- The design docs do not yet mention the Esc menu and its settings (a PRD Should item the
  author asked for).
- None of the wiki's known-issues findings was fixed.
- The extra check on a real development save (moving it to Magnet and Chain) was not run.

## Not checked

- The Esc menu with a real mouse, Esc after clicking a button, and the camp's menu.

- A build that used all its cuts against the boss.
- Whether the game is fun for anyone but the author.
- Crit, Armor and Regen beyond "compiles and does not crash". Gun and Orbit numbers
  are estimates.
- Frame rate with the new weapons and 150 enemies. The only measurement had weapons
  off.
- Story 010's 8 human-playthrough criteria.
- The camp map's edge.

## The clock

Story 012 was started after its 18:00 cut-off, on the author's decision (see
[entry 13](13-story-012-boss-and-the-void.md)). The 23:00 feature freeze still stands.
Day 4 is for art swaps, story 013 and the submission; the deadline is 16 Nov, 01:59 UK
time.
