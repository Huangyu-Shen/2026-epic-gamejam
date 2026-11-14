# Day 3 (14 Nov) · Keeping the documentation true to the code

[Dev log index](../README.md) · [← previous](03-a-bug-only-the-real-save-showed.md) · [next →](05-status-so-far.md)

Changing a system makes the documents about it wrong. After story 008, the code
wiki, the loot design document and the story files all described relics that no
longer existed. I did not want a repository whose documents contradict its game,
so the documentation was brought up to date in the same piece of work.

## Who did what

- **A sub-agent updated the wiki and the loot design document.** The brief listed
  what changed in the code, with the file and function names, and told it to
  verify each statement against the source before writing, to touch only those
  documents, and to finish by searching for the old words ("relic", "dismantle",
  "warehouse") and justify every remaining hit.
- **I checked the result.** All wiki links and anchors resolve, there are no Chinese
  characters left, and every remaining mention of the old words is either a code
  name that still exists, the old-save migration, or a note that says it was
  removed.
- **The agent listed what it could not verify**, and I treated that list as the
  to-do list for my own check. It could not read the binary data tables, so I read
  the rows back out of them myself. It had not run the new command-line flags, and
  I had. It left two things alone on purpose: source line counts that are now
  stale, and statements in the loot design document that were already different
  from the code before this change.
- **The story file was closed with completion notes**, not just ticked boxes: what
  was built, what changed for players, what was found by playing, and what was not
  checked.

## Why I keep the "not checked" lists

A document that sounds certain about things nobody has run is worse than a gap.
Every entry in this log, and every story file, ends with what was not verified.
When a later entry contradicts an earlier one, the earlier one is corrected, not
quietly rewritten: Day 2's note about the data tables is an example, where an
unproven claim was softened after the fact.

## This dev log, reorganised

I also asked for this log to be split into folders by day, with one file per topic
inside each day, so that a reader can open the part they care about. Day 1 and
Day 2 were cut up from their single files without changing their content; the
files are numbered in the order things happened.
