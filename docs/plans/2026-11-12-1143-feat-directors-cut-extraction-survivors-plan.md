---
title: Director's Cut Extraction Survivors - Plan
type: feat
date: 2026-11-12
topic: directors-cut-extraction-survivors
artifact_contract: ce-unified-plan/v1
product_contract_source: ce-brainstorm
execution: code
---

# Director's Cut Extraction Survivors - Plan

## Goal Capsule

- **Objective:** In a 10-20 minute playthrough, a judge finishes at least two runs, feels that "every piece of loot is worth agonizing over", and also sees that the "director's cut" is a gameplay mechanic, not just packaging. The game is a Western-fantasy, 3D top-down, mow-down loot-and-extract game with outside-the-run camp growth.
- **Product authority:** The developer (solo entrant). Timeline, hard rules and submission process follow `PLAN.md`.
- **Open blockers:** None.

---

## Product Contract

### Summary

A Western-fantasy 3D top-down mow-down loot-and-extract game. The world is a film that has been cut to ribbons. Before each run, the player slots clip cards into the film slots to decide this run's elites, horde, Boss, extraction points and duration. In the run, the player auto-attacks to mow down enemies and builds up their character while looting common materials and rare supplies on a fixed map: rare supplies can be used on the spot to turn the tide, but the more valuable the backpack, the more and stronger the monsters. After a successful extraction, materials are used to upgrade the camp. The Editing Room in the camp "restores cut clips", turning them into new cards and new weapons.

### Sticky Note

> Fantasy top-down mower. Slot cards before departure to set the difficulty. The more treasure you find, the stronger the monsters get: use it to survive, or carry it out to build the camp.

Any new idea that does not fit on this sticky note goes into Scope Boundaries, not into this development.

### Delivery Tiers

Requirements are delivered in three tiers. The next tier starts only after the previous one is fully playable; features freeze on the evening of Day 3, and unfinished tiers are cut outright.

| Tier | Content | Deadline |
|---|---|---|
| Must | Movement, auto-attack, 1 enemy type, pick one of three on level-up (R1, R3); 1 material, fixed supply points (R2, R7); rare supplies usable on the spot (R8); heat (R10); backpack limit (R9); extraction and death (R4-R6, the loss part of R11); 2 film slots, 2 cards: horde, Boss (R12-R14); 2 camp buildings: Blacksmith, Clinic, exponential costs (R16, R18, R19) | End of Day 2 can run one full loop |
| Should | Materials increase to 3 (R7); 4 buildings, including the Editing Room unlocking new cards and new weapons (R15, R17); extraction and duration cards (R13); NG film progress (R11) | Day 3 daytime |
| Could | Building appearance changes with level (R16); secondary stats such as luck and revival (R17); more enemy types, more upgrade pool content | If there is spare capacity, frozen on the evening of Day 3 |

### Reference Games

| Game | What to borrow |
|---|---|
| Vampire Survivors | Auto-attack mowing, in-run pick-one-of-three level-ups, outside-the-run stat upgrade list |
| Halls of Torment | Camera and readability of a 3D (top-down) survivors game |
| Delta Force / Escape from Tarkov | The tension of the extraction countdown, "it only counts if you carry it out" |
| Hades | Returning to camp to see growth, the camp as a pause between runs |

### Problem Frame

In current extraction games (Delta Force, Arena Breakout, Escape from Tarkov), what you carry out of a run has almost no effect outside the run, and outside growth is often blocked by specific random drops. With bad luck you cannot level up. After being broken down, players see no steady progress, so they have no motivation to take risks again, and at some point they cannot keep playing.

Vampire Survivors-style games have stable outside growth, but the in-run pickups are just numbers, with no "keep or use" trade-off.

This contest lasts only 4 days (11/12-11/15), the developer works solo, and uses UE. The seven scoring categories are worth 5 points each, and the "Theme" score looks at how fresh the interpretation of Director's Cut is. Judges play only a short time, so the core experience must appear in the first few runs.

### Key Decisions

- **Genre: survivors x extraction x camp building.** It merges the developer's two ideas, "3D Vampire Survivors" and "make extraction games more valuable". (session-settled: user-directed — chosen over pure 3D survivors, a solo fantasy adventure, and a local party game: feasible solo in 4 days, and "in-run value affects outside the run" is the point with the most room for innovation)
- **Solve "bad luck means no level-ups" with a combination of four mechanics.** The four are: fixed points to fetch on demand, common materials, carry out or use on the spot, and keeping progress on failure. Governs R2, R7, R8, R11, R18. (session-settled: user-directed — all four candidates adopted, and the developer rewrote the material rules)
- **2-3 common materials, not 1 universal material.** Keeps the route planning of "go to this kind of point for the building you want to upgrade". Governs R7, R18. (session-settled: user-approved — a single material was not chosen because it would strip fixed points of planning meaning)
- **The theme enters from two layers: the Editing Room restores cut content, plus pre-run cutting with card slots.** The two layers interlock: the Editing Room unlocks cards, and the pre-run step spends them. Governs R12-R15, R17. (session-settled: user-directed — chosen over doing only one layer, or only a "film set Take" wrapper)
- **Pre-run cutting is card slots, not a draggable timeline.** Limits the scope to what a jam can bear. Governs R12. (session-settled: user-approved — the UI cost of a timeline editor is too high)
- **The jam version's clip cards cover only two groups: monster + climax scene, and extraction + duration.** Governs R13. (session-settled: user-directed — zone/loot cards and atmosphere cards were not chosen)
- **Setting is Western fantasy, mainly with free assets.** Chosen by "most available free material", and it also fits the fantasy adventure direction the developer wants to make after the jam. Governs R20. (session-settled: user-approved — B-movie horror and retro sci-fi were not chosen because they have few free assets and a style that is hard to unify)
- **The camera is top-down, and weapons auto-attack.** Best readability for hordes and the easiest to build. Governs R1. (session-settled: user-approved — twin-stick aiming and a third-person over-the-shoulder camera were not chosen)
- **The camp is a 3D scene with a fixed camera and click-to-select buildings.** Growth is visible, with moderate workload. Governs R16. (session-settled: user-approved — a walkable camp and a pure UI menu were not chosen)
- **Keep the outside layer "thin": only Vampire Survivors-style stat upgrades.** The run is the core; the outside layer only makes "carrying it out" worthwhile. Governs R17-R19. (session-settled: user-directed — the developer took Vampire Survivors' outside upgrade list as the reference and rejected the plan of "no outside layer, only a box-office score")
- **The heat mechanic is the developer's own proposal: the more valuable the loot in the backpack, the more and stronger the monsters.** It makes every pickup a decision. Governs R10.

### Requirements

**In-run: map and combat**

- R1. Top-down 3D camera. WASD to move, weapons auto-attack; there is also a dodge, and an interact key for searching and extraction.
- R2. Only one hand-built fixed map. The material category produced by each supply point is fixed and knowable, with a few high-grade supply points; only the quantity and rarity bonus are random.
- R3. In-run growth follows Vampire Survivors: kills give XP, and on level-up the player picks a new weapon, skill or enhancement from the upgrade pool.
- R4. The map has multiple extraction points, which open one after another over time. The player extracts by standing in an extraction point for the required time (default 30 seconds, adjustable).
- R5. The player decides when to extract; a run targets 3-10 minutes.
- R6. On successful extraction, all items in the backpack go into the camp storage.

**Loot, backpack and heat**

- R7. There are 2-3 common materials, which can be found directly in the run.
- R8. Rare supplies can be consumed in the run in exchange for powerful growth that can turn the tide.
- R9. Backpack capacity is limited; when the pack is full in the run, the player must make trade-offs.
- R10. Heat: the higher the total value of the items in the backpack, the more monsters spawn and the stronger they are. Heat takes effect in real time and falls after items are consumed or dropped.
- R11. On death all items in the backpack are lost, but the run counts as an "NG film" that gives the Editing Room some restore progress.

**Pre-run editing (Director's Cut)**

- R12. Before each run, the player slots clip cards into film slots (3-5; fewer at first, more after the Editing Room is upgraded), and the run starts once confirmed.
- R13. The jam version's clip cards fall into four categories:
  - Monster: elite waves, horde, swap monster type
  - Climax scene: insert a Boss scene
  - Extraction: delete an extraction point, or delay the extraction window
  - Duration: short cut or extended cut
- R14. Each card has a risk value. The sum of the slotted cards' risk determines this run's material payout multiplier, shown in the UI before departure.
- R15. New clip cards are unlocked by the Editing Room "restoring cut clips".

**Outside the run: camp**

- R16. The camp is a 3D scene with a fixed camera. Clicking a building opens an upgrade panel; after a building is upgraded, its appearance visibly changes.
- R17. The camp has four buildings; the upgrades each handles are in the table below.

| Building | Upgrades |
|---|---|
| Editing Room | Film slot count, unlock new clip cards, add new weapons/skills to the in-run upgrade pool, luck |
| Blacksmith | Attack power, attack range, projectile count |
| Clinic/Shrine | Max health, health regeneration, revival count |
| Storage/Packer | Backpack capacity, greed (material gain) |

- R18. Building upgrades cost only common materials, with each level's requirement growing exponentially. Rare supplies can be salvaged into more common materials, or left unsalvaged and hoarded in storage.
- R19. The first 2-3 upgrades must be reachable within the first 2-3 runs.

**Presentation**

- R20. Art is mainly free external assets, with a unified overall style; all external assets are listed at submission.
- R21. World framing: the world is a fantasy film cut to ribbons, and the Editing Room is a building in the camp. All in-game text is in English.

### Key Flows

- F1. One full loop
  - **Trigger:** The player clicks "Depart" in the camp.
  - **Steps:** Slot cards in the Editing Room and see the risk multiplier -> enter the fixed map, mow and level up -> go to supply points as needed to loot, heat rises -> agonize over whether to keep or use rare supplies -> go to an open extraction point and stand the full time -> return to camp, salvage or hoard, upgrade buildings.
  - **Outcome:** The camp gets stronger, may unlock new cards or weapons, and the next run has more editing options.
  - **Covered by:** R2-R18

### Acceptance Examples

- AE1. **Covers R4, R6.** Given an open extraction point, if the player stands in the point for the full 30 seconds, extraction succeeds and all backpack items go into the camp storage.
- AE2. **Covers R11.** If the player dies with rare supplies in the backpack, all the supplies are lost, and the Editing Room restore progress increases by one NG film's worth.
- AE3. **Covers R8, R10.** The player picks up a high-value rare supply, heat rises, and monsters spawn in greater numbers and strength; the player uses it on the spot, gains powerful growth, and heat falls back.
- AE4. **Covers R14.** The player slots the "Horde" card and the "Boss" card; before departure the UI shows the total risk and the payout multiplier; on extraction in this run, the materials carried out are computed with that multiplier.
- AE5. **Covers R19.** A new-save player slots only basic cards and extracts successfully in both of the first two runs, so can upgrade buildings at least twice.

### Success Criteria

- A judge who has never played can complete at least two runs in 10-20 minutes, and do the following:
  - At least once, a pre-run cut changes the Boss, elites or horde
  - Face a "keep it or use it" decision at least once
  - See a camp building change at least once
- The packaged Windows build runs in a clean environment and is under 1 GB.

### Scope Boundaries

**Later:**

- Zone/scene, loot and atmosphere (night/storm) clip cards
- Random maps, multiple maps
- Online co-op, local party mode
- Collection showcase. In the jam version, hoarding shows up only as "storage can leave things unsalvaged".
- Safe boxes and similar guaranteed slots
- A draggable timeline editor
- A character that can walk around the camp

**Not part of this game's identity:**

- Gacha, microtransactions, forced online play

### Dependencies / Assumptions

- Free assets are prepared on these assumptions: the Infinity Blade series, Megascans, Paragon characters and so on are still free to use in UE projects on Fab, but their 2026 availability is unverified.
- The official rules allow licensed external assets, to be listed at submission; `PLAN.md` records that external material is not scored, so the visuals score has to come from integration and a unified style.
- The UE project `Game_Jam_01/` is currently an empty project with no game content.

### Outstanding Questions

**Deferred to Planning**

- How the timer is handled if the player leaves the extraction zone or takes attacks midway.
- Values: extraction duration, extraction point opening times, heat curve, mapping between risk value and multiplier, starting point and base of the upgrade cost curve.
- Using a rare supply on the spot both grants a boost and lowers heat, so "using" naturally dominates; values must make "keeping" worth just as much.
- What the 2-3 materials are called, and what each corresponds to in the fantasy world (such as metal, cloth, magic parts).
- Which asset packs are actually usable, and which pack is used for characters, monsters, Boss and camp buildings.
- Which systems use C++ and which use Blueprint.
