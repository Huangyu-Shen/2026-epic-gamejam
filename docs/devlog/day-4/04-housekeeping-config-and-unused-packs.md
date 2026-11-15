# Day 4 (15 Nov) · Config that pointed at the wrong project, and 1.1 GB of unused packs

[Dev log index](../README.md) · [← previous](03-the-hero-and-the-enemies-faced-the-wrong-way.md) · [next →](05-tech-debt-scan-and-register.md)

## The package command was missing because it was wrong

Story 013 says the package command is written in `production/session-state/active.md`. It was
not. Searching for it showed why: `project.yaml` still pointed its four commands (build, test,
run, smoke) at `Game_Jam_01/`, the warm-up project that is not in the repository, and the build
command had no stage, pak or archive options, so it could not have made a runnable package. The
real command was only in `docs/wiki/build-and-run.md` section 8, recorded from the earlier
packages.

I changed the four commands to `Game_01 5.8/Game_01.uproject` and put the full package command
in `build`, with a comment to close the editor first and to leave the `.pdb` out of a zip.
`project.yaml` is a file I may only change with the author's approval, which I had. I did not run
the new command. `engine.language` still says `"Blueprint"` for a C++ project; that is on the
debt register.

## What the engine association GUID is

The author had changed `EngineAssociation` in `Game_01.uproject` from a GUID to `"5.8"` on
purpose, and I first took it for an accident of the editor. The GUID is a number the engine
registers on one computer for an engine that was not installed through the Epic Launcher; it
lives in that computer's registry and means nothing on another one. `"5.8"` resolves through the
Launcher on any machine that has it. For a public repository `"5.8"` is the better value, so it
was committed.

## Deleting the unused packs

Six imported packs were never used (two sci-fi packs, Battle Wizard, Four Evil Dragons, and two
music and sound packs), about 1.1 GB, and the project's `Content` folder was down to about 464 MB
without them. Before deleting I checked that none of them was in git, and that nothing in the
source, the config, the maps, the data tables or the packs still in use referred to them.

The delete command was refused by the permission system. The author said yes in chat and it was
refused again. I did not try another way around it; the author deleted the folders by hand.
That matches Day 3's lesson about a blocked agent: a refused command is a boundary, not a bug to
route around. The docs then said "deleted on 15 Nov" where they had said "imported, not used".
