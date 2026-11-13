---
name: ccgs
description: TRAE entry point for the Claude Code Game Studios workflow. Indexes the 74 game-development workflows under .claude/skills and runs them step by step. Use when the user mentions story, epic, sprint, GDD, ADR, QA, code review, milestone, or release checks, or wants to develop or review a game following the studio workflow. Not for ad-hoc coding tasks unrelated to the studio workflow.
---

# CCGS - Claude Code Game Studios Workflow Entry

This repo ships the whole CCGS toolchain as a "project template" under `.claude/`, so **nothing needs to be installed**.
This skill is the TRAE-side entry point: it maps "which step the user wants to do" to a specific workflow file, then follows the steps in that file.

## What each location holds

| Location | Contents |
|---|---|
| `.claude/skills/<name>/SKILL.md` | 74 workflow definitions, **the thing you actually need to read** |
| `.claude/agents/` | 49 role definitions (read the matching role's constraints before coding) |
| `.claude/rules/` | Coding and documentation rules |
| `.claude/hooks/` | Pre-processing scripts (`yaml-helper.sh`, etc.) |
| `.claude/scripts/` | Status query scripts (`story-status.sh`, `artifact-check.sh`, etc.) |
| `.claude/docs/` | Templates and references (`workflow-catalog.yaml` is the authoritative stage table) |
| `CLAUDE.md` | Main config, automatically imported by TRAE rules |
| `project.yaml` | Machine-readable config (single source of truth) |

## Execution protocol

1. **Locate first**: find the matching workflow in the table below, then `Read .claude/skills/<name>/SKILL.md`.
2. **Follow it**: that file is the operating manual. Run its steps in order and do not simplify from memory.
3. **Run the prefetch commands yourself**: `` !`bash ...` `` in a `SKILL.md` is Claude Code's pre-execution syntax, and TRAE does not run it automatically.
   When you see it, run the same command by hand in Git Bash, then use the result as the "resolved value".
4. **Ignore Claude Code-specific frontmatter**: `allowed-tools`, `model`, `argument-hint` and `user-invocable` have no effect and need no simulation.

### How to run commands

`bash` is not on PATH, so you must use Git Bash by absolute path:

```powershell
& "D:\Git\Git\bin\bash.exe" -lc 'cd "/d/project/2026-epic-ai-megajam" && bash .claude/hooks/yaml-helper.sh resolve_config --keys project.stage,workflow'
```

Verified working. Example output:

```
workflow: minimal (rigor:minimal)
project.stage: Systems Design (project.yaml)
```

## Workflow index

| Workflow | Purpose |
|---|---|
| `start` | First-time onboarding, pick a starting workflow |
| `help` | Stuck / not sure what to do next |
| `project-stage-detect` | Determine the project's stage and find gaps |
| `onboard` | Generate an onboarding guide for a new agent or person |
| `brainstorm` | Find a game concept from scratch or change direction |
| `prototype` | Throwaway prototype before the GDD, ending in PROCEED/PIVOT/KILL |
| `setup-engine` | Configure and pin the engine version |
| `settings` | View or change project config |
| `map-systems` | Break the concept into systems, order dependencies and design sequence |
| `art-bible` | Visual identity baseline (before map-systems) |
| `design-system` | Write one system's GDD section by section |
| `quick-design` | Lightweight spec for small changes, embedded straight into a story |
| `review-all-gdds` | Overall cross-GDD consistency review |
| `design-review` | Completeness and implementability review of a single design doc |
| `ux-design` | Write the UX spec for a screen, flow or HUD section by section |
| `ux-review` | Review of a UX spec and its implementation |
| `consistency-check` | Cross-document conflict scan between GDDs and the entity registry |
| `content-audit` | GDD planned amounts vs implemented amounts |
| `create-architecture` | Architecture blueprint before writing code |
| `architecture-decision` | Produce an ADR (context / alternatives / consequences) |
| `architecture-review` | GDD requirement -> ADR traceability matrix |
| `create-control-manifest` | Distill accepted ADRs into a "must do / must not do" cheat sheet |
| `create-epics` | GDD + architecture -> epics (split by module) |
| `create-stories` | One epic -> implementable stories + acceptance criteria |
| `story-readiness` | Whether a story is ready to start |
| `dev-story` | **Implement one story** (with tests, following its ADRs) |
| `story-done` | Close a story: verify acceptance criteria one by one |
| `estimate` | Task effort estimation |
| `sprint-plan` | Plan or update a sprint |
| `sprint-status` | Sprint snapshot and risks |
| `vertical-slice` | End-to-end validation slice before going into production |
| `gate-check` | Phase advancement gate: PASS/CONCERNS/FAIL |
| `milestone-review` | Milestone review, go/no-go |
| `retrospective` | Sprint/milestone retrospective |
| `scope-check` | Scope creep check |
| `balance-check` | Imbalance check for numbers, economy and progression curves |
| `perf-profile` | Performance profiling and ranked optimization suggestions |
| `playtest-report` | Playtest report (template, or structure existing notes) |
| `qa-plan` | Test plan for one sprint |
| `smoke-check` | Smoke gate before QA handoff |
| `soak-test` | Long-run stability protocol |
| `regression-suite` | Mapping of test coverage to critical paths |
| `test-setup` | Set up the test framework and CI (once, before the first sprint) |
| `test-helpers` | Generate engine-specific test helper libraries |
| `test-evidence-review` | Quality review of test evidence |
| `test-flakiness` | Find flaky tests from CI logs |
| `code-review` | Architecture-level code review |
| `tech-debt` | Tech debt register and prioritization |
| `bug-report` | Structured bug report / code risk analysis |
| `bug-triage` | Re-prioritize existing bugs |
| `hotfix` | Emergency fix lane |
| `day-one-patch` | Launch-day patch |
| `release-checklist` | Pre-release checklist |
| `launch-checklist` | All-department launch readiness review |
| `changelog` | Generate a changelog from commit history |
| `patch-notes` | Player-facing patch notes |
| `security-audit` | Audit of save tampering, cheating, network and data exposure |
| `localize` | Localization workflow |
| `asset-spec` | Per-asset visual specs + AI generation prompts |
| `asset-audit` | Audit of asset naming, size and format |
| `adopt` | Compliance audit and migration plan for an existing project |
| `reverse-document` | Work backwards from an existing implementation to design/architecture docs |
| `propagate-design-change` | Find ADRs invalidated after a GDD change |
| `team-combat` | Combat team orchestration (design -> implement -> validate) |
| `team-level` | Level team orchestration (full area production) |
| `team-narrative` | Narrative team orchestration |
| `team-ui` | UI team orchestration (through the UX pipeline) |
| `team-audio` | Audio team orchestration |
| `team-live-ops` | Live-ops team orchestration |
| `team-polish` | Polish team orchestration (performance / presentation / feel) |
| `team-qa` | QA team, full test cycle |
| `team-release` | Release team executes a release |
| `skill-test` | Validate skill file structure |
| `skill-improve` | Skill test-fix-retest loop |

## Integration with TRAE

- `CLAUDE.md` is imported by TRAE rules (`AI.rules.importClaudeMd`), so the collaboration protocol and coding standards take effect automatically.
- Unreal-side capabilities go through the MCP `unreal-mcp` (`list_toolsets` / `describe_toolset` / `call_tool`), which can read and write the editor directly.
- Current stage of this repo: `Systems Design`, workflow level `minimal`.
- Active epics are in `production/epics/`, and the session checkpoint is in `production/session-state/active.md`.
