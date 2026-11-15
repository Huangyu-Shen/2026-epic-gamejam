# Day 4 (15 Nov) · Preparing the submission materials

[Dev log index](../README.md) · [← previous](01-plan.md)

Written on the afternoon of 15 Nov, before the final package. Nothing here is a claim that the
final build works; it is what the paperwork said, what the code and the files said, and what
is still open.

## The stories' checkboxes were behind the code

Story 011 (art, audio, feel) had all 20 acceptance criteria unticked even though most of its
code had been merged in pull requests 4 to 11. I went through it against the code, the config
and the retained evidence, and ticked 8. The rest stay open, each with the reason beside it.

What I read and what it showed:

- **Weapon visuals** are meshes (`AWeaponEffect`). `DrawDebug` is left only in the stand-in
  character, which the story says does not count.
- **Audio** has one entry point and 11 sounds assigned in the project config. The sound and
  music folders are in the always-cook list, because a soft pointer alone is skipped by the
  cooker without an error (this already happened to the run map). That is a configuration
  check, not a check in a package by ear.
- **Hit feedback** exists: a sound and a 0.1 second scale pop to 1.2 times. The white colour
  flash changes the hidden cube, so it shows on cube enemies and not on modelled ones. The
  author remembered feedback when playing; the code agrees.
- **Star-ups** scale damage and reach, but I found nothing that changes how the weapon looks,
  and no star-up screenshot. That criterion stays open.
- **Frame rate** is 130 FPS average with 151 enemies in a package, but that probe had the
  weapons switched off and was run before most of the art. **Package size** was 679 MB without
  the debug file at the same time. Both are old numbers and both are re-measured in story 013.
- **WeLoveIndies audio** was never used: the sounds and music are Fab packs, so the licence
  question in the story does not apply.

Story 012 (the Cut Knight boss) was already closed. I corrected its notes: criterion 4 is
marked obsolete (a cut is now refused when permanent cuts run out, so skipped cuts no longer
exist), the evidence status now points to the folder, and the "not yet done" line now says what
really is not done: a build that spent all its cuts has not been checked against the boss.

## Numbers recounted

The "By the numbers" section of the log now says 27 commits on `main`, 89 C++ files and about
10,600 lines including 303 lines of tests, and 12 automated tests. Earlier figures (19 commits,
82 files, 9,160 lines) were from before pull requests 5 to 11. I counted them with `git
rev-list`, `find` and `wc`, not from memory.

## What is still open for story 013

- The final package, built from a clean working tree at a named commit. The tree is not clean
  today: the imported Fab packs are untracked on purpose, and a stray image
  (`ZenUV_SciFi_Project.png`) is untracked too.
- Both maps checked in the package, twice; a start from a deleted save and from an existing
  one; the zip unpacked into a new folder.
- Packaged-build checks that only the author can do: sound and music by ear, no leftover debug
  text, a frame-rate run with weapons on.
- The gameplay video (30 to 60 seconds: a cut, a star-up, an extraction).
- The imported packs that were not used (the two sci-fi packs, Battle Wizard, Four Evil Dragons,
  two music and sound packs, about 1.1 GB) were deleted from `Content/` on the author's
  decision, after I checked that nothing referenced them. `Content/` is now about 464 MB.
- No licence has been chosen for the repository.

## What I did not do

I did not build a package, run the game, or record anything today. Claude does not upload or
submit; that is mine to do before 16 Nov, 01:59 UK time.
