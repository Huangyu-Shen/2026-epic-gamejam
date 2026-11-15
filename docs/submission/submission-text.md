# Submission text (draft, for the author to paste)

Prepared 2026-11-15 from `docs/devlog/README.md`, `README.md` and the stories. Claude does not
upload or submit. Check every line against the final build before pasting; items marked
**CHECK** depend on a check that has not been done yet.

## Title

Cut Survivors

## Short description

A fantasy top-down horde-survival game with extraction, built solo in four days in Unreal
Engine 5.8. Every level-up is an editing decision: you are offered three options, you take one,
and you may first cut one, which goes to the boss. Slot film cards before a run to make it
harder for a bigger payout, loot the map, and reach the extraction portal alive to bank what you
carry. Carry a lost film reel out alive and a new option joins the pool for every later run.

## How it fits the theme ("Director's Cut")

You edit your own run. Cutting an option removes it from your pool and hands it to the boss;
film cards edit the run before it starts; reels you rescue edit what future runs can offer.

## Controls

WASD move · Space dodge · E search a container · 1–3 take an upgrade · 4–6 cut one first · Esc menu

## Tools used

- Unreal Engine 5.8.4, C++ gameplay code. Rider to read the code.
- Claude Code (desktop app) with Claude Opus 5.5, Claude Sonnet 5.5 and Claude Fable 5.2,
  connected to the editor through the Unreal MCP plugin.
- Claude Code Game Studios, an open-source template of subagents and skills by Donchitos
  (<https://github.com/Donchitos/Claude-Code-Game-Studios>). Not written by the author; configured
  and used by the author.
- Sub-agents inside Claude Code for parallel translation and documentation work.
- Unreal's Python scripting plugin, from the command line, to create data tables.
- Git and GitHub.

No AI 3D, texture or audio generator was used.

## External assets and licences

All from Fab. Licences are as shown on each Fab page; the author read the pages, not the full
licence texts.

| Pack | Author | Licence on its Fab page | Used for |
|---|---|---|---|
| Path of Adventure - Free Music Pack for JRPG | Kounine | CC-BY (credit required) | Run music and camp music |
| 50 Free Game Sounds Pack | PlaceHolder Inc | Fab standard licence, personal tier (free) | The sound effects |
| RPG Hero Squad PBR Polyart | Dungeon Mason | Free; Unreal format only | The hero, sword, shield and animations |
| Monster for Survival Game PBR Polyart | Dungeon Mason | Free; Unreal format only | Grunt enemies and the boss |
| RPG Tiny Fantasy Forest | Dungeon Mason | Free; Unreal format only | Floor, trees, rocks, containers, extraction portal |
| Cozy Low Poly House | Demolition Studios | Free; Unreal format only | The clinic in the camp |
| WW2 RTS & SLG Production Buildings Pack | Winyunq | CC BY 4.0 | The smithy and the warehouse in the camp |

Imported and then deleted on 15 November, never used in the game, so they are not in the build:
Atmospheric Worlds (Marika Schanz), Small Sound Kit (Sound Works 12), Battle Wizard and Four Evil
Dragons (Dungeon Mason), ZenUV Modular Sci-Fi Kit (Sergey Tyapkin), 5 Free Stylized Sci-Fi
Skyscrapers (Meanopoly).

## Credits (also shown in the game, at the bottom of the Esc menu)

- Music by Kounine, "Path of Adventure - Free Music Pack for JRPG", Creative Commons CC-BY, via
  Fab.
- Buildings by Winyunq, "WW2 RTS & SLG Production Buildings Pack", licence
  <https://creativecommons.org/licenses/by/4.0/>, via Fab. Changes: only two of the buildings are
  used, as static meshes, uniformly scaled.

## Work made before the jam (declaration)

Allowed preparation, in the repository: the Claude Code Game Studios agents and skills and their
configuration, the Unreal MCP setup, curated Unreal Engine 5.8 reference notes, and a jam
schedule (`PLAN.md`). The first commit (`44589d2`) contains only this. A throwaway warm-up
project existed to test the tool chain; it is not in the repository and nothing from it was
copied. The game project was created after the jam started.

## Links

- Repository: <https://github.com/Huangyu-Shen/2026-epic-gamejam>
- Dev log: <https://github.com/Huangyu-Shen/2026-epic-gamejam/blob/main/docs/devlog/README.md>
  (**CHECK**: open it logged out and make sure the images load)
- Gameplay video: <https://youtu.be/G_sneXNheig> (30 to 60 seconds: a cut, a star-up, an extraction)
- Build (zip): `CutSurvivors.zip`, 433.7 MB (454,813,988 bytes), commit `f88cea3`, SHA-256 `803315084F708883A95C8F4103664C5D391C965D20B97B7B260111E38AA7767B`

## Known limits (be honest in the form if it asks)

- The final package builds, both maps load, the offer system works in a run, and the zip
  unpacks and starts in a fresh folder; the full camp → run → camp manual walkthrough is pending.
- The frame-rate figure (about 130 FPS average with about 150 enemies) was measured with weapons
  switched off, before most of the art.
- Whether a build that has used all its cuts can still beat the boss has not been checked.
- The game has been played mostly by its author.
