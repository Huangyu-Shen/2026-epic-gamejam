# Day 2 (13 Nov) · Packaging, in five attempts

[Dev log index](../README.md) · [← previous](04-the-redesign-and-how-it-was-checked.md) · [next →](06-english-sweep-and-gitignore.md)

## Packaging, in five attempts

The jam's own tips say to package early. I had not packaged once, so I did it
on Day 2. It took five tries, and the last two failures were silent ones.

| # | What happened | Cause |
|---|---|---|
| 1 | The cook crashed with "Failed to delete oplog on the ZenServer". | A setting in the project config, `AutoLaunch=False` for the Zen storage server, had been added during the camp story as an editor fix. Nothing recorded why. |
| 2 | Overriding it on the command line let the cook finish. The staging step then failed to launch the server. | Apparently the staging step reads the config file itself and does not see the override. |
| 3 | Passing the override to the pack steps as well changed nothing. | Same. |
| 4 | With my approval, Claude removed the config line. Packaging succeeded. The packaged game **could not load the run map**. | Only the start map had been cooked. |
| 5 | After adding both maps to the cook list it ran. | I also added the data folder as a precaution: the tuning tables are loaded through soft pointers in a settings file, which the cooker does not follow by itself, and the game falls back to built-in defaults without an error if a table is missing. I did not test a package without that line, so I cannot say it was needed. |

The packaged game now loads both maps and runs the level-up probe. The package
folder is 1.1 GB, but 356 MB of that is a debug-symbol file that is not part of
what I will submit, which leaves about 0.75 GB against a 1 GB limit.
