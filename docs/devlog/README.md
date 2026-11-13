# Director's Cut — Dev Log

A solo entry to the 2026 Epic AI MegaJam, built in Unreal Engine 5.8 (C++) with
Claude Code driving the editor through the official Unreal MCP plugin.

**Status:** this log is updated every day of the jam. Days 1 and 2 are written;
Days 3 and 4 will be added as they happen.

| Entry | What it covers |
|---|---|
| [Day 1 — 12 Nov](day-1-2026-11-12.md) | Reading the theme, planning, the first playable loop (stories 001–003) |
| [Day 2 — 13 Nov](day-2-2026-11-13.md) | Progression, camp, UI and save (stories 004–007); realising the theme was missing from the game; the redesign; the packaging problems |

## The game in one paragraph

A fantasy top-down horde-survival game with extraction. Before each run you slot
film cards that decide how hard the run is. During the run you auto-attack,
level up, loot crates and extract. As of Day 2 the level-up is the heart of the
theme: **you are offered three options, you cut one, and you keep the other
two.** Three copies of the same option merge into a two-star version, nine into
a three-star one.

## How I direct the AI

The jam theme says "you're directing the AI agents too". This is the actual
process, not a description of what I intended.

1. **I decide what to build; Claude drafts.** Every idea starts as a question or
   a one-line request from me. Claude proposes options, I choose or reject.
2. **Plan before code.** A requirements plan, then a master PRD with
   Must / Should / Could tiers and cut rules, then one short design doc per
   system. The master PRD is the scope gate: nothing is built that is not in it.
3. **Work in stories.** Each story has acceptance criteria. Claude implements it,
   compiles, runs the game in the editor, and keeps screenshots as evidence.
4. **Verify by running, not by reading.** Compiling is not evidence. The
   evidence is a screenshot or a log line from the running game (see the lessons
   below for why).
5. **I veto on design grounds.** Several of Claude's best-sounding ideas were
   thrown out by me because they failed as game design, not because they were
   hard to build. Day 2 lists them.

## What I decided, and what Claude did

| Mine | Claude's |
|---|---|
| The genre pairing, the scope, the cut rules | Drafting the requirements plan and the design docs |
| Rejecting a design that did not fit the theme (Day 2) | Reviewing its own brief against the code and finding six problems |
| Choosing "cut one, keep two" among the options | Writing the C++, the tests and the in-game verification harness |
| Deciding to remove a project setting so packaging could work | Finding that setting, and finding that the package was missing a map and the data tables |
| Deciding the first goal is a finished, submittable game | Keeping the process notes this log is built from |

## By the numbers

Counted from the repository on 13 November.

- 14 commits on `main`, two of them through pull requests.
- 73 C++ source files, about 6,800 lines (`.h` and `.cpp` under `Source/`).
- 7 stories shipped for the first version of the loop, plus the Day 2 redesign.
- 12 design documents (one master PRD and 11 system docs).
- 4 automated tests (the star-merge and cut rules). The earlier stories were
  verified by playing and by screenshots; automated tests were waived at the
  project's minimal QA level. That is a gap, and I say so in Day 2.
- Performance check: 152 enemies on screen averaged **132 FPS**, with the single
  worst frame at 50 FPS (target was 60 or more). I accepted that as advisory.

## Lessons so far

Each of these happened in this project; the day entries have the details.

1. **A green build proves nothing.** The hero could not die because a handler
   was missing one macro. A tint did nothing because the engine's default
   material has no such parameter. A building spawned as the wrong type because
   a field was set after `BeginPlay`. All three compiled. All three were found
   only by playing.
2. **The worst bugs were silent.** A missing tuning table makes the game fall
   back to built-in defaults without an error, so its absence would never show.
   The first working package left out the run map and said nothing until the
   game tried to load it. The editor setting that broke packaging had no note
   explaining why it was there. When a system is designed to fail quietly,
   check it on purpose.
3. **AI-written documentation can be confidently wrong.** An early code wiki
   claimed one thing about a material; the source says the opposite. I now ask
   for documentation that lists only what was checked against the code.
4. **Ask the agent to attack its own plan, against the code.** Reviewing the
   Day 2 design against the actual source found that the central feature had
   nothing to attach to. That review was worth more than another round of ideas.
5. **The human's job is taste.** The agent produced plenty of plausible
   mechanics. Choosing which one is fun, and which only sounds clever, was the
   part it could not do for me.

## Tools

For the submission's "Tools you used" field:

- **Unreal Engine 5.8.4**, C++ gameplay code. Rider to read the code.
- **Claude Code** (desktop app) with Claude Opus 5.5, Claude Sonnet 5.5 and
  Claude Fable 5.2, connected to the editor through the **Unreal MCP** plugin.
- **Claude Code Game Studios**, an open-source template of subagents and skills
  (49 agents, 74 skills) by Donchitos. I did not write it; I configured and used
  it. <https://github.com/Donchitos/Claude-Code-Game-Studios>
- Git and GitHub for version control.

Not used so far: any AI 3D, texture or audio generator. This list will change on
Day 3 if that changes.

## Before the jam (declaration)

The rules allow preparing tools and ask us to declare what was made earlier.

- **Allowed preparation, in the repository:** the Claude Code Game Studios
  agents and skills and their configuration, the Unreal MCP setup and curated
  Unreal Engine 5.8 reference notes. The first commit (`44589d2`) contains only
  this: no gameplay, levels, art or templates. A jam schedule (`PLAN.md`) was
  also written before the jam.
- **A throwaway warm-up project** existed before the jam to test the tool chain.
  It is not in the repository and nothing from it was copied into the game.
- The game project (`Game_01`) was created after the jam started.

## Assets and credits

All visuals so far are Unreal Engine primitive shapes (cubes and cylinders) with
no external assets and no audio. The open-source game catalogue
(github.com/Trilarion/opensourcegames) was read for research and not used as a
code base (Day 2 explains why). This section will list anything added later.
