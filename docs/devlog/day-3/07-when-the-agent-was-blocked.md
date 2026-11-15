# Day 3 (14 Nov) · When the agent was blocked

[Dev log index](../README.md) · [← previous](06-story-009-unlockable-options.md) · [next →](08-story-010-first-number-pass.md)

This one is short, and it matters for the theme: I am directing an AI that works with
real permissions, and the permission system sometimes says no.

## What happened

To add the new rows for story 009 to the data tables, Claude had used a Python script
through the engine's command-line tool. That worked twice. When it wanted to adjust one
row (a stronger Chain), the same command was refused by the tool's permission checker,
with a reason I could not map to what was being asked ("undermining oversight"), and a
second try with the same command was refused too.

## What Claude did, and did not do

- It did not retry the command or look for another way to write the same data. The
  refusal message itself says to do the rest of the work, then stop and ask.
- It carried on with everything that did not depend on the blocked step: build,
  tests, probes, the packaged run, the documents.
- It put the consequence into the record instead of hiding it: the table kept the
  first Chain numbers, so the C++ fallback was changed to match them, and the story's
  notes say a stronger row was wanted but the table edit was blocked.

## What I did

When the next story (010) needed table edits, the first question Claude put to me
named the blocked command, listed every value as "from, to", and asked whether it could
write them. I said yes, and the same command ran without a problem.

## Why I am logging it

I cannot say why it was refused once and allowed later; it may be a classifier
judging a long script that rewrites data. What I can say is that the useful behaviour
was the boring one: stop, say what you wanted and why, keep working on the rest, and
let the person decide. An agent that works around a block is the thing I would be
afraid of.
