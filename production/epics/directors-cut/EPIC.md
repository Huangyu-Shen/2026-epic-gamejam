# Epic: Director's Cut (MVP)

> **Source**: `design/game-brief.md` (scope gate: `design/prd/master-prd.md`)
> **Status**: Active

**Goal** (v2, from the brief): A fantasy top-down horde-survival game with extraction. At each level-up the player picks one of three options and may cut one first (it goes to the boss); three copies merge into a star upgrade; a lost film reel carried out alive adds one option to every later run's pool. Film cards before the run still set difficulty and payout.

**Scope**: stories 001-007 are the brief's v1 MVP (done). Stories 008-012 are the brief's v2 items 3-7; v2 items 1 and 2 (cut one, keep two; star merge) were built on 11/13 without a story file and are recorded in commit `e1f27fe` and `production/qa/evidence/v2-cut-one-keep-two/`. Story 013 is the submission checklist, which is not a feature. Anything outside the brief's list stays out (see the brief's Out of scope).

## Stories

| # | Story | Type | Status | ADR |
|---|-------|------|--------|-----|
| 001 | Loot, bag and heat — the keep-or-use choice | Logic | Complete | N/A |
| 002 | Extraction and death | Integration | Complete | N/A |
| 003 | In-run combat, enemies and greybox map | Visual/Feel | Complete | N/A |
| 004 | Run progression (level-up, weapons, passives) | Logic | Complete | N/A |
| 005 | Pre-run editing (film cards, multiplier) | Integration | Complete | N/A |
| 006 | Camp and meta growth | Integration | Complete | N/A |
| 007 | Game flow, save and must-have UI | UI | Complete | N/A |
| 008 | Lost film reels — relics become unlock keys | Integration | Complete | N/A |
| 009 | Three unlockable upgrade options | Logic | Ready | N/A |
| 010 | Number retune — XP curve, monster strength, heat | Config/Data | Ready | N/A |
| 011 | Art, audio and feel | Visual/Feel | Ready | N/A |
| 012 | The Cut Knight brings the rejects (only if time allows) | Integration | Ready | N/A |
| 013 | Package and submission materials | Config/Data | Ready | N/A |

## Order for Day 3 (11/14) and Day 4 (11/15)

Two gates fall on Day 3 (PRD §8): at 18:00 every Should item not yet started is cut and Could items no longer begin; at 23:00 features freeze. The five open feature stories add up to two Medium, one Small and two Large, which is more than one day, so the order decides what ships.

1. **008** in full (Must).
2. **009 minimum**: one passive, wired to its reel end to end. This closes the "carry a reel out, see the new option" experience.
3. **011 code part**: real weapon visuals, the audio entry point, hit feedback. These are code and freeze at 23:00.
4. **010 first pass** on the packaged build.
5. The rest of **009** (second passive, the weapon), only if started before 18:00.
6. **012**, only if started before 18:00 and only after step 3.
7. After the freeze: **011** asset swaps, **010** final pass, then **013** on Day 4.

