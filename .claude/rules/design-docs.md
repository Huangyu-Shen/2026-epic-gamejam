---
paths:
  - "design/gdd/**"
---

# Design Document Rules

- The 8 GDD sections are: Overview, Player Fantasy, Detailed Rules, Formulas, Edge Cases, Dependencies, Tuning Knobs, Acceptance Criteria
- **How many are required depends on `modes.workflow`** — all 8 at `full`, 5 (+ Formulas when the system **defines numeric rules**: rates, curves, thresholds, costs, damage, drop weights — anything a balance pass would tune) at `standard`, none at `minimal` (the design record there is `design/game-brief.md`). Resolve the tier before flagging a section as missing; see `.claude/docs/workflow-modes.md`
- **The Formulas test is what the system defines, not what it is called.** A system's `Category` in `systems-index.md` is a hint — `Gameplay`, `Economy` and `Progression` almost always qualify — never the test. Do not gate Formulas on a category token: the categories `/map-systems` writes (`Core · Gameplay · Progression · Economy · Persistence · UI · Audio · Narrative · Meta`) do not include `combat` or `AI`, which are example systems *within* `Gameplay`. Every site stating this rule must state the same test.
- **Section-name alias**: The GDD template and `/design-system` author this section as `## Detailed Design`; the design standard calls it **Detailed Rules**. These are the SAME required section — accept either heading and never rewrite one into the other.
- Formulas must include variable definitions, expected value ranges, and example calculations
- Edge cases must explicitly state what happens, not just "handle gracefully"
- Dependencies must be bidirectional — if system A depends on B, B's doc must mention A
- Tuning knobs must specify safe ranges and what gameplay aspect they affect
- Acceptance criteria must be testable — a QA tester must be able to verify pass/fail
- No hand-waving: "the system should feel good" is not a valid specification
- Balance values must link to their source formula or rationale
- A new design document whose content is still undecided is written incrementally:
  create the skeleton first, then fill one section at a time with user approval
  between sections, writing each approved section to the file immediately. The
  reason is that open design decisions belong to the user. Exception: when the
  decisions were already made (in a confirmed plan, PRD or earlier dialogue), or
  the user asks for whole documents in one pass, or the change is an edit or
  tuning pass on an existing doc, write the whole thing, then show a summary of
  what was written and which values are defaults rather than decisions
