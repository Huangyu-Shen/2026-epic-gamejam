# Day 3 (14 Nov) · Closing the stories without pretending

[Dev log index](../README.md) · [← previous](08-story-010-first-number-pass.md) · [next →](10-status-so-far.md)

Building a story and closing it are two steps here. After each of stories 008, 009
and 010 I ran the template's `/story-done` skill, which walks every acceptance
criterion, asks what covers it, and prints a verdict before anything is marked
Complete.

## What the check does

- It reads each criterion and asks for its evidence: a screenshot kept in the
  repository, a log line from a probe, an automated test, or a person's answer.
- A behaviour or feel criterion is never passed from reading the code. Either I say
  "yes, it passes", or it stays untested, or it is marked as needing a playtest.
- It compares the files changed with the files the story said it would touch, and
  lists any extra as out of scope.
- It gives one of four verdicts, and only I decide whether to close.

Review gates (a second reviewer agent) are off at this project's solo setting, so every
story's "reviewed by" line reads: self, no independent review. That is a fact to
state, not a quality mark.

## Story 010 as the example

Story 010 closed as **complete with notes**, not complete:

- **4 of 12 criteria passing.** The 8 that need a human (two runs from a deleted save,
  a first-time player extracting, building upgrades paid from two runs, the Cut Knight
  fight, heat on the trip to extraction, and others) are marked deferred, not passed.
- **The frame-rate criterion** ("holds 60 or more") had an average of 118 and a
  worst single frame of 13. The skill asked me; I counted the average and said so,
  and the notes still record the 13.
- **The bot run that never finished** in the editor stays in the notes as unexplained.
- **A deviation from the story's own wording:** it said only data values change, and
  I added bot code to see them. Listed as advisory.

## Why close at all

The alternative was leaving three stories "in progress" until a human has played them,
and with a deadline, that means never. Closing with the gaps written into the file keeps
the list of what is still unchecked next to the story, where the next reader (me, on
Day 4) will see it.

## What this does not prove

That the stories are good. It proves that each claim in the file has a named source,
and that the ones with no source are labelled.
