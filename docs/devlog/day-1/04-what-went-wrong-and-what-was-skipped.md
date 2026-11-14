# Day 1 (12 Nov) · What went wrong, and what I did not do

[Dev log index](../README.md) · [← previous](03-first-stories-and-performance.md)

## What went wrong

**The hero could not die.** Everything compiled and ran, but at zero health
nothing happened. The death handler was missing one `UFUNCTION()` macro, so the
event binding silently never connected. Claude found it only by playing until
health reached zero. The fix was one line; the lesson was that a successful build
says nothing about whether a feature works.

**A rename left stale paths.** The project folder was renamed from `Game_01` to
`Game_01 5.8` when the first stories were committed. Paths in the README and
`.gitignore` kept pointing at the old name, and Day 2 started with cleaning them
up.

## What I did not do

- **No playtest with another person.** The plan's exit criterion for Day 1 was
  to test "keep it or use it" with one or two people. I did not. I put it off and
  kept building, which is exactly the risk the cut rules were meant to prevent.
- No art and no audio, on purpose, until the loop works.

## Evidence

- [Story 001](../../../production/qa/evidence/story-001-loot-bag-heat/) ·
  [Story 002](../../../production/qa/evidence/story-002-extraction-death/) ·
  [Story 003](../../../production/qa/evidence/story-003-combat-enemies-map/)
