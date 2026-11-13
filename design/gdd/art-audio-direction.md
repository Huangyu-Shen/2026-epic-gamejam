# Art & Audio Direction

> **Status**: Designed
> **Author**: The developer + Claude
> **Last Updated**: 2026-11-12
> **Last Verified**: 2026-11-12
> **Implements Pillar**: The Director's Cut theme's look and sound; the "visuals" and "audio" judging criteria

## Summary

The visual direction is "dark fantasy on a film set": a fantasy town and monsters, with traces of a movie set showing everywhere. Art relies mainly on free external assets, graybox first and replacement later, and uses a unified color grade, lighting and post-processing to merge assets from different sources into one style. Audio likewise relies mainly on free resources, with the focus on hit feedback and sound effects for key events.

> **Quick reference** - Layer: `Presentation` · Priority: `Vertical Slice` (the graybox readability spec is MVP) · Key deps: `map-level`, `ui-hud`, `enemies-spawning`

## Overview

Two phases:
1. **Graybox phase (Day 1-2, Must)**: everything uses basic shapes and flat-color materials, and only needs to be "readable": the player, enemies, supply points and extraction points can be told apart at a glance.
2. **Replacement phase (Day 3, Should)**: bring in free assets, unify the style, and add sound effects and music.

## Player Fantasy

"An old fantasy movie that was badly cut, and I am the one reshooting it." The picture is gloomy but has warm light, the monsters look like skeleton extras, and the set lights and film elements remind the player: this is a scene being shot.

## Detailed Design

### Core Rules

**Visual style**

1. Tone: night, cold blue ambient light + warm orange set lights (point lights). The cold/warm contrast is the game's unified "signature".
2. The style sits between realistic and stylized: mixing assets is allowed, but they must all go through the same post-processing (tone mapping, color grading, film grain, a slight vignette).
3. Set elements are used as accents: set lights, scaffolding, film reels, clapperboards, camera tracks. They appear at map edges, area transitions and the camp, and do not interfere with the readability of the combat area.
4. Readability takes priority over detail: under the angled top-down camera, characters and enemies must have a clear silhouette; ground texture contrast must be lower than that of characters.

**Graybox readability spec (Must)**

| Object | Graybox representation | Color |
|---|---|---|
| Player | Capsule + facing arrow | White |
| Skeleton Grunt | Small cube | Bone white |
| Ghoul Runner | Narrow, long box | Sickly green |
| Bone Archer | Triangular prism | Dark purple |
| Elite | Original shape ×1.5 + gold outline | - |
| Boss | Large capsule | Dark red |
| Supply Crate / Stash / Vault | Small / medium / large cube | By material: gray-blue / green / purple |
| Extraction point | Ground ring + light pillar | Gray when closed, bright white when open |
| Walls and buildings | Gray boxes | Mid gray |
| Ground | Plane | Dark gray |

**Visual theme per area (Should)**

| Area | Main color | Landmark | Atmosphere |
|---|---|---|---|
| Central Plaza | Warm orange | Fountain | Where the set lights are most concentrated |
| Armory Quarter | Iron gray + firelight | Big chimney | Forges, weapon racks |
| Herb Gardens | Dark green | Windmill | Vegetable plots, fences, fireflies |
| Mage Tower | Purple | Tall tower | Floating runes, arcane light |
| Graveyard | Cold blue + fog | Spire | Gravestones, dead trees, thin mist |

**Free asset candidates (Should, availability must be verified on Day 2)**

| Use | First choice | Fallback |
|---|---|---|
| Scenery (town, ruins) | Megascans medieval/ruins assets | Free stylized medieval packs on Fab |
| Monsters (skeletons, ghouls) | Infinity Blade: Adversaries / Enemies | Free skeleton packs on Fab |
| Player character | Paragon series characters (pick one melee hero) | Infinity Blade: Warriors |
| Weapons and effects | Infinity Blade: Weapons / Effects | Niagara samples that ship with UE |
| Camp | Megascans tents, wooden huts, campfires | Stylized medieval packs |

Rules:
- Whether the assets above are still free on Fab in 2026, and whether their licenses allow use in a competition entry, are **both unverified**. On Day 2, download them and confirm the license first; if one is unusable, switch to the fallback.
- Every time an external asset pack is used, record it in the external asset list immediately (pack name, source link, license, where it is used). The list must be attached at submission.
- Do not use any asset that carries a trademark or logo.

**Audio**

| Category | Content | Tier |
|---|---|---|
| Combat | Weapon hits, enemy deaths, player getting hit | Should |
| Pickups | XP gems, materials, relics (distinct) | Should |
| Events | Level up, heat tier-up, horde warning, Boss entrance, extraction opens, extraction countdown, extraction success, death NG | Should |
| UI | Button hover/click, card insert, clapperboard clap | Should |
| Music | 1 track for camp (calm), 1 track in-run (tense) | Should |
| Dynamic music | In-run music adds layers with the heat tier | Could |
| Ambience | One looping ambient track per area | Could |

- Free audio sources: WeLoveIndies (free for participants, **non-commercial use only**; commercializing after the jam requires buying a license), free audio on Fab, UE's built-in sound effects.
- All external audio is likewise recorded in the external asset list.
- All voice and text is in English (the clapper loader's countdown "Five, four..." can use TTS or have no voice).

### States and Transitions

Not applicable.

### Interactions with Other Systems

| System | Interaction |
|---|---|
| `map-level.md` | Area themes and landmarks |
| `enemies-spawning.md` | Requirements for telling enemy looks apart |
| `ui-hud.md` | Fonts, color scheme, subtitle bar |
| `player-combat.md` | Audio-visual feedback for getting hit, dodging and low health |
| `loot-inventory-heat.md` | Audio-visual feedback for supply points, relics and heat tiers |

## Formulas

**Build size budget**

```
BuildSize = engine runtime (about 300-450 MB) + cooked content <= 1 GB
```

Cooked content budget <= 450 MB: scenery <= 250 MB, characters and monsters <= 100 MB, audio <= 60 MB, UI and other <= 40 MB. For each asset pack brought in, migrate only the assets actually used (use Migrate); never import a whole pack.

**On-screen rendering budget**

```
150 enemies × triangles per enemy <= 1.5 M triangles
```

That is, each regular enemy <= 10 k triangles (Nanite may be used for static scenery; skeletal mesh enemies follow this budget).

## Edge Cases

| Situation | Handling |
|---|---|
| The first-choice asset pack is unavailable or its license does not allow use | Switch to the fallback right away; if neither works, keep the graybox shapes and change only materials and colors |
| Styles are clearly inconsistent after mixing assets | First use post-processing and unified material parameters to bring them in line, then replace the most jarring one |
| Build size exceeds 1 GB | In order: compress textures to 1K, delete unused assets, lower the audio bitrate |
| Enemy models are too detailed and cause frame drops | Switch to a low-poly model or force the lowest LOD |

## Dependencies

- Upstream: `map-level` (area layout), `enemies-spawning` (enemy types), `ui-hud` (interface).
- Downstream: none.

## Tuning Knobs

| Parameter | Default | Safe range | Affects |
|---|---|---|---|
| Film grain intensity | 0.2 | 0-0.5 | Cinematic feel vs. clarity |
| Vignette intensity | 0.3 | 0-0.6 | Focus on the center |
| Ambient light intensity | Low | - | Night feel and readability |
| Default music / SFX volume | 0.6 / 0.8 | 0-1 | Mix balance |

## Visual/Audio Requirements

This document itself.

## Game Feel

The "cinematic feel" of the picture comes from light and post-processing, not from model detail. Gameplay readability always comes first: any art replacement that makes it hard for the player to see enemies or supply points is rolled back.

## UI Requirements

See the font and color rules in `ui-hud.md`.

## Cross-References

- `design/prd/master-prd.md` (global constraints: 1 GB, no trademarks, external assets listed)
- `design/gdd/map-level.md` (areas)
- `design/gdd/ui-hud.md` (fonts, color scheme)

## Acceptance Criteria

- **AC1 (Must)**: In the graybox phase, one screenshot lets you tell apart the player, enemies, supply points (including type) and extraction points (including open state).
- **AC2 (Should)**: After replacement, each of the 5 areas has a screenshot proving its main color and landmark are recognizable, saved in `production/qa/evidence/`.
- **AC3 (Should)**: The external asset list names every external pack used, including name, source, license and use.
- **AC4 (Should)**: Packaged build size < 1 GB.
- **AC5 (Should)**: All Should-tier sound effects in the table above are hooked up, and each event plays its sound.
- **AC6 (Must)**: With 150 enemies on screen, frame rate >= 60 FPS (test again after the art replacement).

## Open Questions

- Whether the player character is a Paragon hero or an Infinity Blade warrior depends on the Day 2 availability check and on whether the animations are complete (run, dodge, hit reaction and death are needed).
