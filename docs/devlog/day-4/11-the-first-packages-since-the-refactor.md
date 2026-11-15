# Day 4 (15 Nov) · The first packages since the refactor

[Dev log index](../README.md) · [← previous](10-closing-the-register.md)

After the refactor I went back to stories 011 and 013 and built Windows Development packages with the command
now recorded in `project.yaml`. They took one to two minutes each once the cook was warm.

## The package found a regression I had made

Earlier that day I limited three development plugins (`ModelContextProtocol`, `AllToolsets`, `Terminal`) to
the Editor target, so they would not go into the package. Every test and probe still passed. The first
package started with an ensure in its log: "Failed to load class GameFeatureData". Those plugins had been
pulling in the GameFeatures plugin, and the project's asset manager settings still scan for game feature
data. I checked the old package from before the change: it mounted GameFeatures and had no ensure.

The obvious fix, deleting the `GameFeatureData` line from `DefaultGame.ini`, made the cook fail: the editor
side still wants that rule. Enabling the GameFeatures plugin in `Game_01.uproject` fixed it: the next package
has no ensure, no error, and the three dev plugins are not in it. None of the editor tests could have shown
this; only running a package did.

## What was measured

All on the packaged Development build, windowed 1920x1080:

- **Size.** 703 MB without `Game_01.pdb` (355 MB); 1,058 MB with it. The limit is 1 GB without the pdb.
- **Frame rate.** A new `-PerfWeapons` flag on the performance probe gives the hero all six weapons at two
  stars and keeps 150 enemies alive by topping the horde up every frame. Average 108 FPS, worst single frame
  58.7. With the weapons off, average 88 and worst frame 24.6, on the first run after a start, so it includes
  warm-up. The worst frames are below 60 and I treat them as advisory, as on Day 3.
- **Saves.** On the run map a new slot says "No camp save found" and the next start says "Loaded camp save",
  twice in a row, each settling an extraction. The camp map loads as the default map.
- **Screenshots.** Camp, combat, the result screen, and a star-up pair taken with a new `-ShowcaseCopies=N`
  flag: the same scene with the Gun and Orbit at 1, 2 and 3 stars, where the orbit goes from two small balls
  to six large ones. They are in `production/qa/evidence/story-011-art-audio-feel/packaged-013c/`.

## What this closed

Story 011 now has all 16 criteria checked and is marked complete, with the worst-frame figures and the fact
that a star-up changes size and count but not colour written down as deviations. Story 013 is in progress.

## What is still the author's, or not done

The hand walk-through (camp, run, extract, camp, twice), unpacking the zip into a new folder, the gameplay
video, the final package from a clean tree at a named commit, and the submission itself. Nothing was
uploaded. The package used for these checks was built from a tree with uncommitted probe flags, so it is not
the final one.
