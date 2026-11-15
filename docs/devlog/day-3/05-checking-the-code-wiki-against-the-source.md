# Day 3 (14 Nov) · Checking the whole code wiki against the source

[Dev log index](../README.md) · [← previous](04-keeping-the-docs-honest.md) · [next →](06-story-009-unlockable-options.md)

The code wiki (nine pages in `docs/wiki/`) had been written for the first seven
stories and patched for story 008. I wanted to know how much of it was still true,
so I ran a code-wiki skill that rewrites a wiki and holds it to one rule: every
sentence is checked against the source, and the old pages are treated as clues, not
facts.

## How it was split

The pages were divided between four sub-agents so that no two could edit the same
file. Each agent had to read every non-test source file it needed, check its own
pages line by line, and report the corrections as "the old page said X; the code
actually does Y", plus any defect it noticed in the code itself. Meanwhile I did the
checks that did not need reading: the test count, the line counts, the engine version
from the engine's own build file, and the skill's link checker.

One agent was stopped half-way by a usage limit on the account. I said "try again"
and resumed the same agent from where it had stopped, rather than starting a new
one, and gave it the extra findings the other three had reported so far.

## What was wrong in the old pages

The wiki had looked careful, and it still had real errors. A few of the clearest:

| The page said | The code does |
|---|---|
| A level-up is "pick one of three" (`PickOffer`) | There is no `PickOffer`. The player cuts one and keeps two (`CutOffer`), and with fewer than three options left the player simply gets them. |
| Each option stacks up to five times | The cap is at least nine, because three-star needs nine copies. |
| The hero calls `ReportExtracted` | The extraction zone's own tick does. The hero only reports death. |
| The result screen appears when the run-ended event fires | The HUD never listens to that event; it polls and reads the result. |
| An extraction saves once | It can save several times: once for the Iron, once per newly unlocked option, and once explicitly. |
| Reel and container rows are looked up by name | They are matched by a field inside the row; the row name is only a label. |
| Packaging "has not been made to work" | It works (Development build), with the command and two settings that make it work. |

The line counts, the dependency graph (missing real edges, and a real cycle) and the
list of data tables (there is also a struct of star settings that is not a table)
were wrong too. The pages that explain things from the source now carry the numbers
that were counted, not remembered.

## What the agents found in the code itself

The most useful by-product. Each of these was verified in the source before it was
written into the known-issues page:

- A container table with rows but none for one container type does not fall back to
  the built-in default for that type, so that container can never be searched. A
  mistyped field in the table is silent.
- The save only tests "version below 2", so a save written by a newer build would be
  read as version 2 and then overwritten.
- Enums are stored by name, so renaming an entry silently changes old saves.
- Migrating an old save drops relics that map to no option, and once the migrated
  save is written there is no way back.
- A field that says it is shown on the HUD (a reel's display name) is never read.
- A dash in progress keeps moving the hero after death, because that branch does not
  check whether the hero can act.
- The debug key that kills the hero is bound in every build.
- A pile of comments still describe features that were removed.

None of these was fixed in the same pass. The skill's rule is that the wiki reports,
and the user decides what to fix.

## What was checked, and what was not

- The link checker found 270 links and no problems; no Chinese characters remain.
- All nine automated tests passed on a fresh run (this was before story 009).
- I spot-checked four of the agents' claims against the code by hand (that one
  function has no caller, the stack cap, the debug key binding, and the boss's
  experience multiplier). All four were right.
- Not checked: the numbers inside the binary data tables (the agents could only
  read their row names), the Mermaid diagrams (never rendered), and whether the
  editor and package compile at this moment.
- One agent ran a read-only Git command although the brief said not to. It changed
  nothing, and I note it because the brief was explicit.
