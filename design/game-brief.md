# Game Brief: Director's Cut (v3)

> Working title. The only gate on scope is `design/prd/master-prd.md`; this page is a one-page summary of it, and the PRD wins on any conflict.
> v2 (2026-11-13): all 7 v1 MVP items are done (stories 001-007). v2 moves the theme out of the pre-run menu and into every level-up.
> v3 (2026-11-14, after the first playtest of the package): the level-up rule is **pick one of three, with an optional cut** (the old "cut one, keep two" played backwards for players). The pool grew from 7 to 13 options. Where this page or the PRD still says "cut one, keep two", v3 is what the game does.

**One-sentence pitch:** A fantasy top-down mower-style search-and-extract game. At each level-up, pick one of three cards, and may first cut one so it goes to the boss; three identical copies merge into a star upgrade; carry a lost film reel out alive, and every later run's upgrade pool gains one more option.

## Core loop
- Camp: upgrade buildings with materials; before departure, slot film cards to set difficulty and multiplier (unchanged)
- In run: mow down enemies; at level-up, **take one of three; optionally cut one first** (the cut card leaves the pool and goes to the boss; cards neither taken nor cut stay in the pool); 3 copies make a 2-star, 9 copies make a 3-star
- Search vaults for **lost film reels**; heat rises while you carry one
- Extraction: materials x multiplier, and the reel you carry out permanently unlocks a new upgrade option; dying loses everything

## Player goal & fail state — what "working" looks like
- Goal: cut your build into shape to hit big numbers, and carry reels out to grow the upgrade pool.
- Fail: death, and the whole bag is lost.
- Acceptance (within 15 minutes from a cold start): at least 2 runs; one star-up; one permanent cut; one reel carried out and its new upgrade option seen in the next run; one camp building change.

## MVP v2 — new work (the 7 v1 items are done)
1. **Pick one of three, optional cut** (v3; v2 was "cut one, keep two"): at most one cut per level-up; cut cards leave the pool for the run and the boss gets them; number of permanent cuts = pool size − 4 (9 on a new save with the 13-option pool), and once used up the cut is refused
2. **Three-into-one star merge**: stacks on top of the existing level; on star-up, numbers and look (projectile, area, size) jump together
3. **Lost film reels**: relics become unlock keys; remove "use" and "salvage"; the save records unlocked options
4. **3 unlockable upgrade options**: 2 passives + 1 weapon
5. **Number retune**: XP curve, monster strength, heat
6. **Art, audio and feel**: a unified-style asset set, core sound effects, music, hit feedback
7. **The Cut Knight brings the rejects**: the boss uses what you cut this run. Built (story 012): a cut weapon gives the boss an attack, a cut passive raises a boss stat

## Out of scope — not building this
- Discussed but not doing: scene-based structure, gold shop, CUT screen wipe, gray-to-color restore options, cross-run multiplier, achievement unlocks
- Cut from the original plan: more cards, monsters, materials, buildings; NG film
- Never doing: multiplayer, random maps, drag-and-drop timeline, walkable camp, gamepad, multiple languages, gacha / pay-to-win / forced online

## Build order
1. Package test (not a feature, but we must confirm first that it packages)
2. Level-up cut rule -> three-into-one star merge -> first number pass
3. Lost film reels -> 3 unlock options
4. Art, audio, feel (Day 3, features frozen at 23:00 that night)
5. The Cut Knight brings the rejects (only if there is spare capacity)

## Changes to v1 behavior
- Level-up was changed twice: v1 pick one of three; v2 cut one, keep two; v3 (now) pick one of three with an optional cut. The base pool is 5 weapons (Blade, Bolt, Nova, Gun, Orbit) and 8 passives (Swiftness, Might, Vitality, Greed, Reach, Armor, Regen, Crit); 3 more (Chain, Haste, Magnet) are unlocked by lost film reels.
- Relics can no longer be used, and can no longer be salvaged at the warehouse.
- "Keep it or use it" is no longer a selling point.

---
**Who it's for / what they feel:** One player, keyboard and mouse, a judge who has never played it, playing for 10-20 minutes; they feel that every level-up is cutting their own build, and the numbers keep rolling bigger.

**Art & audio direction:** A unified material and palette pulls the assets together; a few hero assets are AI-generated or from free packs; audio uses WeLoveIndies (non-commercial only) and the engine's built-ins.

**Reference game:** Vampire Survivors (auto-attack mowing), Brotato (the thrill of a snowballing build), Vampire Crawlers (relics as unlock keys); from extraction games we keep only "station extraction, death loses everything".
