<div align="center">

# Cut Survivors

### 2026 Epic AI MegaJam entry (theme: Director's Cut)

> A solo Unreal Engine 5.8 entry for the 2026 Epic AI MegaJam (theme: Director's Cut), built in four days with Claude Code and a team of game-studio agents: a fantasy top-down horde-survival game with extraction, where every level-up is an editing decision.

[![GitHub Repo](https://img.shields.io/badge/GitHub-181717?style=for-the-badge&logo=github&logoColor=white)](https://github.com/Huangyu-Shen/2026-epic-gamejam)
[![Unreal Engine](https://img.shields.io/badge/Unreal%20Engine-5.8-313131?style=for-the-badge&logo=unrealengine&logoColor=white)](https://www.unrealengine.com/)

<br />

[Features](#-features) · [Controls](#-controls) · [Quick Start](#-quick-start) · [Usage](#-usage) · [License](#-license)

</div>

---

## ✨ Features

- **Pick one, or cut one first.** Each level-up offers three options. You take one, and you may first cut one of the three: it leaves your pool for the run and goes to the boss. Options you neither take nor cut stay in the pool. Three copies of an option merge into a 2-star version, nine into a 3-star one. The pool never shrinks below four options.
- **The boss takes what you cut.** With the Cut Knight film card slotted, a boss starts the run and gains each option you cut: a cut weapon becomes an attack aimed at you, a cut passive raises one of its stats.
- **Film cards.** Before a run, slot film cards that force a horde or a boss in exchange for a bigger payout.
- **Heat and extraction.** Rare loot in the bag raises heat, which makes enemies stronger. Reach the extraction zone alive to bank what you carry; die and you lose the bag.
- **Lost film reels.** Carry a reel out alive and a new option joins the level-up pool for every later run.
- **Camp.** Wood and stone from successful runs upgrade the Smithy (weapon damage) and the Clinic (max health); the Warehouse stores your unlocked options. Progress is auto-saved.
- **Built with AI.** Claude Code drives the editor through the Unreal MCP plugin and organises design and implementation with Claude Code Game Studios agents and skills.

Design and status: [design/game-brief.md](design/game-brief.md) (v2 brief), [design/prd/master-prd.md](design/prd/master-prd.md) (scope gate). Requirements plan: [docs/plans/2026-11-12-1143-feat-directors-cut-extraction-survivors-plan.md](docs/plans/2026-11-12-1143-feat-directors-cut-extraction-survivors-plan.md). Jam schedule: [PLAN.md](PLAN.md). Code overview: [docs/wiki/README.md](docs/wiki/README.md).

**Dev log (public):** <https://github.com/Huangyu-Shen/2026-epic-gamejam/blob/main/docs/devlog/README.md>, one folder per day under [docs/devlog/](docs/devlog/README.md).

> Some per-system design documents in `design/gdd/` still describe the first version of the design (keep-or-use-it relics, four camp buildings). The brief and the master PRD are current.

## 🎮 Controls

WASD move · Space dodge · E search a container · 1–3 take an upgrade · 4–6 cut one first · Esc menu (resume, volume, return to camp, quit).

## 🚀 Quick Start

```bash
git clone https://github.com/Huangyu-Shen/2026-epic-gamejam.git
cd 2026-epic-gamejam
git lfs pull
```

The Unreal project is in `Game_01 5.8/` (a C++ project; open `Game_01 5.8/Game_01.uproject` with Unreal Engine 5.8). `Game_Jam_01/` was a pre-jam warm-up project and is not in the repository.

## 📦 Installation

- Unreal Engine 5.8
- Git and Git LFS (binary assets such as `.uasset` and `.umap` go through LFS)
- Visual Studio with the "Game development with C++" workload
- [Claude Code](https://claude.com/claude-code) and the Unreal MCP plugin (turn on Auto Start Server in the editor; the address is in `.mcp.json`)
- To run the hooks in `.claude/` on Windows you need Git Bash, Python and jq

## 📖 Usage

- Start Claude Code from the repository root so that `CLAUDE.md`, the hooks and the permission rules load.
- Project configuration is in `project.yaml` (engine, modes, naming rules).
- Plans and requirements are in `docs/plans/`; engine API references are in `docs/engine-reference/unreal/`.
- Commits before the `jam-start` tag contain only tooling, configuration and scripts. Game content starts after that tag.
- Design documents: the master PRD is `design/prd/master-prd.md`; per-system documents are in `design/gdd/`.

## 🛠 Tech Stack

| Layer | Choice |
|-------|--------|
| Engine | Unreal Engine 5.8 |
| Language | C++ first, Blueprint for data and presentation |
| AI tools | Claude Code + Unreal MCP |
| Workflow | Claude Code Game Studios (agents, skills, hooks) |
| Version control | Git + Git LFS, hosted on GitHub |

## 📄 License

No license has been specified yet.
