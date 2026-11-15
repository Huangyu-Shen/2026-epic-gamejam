# Director's Cut — Dev Log

A solo entry to the 2026 Epic AI MegaJam, built in Unreal Engine 5.8 (C++) with
Claude Code driving the editor through the official Unreal MCP plugin.

**Status:** this log is updated every day of the jam. Days 1 and 2 are written,
Day 3 is in progress (stories 008, 009 and 010 closed, 010 with 8 criteria deferred to a human playtest), and Day 4 has a plan
on record.

**Public address of this log** (the repository is public):
<https://github.com/Huangyu-Shen/2026-epic-gamejam/blob/main/docs/devlog/README.md>

The log is split into one folder per day. Inside each folder the entries are
numbered in the order things happened, and each one covers a single topic, so you
can read a day from top to bottom or open just the part you care about.

### Day 1 — 12 Nov: from a theme to a playable loop

| Entry | What it covers |
|---|---|
| [01 Reading the theme](day-1/01-reading-the-theme.md) | What "Director's Cut" can mean in a game, and the first design |
| [02 Planning before code](day-1/02-planning-before-code.md) | The requirements plan, the master PRD with its cut rules, one design doc per system |
| [03 The first stories and a frame-rate probe](day-1/03-first-stories-and-performance.md) | Stories 001–003 and the 150-enemy performance measurement |
| [04 What went wrong, and what I did not do](day-1/04-what-went-wrong-and-what-was-skipped.md) | The hero that could not die, and the playtest I skipped |

### Day 2 — 13 Nov: finishing the loop, then admitting the theme was missing

| Entry | What it covers |
|---|---|
| [01 The first loop](day-2/01-finishing-the-first-loop.md) | Stories 004–007: progression, pre-run editing, camp, flow, save and UI |
| [02 The moment I stopped](day-2/02-the-moment-i-stopped.md) | Why the game did not fit the theme, and the open-source research |
| [03 Seven ideas, and the agent review](day-2/03-seven-ideas-and-the-agent-review.md) | Which ideas were dropped and by whom; asking the agent to attack its own plan |
| [04 The redesign and how it was checked](day-2/04-the-redesign-and-how-it-was-checked.md) | "Cut one, keep two", star merging, tests and an in-game probe |
| [05 Packaging in five attempts](day-2/05-packaging-in-five-attempts.md) | Why the first package would not build, then would not run |
| [06 Making the repository English](day-2/06-english-sweep-and-gitignore.md) | Translating the project with parallel agents; auditing `.gitignore` |
| [07 Honest status](day-2/07-honest-status.md) | What was built, decided, unchecked and missing at the end of the day |

### Day 3 — 14 Nov: lost film reels, a playtest, and the boss (in progress)

| Entry | What it covers |
|---|---|
| [01 Reviewing the new stories](day-3/01-reviewing-the-new-stories.md) | The holes found in stories 008–012 before building them, and the order for the day |
| [02 Story 008: lost film reels](day-3/02-story-008-lost-film-reels.md) | Relics become unlock keys; data tables; probes in the editor and the package |
| [03 A bug only my real save showed](day-3/03-a-bug-only-the-real-save-showed.md) | A save-version default that silently skipped the migration |
| [04 Keeping the docs honest](day-3/04-keeping-the-docs-honest.md) | Updating the wiki and design docs, and checking the sub-agent's work |
| [05 Checking the code wiki against the source](day-3/05-checking-the-code-wiki-against-the-source.md) | Four agents re-verify nine pages; the old errors and the code defects they found |
| [06 Story 009: unlockable options](day-3/06-story-009-unlockable-options.md) | Haste, Magnet and Chain; NEW! in the next run; two problems the probes showed |
| [07 When the agent was blocked](day-3/07-when-the-agent-was-blocked.md) | A command refused by the permission checker, and what the agent did |
| [08 Story 010: the first number pass](day-3/08-story-010-first-number-pass.md) | A bot measures the pace; five sets tried; four numbers changed |
| [09 Closing the stories](day-3/09-closing-the-stories.md) | How stories 008–010 were closed, with the gaps written down |
| [10 The first playtest of the package](day-3/10-first-playtest-of-the-package.md) | What the author found, and the level-up rule that was backwards |
| [11 The new level-up rule and UI](day-3/11-new-level-up-rule-and-ui.md) | Take one of three with an optional cut; 7 to 13 options; the cleaner UI |
| [12 Sound, free assets, and a bar that never showed](day-3/12-extraction-sound-and-the-two-zones.md) | Audio from free packs, visible weapon effects, the two-zone extraction bug |
| [13 The boss takes what you cut, and the void](day-3/13-story-012-boss-and-the-void.md) | Story 012 built after its cut-off; the floor edge fix; what was left out of git |
| [14 Status so far](day-3/14-status-so-far.md) | What is done, not done and not checked |

### Day 4 — 15 Nov: package and submit (planned)

| Entry | What it covers |
|---|---|
| [01 Plan](day-4/01-plan.md) | The submission checklist, written in advance |

## The game in one paragraph

A fantasy top-down horde-survival game with extraction. Before each run you slot
film cards that decide how hard the run is. During the run you auto-attack,
level up, loot crates and extract. As of Day 2 the level-up is the heart of the
theme: **you are offered three options, you cut one, and you keep the other
two.** Three copies of the same option merge into a two-star version, nine into
a three-star one. As of Day 3, relics are **lost film reels**: carry one out
alive and a new option joins the pool for every later run.

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

Counted from the repository on 14 November, part-way through Day 3. Work from
Day 3 is not committed yet, so these are working-tree counts.

- 18 commits on `main`, three of them through pull requests; the work of stories 009
  and 010 and the wiki re-check is not committed yet.
- 72 C++ source files, about 7,140 lines (`.h` and `.cpp` under `Source/`, not
  counting the 302 lines of tests).
- 9 stories shipped: the 7 for the first version of the loop, plus stories 008 and
  009 on Day 3, and a first pass at story 010; the Day 2 redesign (cut one, keep
  two; star merge) was built without a story file.
- 13 story files in all, and 12 design documents (one master PRD and 11 system docs).
- 12 automated tests (the star-merge and cut rules, the lost-reel unlock rules, and
  the locked-option rules).
  The earlier stories were verified by playing and by screenshots; automated tests
  were waived at the project's minimal QA level. That is a gap, and I say so in
  Day 2.
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
6. **Test against data the old code wrote.** A save-version default that looked
   right passed every test written by the new code, and silently skipped the
   migration of my real save. Only loading a copy of the real file showed it. See
   [Day 3, entry 03](day-3/03-a-bug-only-the-real-save-showed.md).
7. **Delegate the work, not the checking.** Eight sub-agents translated the
   project in parallel and a sub-agent updated the wiki; both reported success, and
   in both cases the checks that mattered (links, leftovers, unverified claims)
   were run afterwards by me and the main agent. See
   [Day 2, entry 06](day-2/06-english-sweep-and-gitignore.md) and
   [Day 3, entry 04](day-3/04-keeping-the-docs-honest.md) and
   [entry 05](day-3/05-checking-the-code-wiki-against-the-source.md).
8. **Measure the pace with a bot before arguing about feel.** A bot that only walks in
   circles showed that a three-to-ten-minute run was impossible on the numbers the
   game had. It does not tell me whether the game is fun, but it found a problem
   that no earlier check had. See
   [Day 3, entry 08](day-3/08-story-010-first-number-pass.md).
9. **A blocked agent should stop and say so.** See
   [Day 3, entry 07](day-3/07-when-the-agent-was-blocked.md).

## Tools

For the submission's "Tools you used" field:

- **Unreal Engine 5.8.4**, C++ gameplay code. Rider to read the code.
- **Claude Code** (desktop app) with Claude Opus 5.5, Claude Sonnet 5.5 and
  Claude Fable 5.2, connected to the editor through the **Unreal MCP** plugin.
- **Claude Code Game Studios**, an open-source template of subagents and skills
  (49 agents, 74 skills) by Donchitos. I did not write it; I configured and used
  it. <https://github.com/Donchitos/Claude-Code-Game-Studios>
- Sub-agents inside Claude Code for parallel translation and documentation work.
- Unreal's Python scripting plugin, run from the command line, to create data
  tables without opening the editor.
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

Visuals are Unreal Engine primitive shapes (cubes, cylinders) with one flat-colour
material made for this project. The open-source game catalogue
(github.com/Trilarion/opensourcegames) was read for research and not used as a
code base (Day 2 explains why).

Audio, all from Fab and imported on 14 November. None was made with AI according to
the listings. Licences are as shown on each Fab page; I read the pages, not the full
licence texts, and I am not giving legal advice.

| Pack | Author | Licence on its Fab page | Used for |
|---|---|---|---|
| Path of Adventure - Free Music Pack for JRPG | Kounine | Creative Commons CC-BY (credit required) | Run music (Battle loop) and camp music (Village loop) |
| 50 Free Game Sounds Pack | PlaceHolder Inc | Fab standard licence, personal tier (free) | The nine sound effects |
| Atmospheric Worlds - FREE Game Music Pack | Marika Schanz | Fab standard licence; the page says free for commercial and non-commercial use | Imported, not used so far |
| Small Sound Kit | Sound Works 12 | Fab standard licence | Imported, not used so far |

**Credit (CC-BY):** music by Kounine, "Path of Adventure - Free Music Pack for JRPG",
Creative Commons CC-BY, via Fab.
