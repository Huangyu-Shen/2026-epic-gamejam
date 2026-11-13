# Epic AI MegaJam 2026 — Solo UE Track Plan

- **AI tool**: Claude Code (connected to the editor through the official Unreal MCP)
- **Dev machine**: Windows PC (the Mac is only used to initialize the project), synced through GitHub
- **Current phase**: **Phase 0** (today is 11/3; the jam starts 11/12 05:00 UK time)

**Contents**: [Key Dates](#key-dates) · [Phase Overview](#phase-overview) · [Quick Rules Reference](#quick-rules-reference) · [Phase 0](#phase-0--pre-contest-preparation-113---1111) · [Phase 1](#phase-1--day-1--1112-thursday) · [Phase 2](#phase-2--day-2--1113) · [Phase 3](#phase-3--day-3--1114) · [Phase 4](#phase-4--day-4--1115) · [Phase 5](#phase-5--submission--1115-evening---1116-0159) · [Risks](#risks)

## Key Dates

All times are UK time.

| Time | Event |
|---|---|
| 11/11 19:00 | Opening livestream, theme announced |
| 11/12 (Thursday) 05:00 | Jam starts; tag this repo `jam-start` and create the UE project in the repo root |
| 11/14 evening | Feature freeze |
| 11/15 | Package, video, log, upload to itch.io |
| 11/16 01:59 | **Submission deadline**; late entries are disqualified |

## Phase Overview

| Phase | Time | Goal | Completion criteria |
|---|---|---|---|
| 0 | 11/3 - 11/11 | Prepare the toolchain only | Whole pipeline works, including packaging |
| 1 | 11/12 05:00 - night of 11/12 | Settle the gameplay + greybox prototype | Core loop is playable |
| 2 | 11/13 | Gameplay expansion + level + assets | Level can be completed |
| 3 | 11/14 | Art/audio/UI polish, freeze features in the evening | Content complete |
| 4 | 11/15 | Fix bugs, package, video, log, upload | Build and materials ready |
| 5 | Evening of 11/15 - 11/16 01:59 | Developer submits in person | Submission succeeds (aim to leave several hours of margin) |

## Quick Rules Reference

From the official rules; the original text is authoritative.

**Time and scope**
- Contest period 11/12 05:00 -> 11/16 01:59; late entries are disqualified.
- Before the jam you may only prepare AI tools, agent config, MCP setup, prompt libraries and scripts. You may **not** pre-build gameplay, levels, art or templates; old Unity/Godot code cannot be carried over.
- Gameplay must be custom, not an extension of an official tutorial; it must reflect the theme (announced 11/11 19:00).

**Content and materials**
- All English (in-game text, audio, video, sign-up info); 13+ content; no trademarks/logos.
- Video 30-60 seconds, at least 1920x1080.
- External assets (Fab, etc.) are listed at submission and are not scored; AI-generated content does not count as external assets, but must not infringe.
- Build size limit 1 GB.

**Submission**
- The submission must be made by the developer in person; an agent cannot submit on their behalf.

**Scoring and goals**
- Seven categories, 5 points each: Innovation Ambition, Theme, Fun, Gameplay, Visuals, Level Design, Audio (the Fun score is doubled when tied).
- Target awards: Best Student Project or Best Overall (UE), pick one, plus Solo Driver.

---

## Phase 0 — Pre-Contest Preparation (11/3 - 11/11)

Principle: prepare tools only, make no game content at all. The warm-up project is throwaway and does not carry into the contest.

### Accounts and documents
- [ ] Epic account is a full 18+ account (not Cabined), with a personal email
- [ ] Save a screenshot of the proof of enrollment
- [ ] Sign up/log in to itch.io
- [ ] Read the key points of the official rules, find out how to claim the AI tool credits (announced later)
- [ ] (Optional) Ask the school's international student services whether prize money affects the visa

### Windows environment
- [ ] Confirm the Windows machine runs UE smoothly
- [ ] Read the jam page "Get ready", confirm the UE version requirement (a third party says 5.8+, **unverified**) and install it
- [ ] Install Git + Git LFS, Visual Studio (C++ workload), Claude Code
- [ ] Push this repo to GitHub (private) and clone it on Windows

### MCP pipeline
- [ ] Enable the Unreal MCP + All Toolsets plugins, restart the editor
- [ ] Turn on Auto Start Server; Claude Code connects over local HTTP
- [ ] Warm-up project: place objects, change materials, write small C++/Blueprint logic and compile it
- [ ] Package fully once and run it in a clean environment
- [ ] Test viewport screenshots (oversized base64 blows up the context) and set a "few screenshots" rule
- [ ] Test how to recover when MCP hangs/crashes (save often)

### Prompts and scripts (kept in this repo)
- [ ] CLAUDE.md template: UE conventions, C++ first, minimal Blueprint graph edits
- [ ] Prompt library: greybox prototype, gameplay iteration, asset import, packaging troubleshooting
- [ ] Scripts: build, package, Git commit/save

### Claude Code Game Studios (CCGS)
This repo is the contest project repo. The CCGS config has been copied in from `D:\project\Claude-Code-Game-Studios` (source `Donchitos/Claude-Code-Game-Studios`, framework 1.2.0): `.claude/` (agents, skills, hooks), `CLAUDE.md`, `project.yaml`, `docs/` (including the UE 5.8 reference), `.gitattributes`, `.worktreeinclude`. The `design/`, `src/`, `production/`, `studio-tests/` directory skeletons were **not** copied.
- [x] Copy the CCGS config into this repo
- [ ] Open Claude Code in this repo and run `/start`, confirm the hooks work on Windows (depends on Git Bash, Python, jq)
- [ ] `/setup-engine`: choose Unreal, confirm it recognizes the installed UE version (the reference docs say 5.8)
- [ ] Confirm it can be used together with Unreal MCP (the CCGS docs have nothing on MCP, so verify it yourself)
- [ ] Set `modes.rigor` to `minimal` (the README calls it the jam-speed tier), try the `/brainstorm` -> `/create-stories` -> `/dev-story` <-> `/story-done` flow
- [ ] Pick the few skills to use in the contest out of 80-plus, and write them into CLAUDE.md
- [ ] Check against the official rules text: before the jam the repo holds only tools, agent config and scripts, with no gameplay, levels, art or templates
- [ ] Keep the warm-up project outside the repo; do not commit it into this repo

### Deliverables workflow
- [ ] Trial-record 1080p+ screen video and edit it into 30-60 seconds
- [ ] Trial-upload a test build to itch.io
- [ ] Prepare a devlog template (to compete for Glass Box)

### The day before the jam
- [ ] 11/11 19:00 watch the opening livestream to hear the theme
- [ ] Sleep early; be ready to get up at 05:00 on 11/12

---

## Phase 1 — Day 1 · 11/12 (Thursday)

Goal: core loop is playable

- [ ] Around 05:00 tag this repo `jam-start` (version history evidence: only tooling commits before this, game content after)
- [x] Create the UE project, enable MCP -> the project is in `Game_01/` (a repo subdirectory, C++ project, `Game_01.uproject`); Unreal MCP + All Toolsets enabled, auto start ticked, port 8000 (service response verified on 11/12)
- [x] Read the theme, settle 1 core gameplay within the first 3 hours, write a one-page scope (small over big) -> theme is Director's Cut; requirements in `docs/plans/2026-11-12-1143-feat-directors-cut-extraction-survivors-plan.md` (including the sticky note and the must/should/could tiers)
- [ ] Greybox prototype (cubes/capsules only, no art): player control + auto-attack + supply points + heat + using rare supplies on the spot + extraction
- [ ] Validate the one unproven question: is "heat + keep it or use it" fun? Find 1-2 people (friends or the jam Discord/forum) to play 5 minutes each and note their feedback; if it is not fun, change the mechanic that night
- [ ] Devlog for day 1: how the theme was approached, where the anti-luck mechanic came from, prototype feedback (to compete for Glass Box)
- [ ] That night: the core loop is playable; push to GitHub

## Phase 2 — Day 2 · 11/13

Goal: level can be completed

- [ ] Expand gameplay: 2-3 variations/difficulty steps
- [ ] Greybox the level until it can be played start to finish
- [ ] Start AI-generating/organizing art and audio assets (record the external asset list)
- [ ] Write devlog for days 1-2

## Phase 3 — Day 3 · 11/14

Goal: content complete, freeze new features in the evening

- [ ] Level completeness, unified art style
- [ ] Sound effects/music
- [ ] UI (all English)
- [ ] Freeze new features in the evening; commit before every big change

## Phase 4 — Day 4 · 11/15

Goal: build and materials ready

- [ ] Daytime: fix bugs, tune the feel, find people to playtest
- [ ] Afternoon: package the Windows build, test in a clean environment
- [ ] Record a 30-60 second video (1080p+, all English)
- [ ] Finish the devlog, organize the external asset list
- [ ] Upload the build to itch.io

## Phase 5 — Submission · 11/15 evening - 11/16 01:59

- [ ] Fill in team name, Epic email, game name, video link, log link, asset list
- [ ] **The developer** clicks submit, leaving several hours of margin
- [ ] Save the submission confirmation

---

## Risks

| # | Risk | Response |
|---|---|---|
| 1 | Windows machine performance and environment | Phase 0 must run a full package |
| 2 | MCP calls may hang/crash the editor | Save often, use Git |
| 3 | AI cannot edit Blueprint graphs reliably | Keep gameplay in C++ where possible |
| 4 | AI 3D assets need cleanup, build size limit is 1 GB | Control asset scale, check size after packaging |
| 5 | A solo developer easily neglects level design and audio (5 points each) | Reserve dedicated time in Phases 2 and 3 |
| 6 | Mistakenly putting gameplay, levels or art into this repo before the jam | Before 11/12 05:00 this repo holds only tools, config and scripts; keep the warm-up project outside the repo; the `jam-start` tag marks the boundary |
