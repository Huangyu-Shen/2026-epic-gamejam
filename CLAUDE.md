# Claude Code Game Studios -- Game Studio Agent Architecture

Indie game development managed through 49 coordinated Claude Code subagents.
Each agent owns a specific domain, enforcing separation of concerns and quality.

## Technology Stack

- **Engine**: Unreal Engine 5.8
- **Language**: C++ (primary, gameplay logic), Blueprint for data setup and presentation
- **Version Control**: Git with trunk-based development
- **Build System**: Unreal Build Tool (UBT)
- **Asset Pipeline**: Unreal Content Pipeline

> **Note**: Engine-specialist agents exist for Godot, Unity, and Unreal with
> dedicated sub-specialists. Use the set matching your engine.

## Project Structure

@.claude/docs/directory-structure.md

## Engine Version Reference

<!-- ENGINE-REFERENCE-IMPORT: the line below is engine-specific. /setup-engine
     rewrites it to @docs/engine-reference/<engine>/VERSION.md for the chosen
     engine, so a Unity or Unreal project stops loading the Godot reference every
     session. It defaults to Godot (the template's example engine); skills that
     need the pinned version read docs/engine-reference/<engine>/VERSION.md on
     demand regardless of this import. -->
@docs/engine-reference/unreal/VERSION.md


## Technical Preferences

`project.yaml` at the repo root is the primary config store — engine, specialists,
naming, platform, performance, modes. Skills resolve it via `resolve_config`
(see `.claude/docs/config-resolution.md`).

`.claude/docs/technical-preferences.md` is the **legacy fallback**, read on demand
when a key is absent from `project.yaml`. It is not imported here: before
`/setup-engine` runs it is almost entirely `[TO BE CONFIGURED]` placeholders, and
after it runs `project.yaml` holds the real values.

## Coordination Rules

@.claude/docs/coordination-rules.md

## Collaboration Protocol

**User-driven collaboration, not autonomous execution.**
Every task follows: **Question -> Options -> Decision -> Draft -> Approval**

- Agents MUST ask "May I write this to [filepath]?" before using Write/Edit tools
  (exempt: `active.md` appends, agent memory, a subagent's new file at the path
  its orchestrating skill named, see `.claude/rules/agent-memory.md`; and, during
  the jam, any edit inside the game project — the UE project at the repo root
  (`Source/`, `Content/`, `Config/`, `*.uproject`) and `design/` — when it carries
  out work the user already asked for. Those edits have the go-ahead: make them,
  then list them in the closing summary.)
- Agents MUST show drafts or summaries before requesting approval
- Multi-file changes require explicit approval for the full changeset
- No commits without user instruction
- A skipped or dismissed question is not an answer: act on nothing, re-ask in
  plain text and wait
- Started outside this file's folder? Warn first: hooks, deny rules and the
  ask-first mode load only when Claude Code starts here

See `docs/COLLABORATIVE-DESIGN-PRINCIPLE.md` for full protocol and examples.

## Working Style (Claude Fable 5.2)

This project mostly runs on Claude Fable 5.2 at `high` effort. The user hands
all code to Claude and reads it in Rider.

**Ownership.** This codebase is yours: what it's for and how it's built are your
call, and you'll keep building on it. Stay inside the scope of
`design/prd/master-prd.md`: improving structure is welcome, adding features that
are not in the current tier is not. When a change alters how an existing feature
behaves, say so explicitly in the closing summary.

**Finishing work.** Please avoid stopping while you still owe work the user asked
for. Carry on with work that does not depend on the user's answer. Diagnose
ordinary obstacles (errors, timeouts, locked files, failing tools) and work
through them; leave deliberate blockers (deny rules, files marked not to touch,
safety guardrails) alone and say plainly what you found. Stop early only when
the task cannot move forward without user input.

**Verify before claiming.** Before you say something is done, check it: build
it, run the relevant test or the game, and read the output. If you could not
check something, say so. Never invent a file, API or result to fill a gap; say
it is missing.

**Needs approval (always).** `git commit`, `git push`, tags, deleting files or
assets, changing `project.yaml`, `.claude/` or this file, installing plugins or
packages, anything outside this repo. Everything else that serves the current
request — editing code, assets and design docs, running builds, tests and the
editor, retrying a failed command — needs no approval.

**Communicating clearly.** Write for a reader who has not seen your tool calls or
the files you opened. Put the key points first, use plain words and full
sentences, and do not use names you made up during the work without saying what
they are. End a long task with a short recap that stands on its own: what you
did, what you verified, what is not done, and every file you changed.

> **First session?** If the project has no engine configured and no game concept,
> run `/start` to begin the guided onboarding flow.

## Coding Standards

@.claude/docs/coding-standards.md

## Context Management

Read `.claude/docs/context-management.md` on demand — it is a reference, not
session context. Two of its conventions are load-bearing and cited by name
elsewhere in the repo, so they are restated here rather than lost:

- **`production/session-state/active.md` is the session checkpoint.** The file is
  the memory, not the conversation. Read it first after any compaction, crash, or
  `/clear`.
- **Helpers in `.claude/scripts/` emit observations, never verdicts.** A script
  that scores or judges will eventually contradict a mode or override it cannot
  see. (Cited by `artifact-check.sh` and `adr-dep-graph.sh`.)
