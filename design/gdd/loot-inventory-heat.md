# Loot, Inventory & Heat

> **Status**: Designed
> **Author**: The developer + Claude
> **Last Updated**: 2026-11-12
> **Last Verified**: 2026-11-12
> **Implements Pillar**: P1 Every pickup is a choice (core system)

## Summary

The most distinctive system in the game. The player searches fixed supply points for common materials and rare lost film reels (called relics in this document and in the code), and packs them into a limited bag. The total value of what is in the bag is the **Heat**: the higher the Heat, the more and tougher the monsters. In v2 a reel cannot be used during the run: it has to be carried out alive, and then it unlocks an upgrade option for every later run (see the v2 note in the Overview).

> **Quick reference** — Layer: `Core` · Priority: `MVP` · Key deps: `map-level`, `player-combat`, `enemies-spawning`, `camp-meta`

## Overview

> **v2 (story 008)**: relics are now "lost film reels". This note overrides everything below it where they disagree; the rest of the document is the original design and has not been rewritten.
>
> - A reel cannot be used in a run. The use-on-the-spot effects, the buffs, the heat reduction from using a relic (`HeatReductionOnRelicUse`), the quick-use keys 1 / 2 / 3 for relics and the dismantle-at-camp action were all removed. Keys 1-3 now only cut a level-up option.
> - The bag still holds reels, one slot each, and picking one up still adds heat (`HeatPerRarePickup`). Heat is reset only when the run ends.
> - Carrying a reel out alive (extraction) unlocks the upgrade option the reel names (`FRelicRow::UnlocksOption`) for every later run, and the camp saves it. Dying loses the reels and unlocks nothing.
> - A reel drops only if its row names an option, the camp has not unlocked that option yet, and the bag does not already hold that reel (`FRelicRow::IsDroppable`). By default only the Fast Cut reel is mapped (to the option `Haste`); Wide Shot and Chain Scene have no option until story 009, so they do not drop yet.
> - The three reels are Fast Cut, Wide Shot and Chain Scene (enum `ERelicType`: Swiftstep, Warbrand, Aegis; the enum names are kept for save compatibility).
> - Loot numbers (bag size, heat, container yields and reel chances) come from the data tables `DT_LootTuning`, `DT_LootContainers` and `DT_Reels`, referenced from Director Tuning, with C++ defaults as the fallback.
> - The camp shows the unlocked options in a STORAGE list; the old warehouse list and the Dismantle buttons are gone.

- **Materials**: Iron, Herbs, Arcane Dust. Each one maps to one type of area on the map, so the player knows where to go for what.
- **Relics (lost film reels)**: Rare items. In v2 they cannot be used; carried out alive, each unlocks an upgrade option. (Original design: used during the run, or carried out and dismantled.)
- **Bag**: Capacity is counted in slots; materials stack.
- **Heat**: The total bag value is converted into 4 tiers, which directly drive the spawn director.

## Player Fantasy

"The fuller the bag, the faster the heartbeat." Every treasure picked up raises the stakes. In v2 there is no safe way to cash in a lost reel: the player has to gamble and run for the extraction point with it, to unlock a new upgrade option for every later run. (Original design: use it now and stay safely alive, or carry it out for a new building at camp.)

## Detailed Design

### Core Rules

**Materials**

| Material | Tier | Unit Value | Main Source Area | Main Use (see `camp-meta.md`) |
|---|---|---|---|---|
| Iron | Must | 1 | Armory Quarter | Smithy, Storehouse |
| Herbs | Should | 1 | Herb Gardens | Clinic, Storehouse |
| Arcane Dust | Should | 2 | Mage Tower | Editing Room |

The Must tier has only Iron: every supply point yields Iron, and every relic dismantles into Iron. Only once the Should tier is reached are materials distinguished as in the table above.

**Supply Points**

| Type | Tier | Search Time | Yield | Count on Map |
|---|---|---|---|---|
| Supply Crate | Must | 2 s | 2–4 of the area's material | 12 |
| Cache | Should | 4 s | 4–7 of the area's material; 20% chance of 1 extra relic | 5 |
| Vault | Must | 8 s | 1 guaranteed relic + 4–6 of the area's material | 2 |
| Elite Chest | Should | 2 s | Same as Cache, with random material | Where an elite dies |
| Boss Hoard | Must | Picked up on contact | 1 guaranteed relic + 8 materials (random types) | Where the Boss dies |

- Each supply point can be searched only once per run; once searched, its appearance changes to "opened".
- Supply point positions and types are fixed, so the player can memorize routes (see `map-level.md`).
- **Searching**: Stand within 250 cm of the supply point and hold E; a progress bar is shown. Moving out of range or dodging interrupts the search and resets the progress. Taking damage does not interrupt it.
- When the search completes, the yield goes straight into the bag; whatever does not fit drops on the ground next to the supply point.

**Relics** (original design, superseded by the v2 note: no use effects and no dismantling any more)

| Relic | Tier | Value | Use Effect | Dismantle Yield |
|---|---|---|---|---|
| Phoenix Feather | Must | 25 | Instantly restores full HP and grants 3 s of invulnerability | 8 Herbs |
| Warlord's Sigil | Must | 30 | Instantly grants 2 level-ups | 10 Iron |
| Storm Idol | Must | 30 | All normal monsters on screen die instantly; deals 300 damage to elites and the Boss | 6 Arcane Dust |
| Arcane Core | Should | 35 | For 30 s, damage +50% and cooldown −30% | 8 Arcane Dust |
| Blood Chalice | Should | 20 | For 20 s, restore 1 HP per monster killed | 6 Herbs |
| Golden Reel | Should | 40 | All extraction points open immediately for this run, and extraction channel time is halved | 10 Arcane Dust |

- In the Must tier, all dismantle yields are converted into the same quantity of Iron (e.g. Phoenix Feather → 8 Iron).
- Relic drop selection: pick one relic with equal probability from the relics of the currently unlocked tiers.
- The same temporary effect (e.g. two Arcane Cores) does not stack; it only refreshes the duration.

**Bag**

- 8 slots at base; each level of the camp Storehouse's Bag Capacity adds +2 slots, up to a maximum of 16 slots.
- One slot holds at most 20 of one material; once full, the excess takes another slot. One relic takes one slot.
- **Bag panel**: Press Tab to open it; time slows to 30% (not paused), and items can be used or dropped in the panel. Press Tab or Esc again to close it.
- **Quick use** (removed in v2): pressing 1 / 2 / 3 used the 1st / 2nd / 3rd relic in the bag. In v2 keys 1-3 cut a level-up option and reels cannot be used.
- **Dropping**: An item is dropped in place and becomes a ground item; a ground item can be picked back up by getting close and pressing E once (when the bag has a free slot). Ground items persist until the end of the run.
- When the bag is full, searched items drop on the ground, and a "Bag Full" prompt is shown.

**Heat**

1. Heat = the sum of the values of all items in the bag, recalculated immediately whenever the bag changes. Items on the ground do not count.
2. Heat is converted into 4 tiers, and the tier determines the spawn multiplier and the enemy HP multiplier (this file is the only source of this table; `enemies-spawning.md` references it).
3. A tier increase takes effect immediately; a tier decrease takes effect only after the new tier has held for 3 s, to prevent flip-flopping.
4. When the tier changes, the HUD plays a prompt and a sound effect.

| Tier | Heat | HeatSpawnMult | HeatHPMult | Extra Effect |
|---|---|---|---|---|
| Calm | 0–19 | 1.0 | 1.0 | — |
| Noticed | 20–49 | 1.3 | 1.15 | — |
| Hunted | 50–89 | 1.7 | 1.35 | — |
| Infamous | 90+ | 2.2 | 1.6 | Spawn one elite every 45 s (Should) |

**The "keep it or use it" balance principle**

- (v2: the "use on the spot" option and the Heat drop it gave no longer exist, so the only choice is whether to risk carrying the reel out.)
- Use on the spot: gain immediate power, and Heat drops at once, so there are fewer and weaker monsters.
- Carry it out: the dismantle yield is roughly equal to the materials from one Vault; on top of that, the pre-run editing payout multiplier applies, to materials only (see `directors-cut-editing.md`).
- Goal: the value of dismantling one carried-out relic is about the cost of one 2–3 level upgrade at camp. This way, "carry it out" is worth serious consideration in most situations.

### States and Transitions

| Supply Point State | Description |
|---|---|
| Closed | Searchable, with highlight and icon |
| Searching | While the player holds E, a progress bar is shown |
| Opened | Already searched; dimmed, cannot be searched again |

| Bag Panel State | Time |
|---|---|
| Closed | 100% |
| Open | 30% |

### Interactions with Other Systems

| System | Interaction |
|---|---|
| `enemies-spawning.md` | Provides the Heat tier multipliers; receives drop requests from elites and the Boss |
| `player-combat.md` | Interact key (E); in v2 keys 1/2/3 cut a level-up option and relics no longer modify HP, invulnerability, damage or cooldown |
| `run-progression.md` | v2: the unlocked options are read by the level-up offer (story 009). Original design: Warlord's Sigil calls the level-up interface |
| `extraction-death.md` | On successful extraction, hands over the bag contents (v2: reels unlock their options); on death, clears the bag. Original design: Golden Reel modifies the extraction rules |
| `directors-cut-editing.md` | The payout multiplier applies to materials at extraction settlement |
| `camp-meta.md` | Bag slot count, Greed and Luck come from camp; v2: unlocked options are stored in camp (original design: relics are dismantled at camp) |
| `map-level.md` | Supply point positions, types and owning areas |

## Formulas

**Heat**

```
Heat = Σ(MaterialCount × MaterialUnitValue) + Σ(RelicValue)
```

Example: the bag holds 18 Iron (18) + 6 Arcane Dust (12) + Warlord's Sigil (30) → Heat = 60 → Hunted tier. Using the Warlord's Sigil now drops Heat to 30, and 3 s later the tier drops to Noticed.

**Supply point yield**

```
Yield = floor(RandomInt(Min, Max) × (1 + 0.10 × GreedLevel))
```

| Variable | Meaning | Range |
|---|---|---|
| Min, Max | Yield range of the supply point type | See the supply point table |
| GreedLevel | Camp Storehouse Greed level | 0–5 |

Example: a Supply Crate yields 3, Greed level 2 → `floor(3 × 1.2) = 3`; a Cache yields 6, Greed level 5 → `floor(6 × 1.5) = 9`.

**Cache relic chance**

```
RelicChance = 0.20 + 0.05 × LuckLevel
```

LuckLevel ranges 0–3, giving a probability of 20%–35%.

**Expected yield per run** (used to validate the cost curve in `camp-meta.md`)

In a 5-minute run with only the starting cards, a player can search about 5 Supply Crates and 1 Vault:
`5 × 3 + 5 = 20` materials, plus 1 relic (about 8–10 when dismantled). One successful extraction totals about 28 materials.

## Edge Cases

| Situation | Handling |
|---|---|
| A relic is found while the bag is full | In v2 (implemented) the whole pickup is refused with `Pickup refused: ...` and the container stays searchable; original design: the relic drops on the ground and a "Bag Full" prompt is shown |
| The player is killed while the bag panel is open | Normal death settlement; the panel closes automatically |
| The player levels up while the bag panel is open | Close the panel first, then enter LevelUp |
| Items the player dropped are on the ground at death | They are lost along with everything else |
| Dropping items during the extraction channel | Allowed; the channel is not interrupted |
| Storm Idol is used when no monsters are on screen | Superseded in v2: relics cannot be used. Original design: consumed as usual and the effect plays |
| Golden Reel is used when all extraction points are already open | Superseded in v2: relics cannot be used. Original design: only the "channel time halved" effect applies |
| Two quick keys pressed too fast | Superseded in v2: there are no relic quick keys |
| A supply point is surrounded by enemies | No special handling; this is exactly part of Heat and the trade-offs |

## Dependencies

- Upstream: `map-level` (supply points), `camp-meta` (bag slots, Greed, Luck), `player-combat` (input).
- Downstream: `enemies-spawning` (Heat multipliers), `extraction-death` (bag contents), `run-progression` (Warlord's Sigil), `ui-hud` (bag, Heat gauge, search progress).

## Tuning Knobs

| Parameter | Default | Safe Range | Affects |
|---|---|---|---|
| Heat tier thresholds | 20 / 50 / 90 | ±50% | How fast high pressure is reached |
| Tier multipliers | See the Heat table | Spawn 1.0–3.0, HP 1.0–2.0 | Severity of the Heat penalty |
| Tier decrease delay | 3 s | 1–5 s | Whether the tier flip-flops |
| Base bag slots | 8 | 6–12 | How tight the trade-offs are |
| Stack limit per slot | 20 | 10–50 | Whether materials clog the bag |
| Search time | 2 / 4 / 8 s | 1–12 s | Risk of looting |
| Search radius | 250 cm | 150–400 | Input forgiveness |
| Bag panel time scale | 30% | 0–50% | Pressure of managing the bag |
| Relic value | 20–40 | 10–60 | Heat cost of carrying a relic |
| Relic dismantle yield | 6–10 (removed in v2) | 4–20 | Appeal of "carry it out"; in v2 the reward is the unlocked option (`UnlocksOption`, `DropWeight`) |

## Visual/Audio Requirements

- Supply points look different by type: Supply Crates are wooden crates, Caches are locked, Vaults have glowing doors; material types are distinguished by color (Iron gray-blue, Herbs green, Arcane Dust purple).
- Relics have unique icons and light beams, so they can be seen from far away when dropped on the ground.
- On a Heat tier change: the HUD Heat gauge changes color + a "film-set alarm" style cue sound; at Infamous, a faint red film grain appears at the screen edges.
- Using a relic needs a striking full-screen or character effect (full-screen lightning for Storm Idol). (Superseded in v2: relics cannot be used.)

## Game Feel

The moment of picking up a relic must have weight: a brief slow-motion (0.2 s) + a unique sound effect. A Heat increase should let the player "hear" the pressure, not just see a number.

## UI Requirements

Heat gauge, bag panel, bag quick bar (v2 implementation: a `Lost reel: <name>` line per reel, or `No lost film reel`, in the bag text; no keys), search progress, "Bag Full" prompt, and direction markers for supply points at the HUD edge. See `ui-hud.md`.

## Cross-References

- `design/gdd/enemies-spawning.md` (references this file's Heat multiplier table)
- `design/gdd/camp-meta.md` (dismantling (removed in v2), bag slots, Greed, Luck)
- `design/gdd/directors-cut-editing.md` (payout multiplier)
- `design/gdd/extraction-death.md` (what is carried out and what is lost)

## Acceptance Criteria

- **AC1 (Must)**: Holding E for 2 s to search a Supply Crate adds 2–4 Iron to the bag, the crate becomes opened, and it cannot be searched again.
- **AC2 (Must)**: Moving out of the 250 cm range during a search resets the progress.
- **AC3 (Must)**: When bag Heat is ≥ 20, the HUD shows Noticed, and the spawn rate is calculated at 1.3x (verifiable in the log).
- **AC4 (Must)**: *(Superseded in v2.)* New criterion: a lost film reel carried out alive adds its option to the camp's unlocked options and the result screen lists it; dying with a reel unlocks nothing. Original: after using the Warlord's Sigil, 2 level-up choices pop up in succession, and Heat decreases by 30.
- **AC5 (Must)**: Once all 8 bag slots are full, further searched items drop on the ground with a "Bag Full" prompt.
- **AC6 (Must)**: Pressing Tab opens the bag, game speed becomes 30%, and items can be used and dropped.
- **AC7 (Should)**: Supply Crates in the Armory Quarter yield only Iron, those in the Herb Gardens only Herbs, and those in the Mage Tower only Arcane Dust.
- **AC8 (Must)**: After Heat falls from Hunted below the Noticed threshold, the tier changes only after 3 s.

## Open Questions

- Items dropped on the ground do not count toward Heat, so players may drop treasure near the extraction point first and come back for it after the fight. This is allowed by default and treated as a strategy; if playtesting finds it breaks the tension, change it so ground items vanish after 30 s.
