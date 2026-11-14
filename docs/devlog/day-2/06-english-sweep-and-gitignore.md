# Day 2 (13 Nov) · Making the repository English, and auditing what Git should ignore

[Dev log index](../README.md) · [← previous](05-packaging-in-five-attempts.md) · [next →](07-honest-status.md)

Late on Day 2, with the first commits pushed, I asked for two housekeeping jobs.
Neither adds anything to the game. Both matter for a jury that will open the
repository, so they are logged here.

## Everything in the repository in English

The jam rules want English text, but much of the project's writing was in Chinese:
the schedule, the requirements plan, the master PRD, all 11 system design
documents, the code wiki and some notes. I asked Claude to translate every piece
of Chinese in the project while it committed the day's work.

How it was done:

1. **Find the scope first.** A scan of every tracked file for Chinese and
   full-width characters found 31 files and about 2,200 lines. Two of them had
   Chinese file names.
2. **Hand the work out in parallel.** The files were split into eight groups
   with no overlap, and eight sub-agents (running Claude Sonnet 5.5) each
   translated their group in place. The brief told them to keep Markdown
   structure, tables, code, paths and numbers exactly, to translate only the
   human-readable text, to keep each file's line endings, and to prove the file
   was clean with a scan before reporting.
3. **Check their work.** After the agents reported, I re-ran the scan over every
   tracked and new file, over file names, over the binary assets and over every
   commit message. Result: nothing left.
4. **Fix what translation breaks.** Translated headings change their link
   anchors. A link checker over all Markdown found four links broken that way
   (two in the schedule's table of contents, two in the wiki), and they were
   repaired. The same checker also showed that most "missing file" hits were
   false alarms from `%20` in paths, so it was fixed to decode them first.
5. **Look for artefacts.** Empty brackets and half cells left behind by dropped
   glosses: none.

Some files needed special handling. Three small files (the epic page and two
story files) had a stray Chinese line each that was outside the agents' groups, so
Claude translated those directly. The two Chinese-named notes in `docs/` were
renamed to English names.

Several agents reported that the write-guard hook had warned about a sub-agent
writing under `design/`. The writes went through, and the warning is recorded here
because it is exactly the kind of rule I want to know was seen.

## What should Git ignore?

A review of `.gitignore` against what was actually on disk found two real leaks
and a few missing rules:

- The Unreal template ignores `Build/*` only at the repository root, but the
  project lives in a subfolder, so the editor's `Build/Windows/FileOpenOrder/*.log`
  files showed up as untracked. Fixed by making the rule match at any depth.
- The same was true of the project's own session logs folder.
- `project.local.yaml` and `CLAUDE.local.md` are listed as local-only files in
  `.worktreeinclude` but were not ignored, so they could be committed by mistake.
  Added.
- Debug symbols, `*.log`, Rider's `.idea/`, Python caches and `Thumbs.db` were
  added too.

I checked that no file already tracked became ignored, that nothing generated is
tracked, and that the largest tracked files are the screenshots in LFS.

## What I would tell another team

Translating a repository is a good job for parallel agents, but only if the
checking is not delegated. The agents all reported success; the check that found
the broken links was mine.
