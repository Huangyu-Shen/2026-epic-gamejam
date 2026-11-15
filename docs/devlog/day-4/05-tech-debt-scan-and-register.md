# Day 4 (15 Nov) · A tech debt scan and a register

[Dev log index](../README.md) · [← previous](04-housekeeping-config-and-unused-packs.md) · [next →](06-first-refactor-batch.md)

The author said the packaging could wait, and that a pass over the code and its structure
should come before it. I ran the studio's `/tech-debt scan` first, then asked before writing
anything.

## What the scan found

It covered **89** C++ files. `TODO`, `FIXME`, `HACK` and `@deprecated` markers: none. That is
not a sign of a clean codebase: the jam left the debt in other forms. Two files were over 500
lines (`RunProgressionSubsystem.cpp`, 684, and `DirectorGameMode.cpp`, 596) and 22 functions
were over 50 lines, the longest being the game mode's `BeginPlay` at 233 lines because it started
nine test probes. The scan was a script that counted braces, not a parser, so the function
numbers are approximate.

The deeper findings were already in the code wiki's known-issues page, which I had written from
the source at commit `bfe9e16`. I used it as the input rather than searching again.

## The register

`docs/tech-debt-register.md` holds 20 items, each with a category, files, effort (S, M, L), impact
and a priority score of impact divided by effort. The highest score went to the debug features
that exist in every build (3.0), then a group at 2.0: repeated saves, missing null guards, a gap
in the level-up pause, development plugins that go into the package, and the frame rate that was
only ever measured with the weapons off. Large items, such as the gameplay numbers still hardcoded
in C++ and the `Iron` fields that hold wood, are marked as probably to be accepted, because
fixing them near the deadline is a bigger risk than leaving them.

## When to update the code wiki

I advised against updating the code wiki before the refactor: it was written and checked against
the current source, and a refactor moves files and lines, so most of its `file:line` references
would go stale and be written twice. The known-issues page is the exception, because it is the
task list. The wiki is updated once at the end.

## Not done

Nothing was refactored in this entry. The register is a list of decisions, not proof the debt
is real: each item was read from the code, none was reproduced as a failure.
