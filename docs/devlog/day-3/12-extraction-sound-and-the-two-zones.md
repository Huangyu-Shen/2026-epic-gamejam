# Day 3 (14 Nov) · Sound, free assets, and a progress bar that never showed

[Dev log index](../README.md) · [← previous](11-new-level-up-rule-and-ui.md) · [next →](13-story-012-boss-and-the-void.md)

## Audio from free packs

The game had no sound files. The author looked in the Unreal Fab marketplace for free
packs and imported four. They are listed with their licences in the
[assets table](../README.md). One needs credit (CC-BY, "Path of Adventure" by
Kounine) and the credit is in that table.

- An audio subsystem plays 11 named sounds (attack, hit, death, pickup, level up, cut,
  star-up, extraction, extraction start, player death, camp upgrade) and the music.
  Which file plays for each is a setting, not code. A cap keeps 150 enemies from
  making a wall of noise (16 crowd sounds a second).
- The camp upgrade sound was one of the author's complaints. It now plays.

**A trap I hit:** a sound chosen by a soft reference is not put into the package
unless something refers to it or its folder is listed to always be cooked. I listed the
cue folders in the project settings and checked the package log: 9 of 9 sounds were
assigned and the music start was logged.

## The weapons were invisible in a package

Attacks were drawn with the engine's debug lines, which a shipping package does not
show. I replaced them with short-lived in-world shapes: a beam for Blade, Bolt and
Chain, a ring of cubes for Nova, a bullet and a ball for Gun and Orbit. The engine's
default materials ignore a colour parameter, so everything came out grey, and one
default made the beam invisible. A small project material, `M_FlatColor`, fixed both.
A first Nova ring also looked like spokes because the pieces were turned a quarter turn
too far.

## The extraction point

- The zone is now a bright green disc with a tall beam. A yellow disc fills as you
  stay inside. A toast says "EXTRACTING - stay inside the green circle", with a sound
  at the start and another on success.
- **The bug:** the progress bar on the HUD never showed. The level has two extraction
  zones and the HUD only watched the first one. If you stood in the second, nothing
  moved. It now takes the largest progress of all zones. This probably also explains
  why the author first heard no extraction sound: they were most likely in the second
  zone. I do not know that for sure. The author later said extraction was fine.

## Two bugs the packaged run showed that the editor did not

- A packaged run logged "Array has changed during ranged-for iteration" in Bolt and
  Nova. Killing an enemy removes it from the list being walked. The weapons now collect
  their targets first and damage them afterwards. The same pattern was used for the new
  Orbit.
- One new source file had its own header included after another header, which the build
  tool rejects.

## Writing the data tables was blocked, again

Making the material and changing Blade's range both needed the editor's Python
commandlet. The permission checker refused each time (see
[entry 07](07-when-the-agent-was-blocked.md)). I did not retry or look for another
route. I did the rest of the work, asked the author in chat, and ran each command only
after they said "Yes, run it". The package confirmed that the range of 380 was read.

## A package I measured

A Development package without the debug-symbol file was about 706 MB, under the 1 GB
limit. A packaged run with 151 enemies averaged 130 frames a second, worst 96, with
weapons switched off. The directory of a later package measured 1.1 GB; I did not check
what was in it, and the debug-symbol file is the likely reason.
