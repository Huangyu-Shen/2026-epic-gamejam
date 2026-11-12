<div align="center">

# 2026 Epic AI MegaJam

> 单人参加 2026 Epic AI MegaJam 的 Unreal Engine 项目：主题 Director's Cut，用 Claude Code 和一组游戏工作室 agent 在 4 天内做一款奇幻割草搜打撤游戏。

[![GitHub Repo](https://img.shields.io/badge/GitHub-181717?style=for-the-badge&logo=github&logoColor=white)](https://github.com/Huangyu-Shen/2026-epic-gamejam)
[![Unreal Engine](https://img.shields.io/badge/Unreal%20Engine-5.8-313131?style=for-the-badge&logo=unrealengine&logoColor=white)](https://www.unrealengine.com/)

<br />

[Features](#-features) · [Quick Start](#-quick-start) · [Usage](#-usage) · [License](#-license)

</div>

---

## ✨ Features

- **Director's Cut 割草搜打撤**：西方奇幻、3D 斜俯视、自动攻击。出发前把片段卡插进胶片槽，决定这一局的精英怪、尸潮、Boss、撤离点和时长。
- **留着还是用掉**：稀有物资可以当场用来扭转局势，但背包越值钱，热度越高，怪就越多越强。
- **局外营地**：带出的通用材料用来升级四栋建筑。剪辑室负责"恢复被剪掉的片段"，解锁新卡和新武器。
- **AI 协作开发**：Claude Code 通过 Unreal MCP 连接编辑器，用 Claude Code Game Studios 的 agent 和 skill 组织设计与实现。

需求详见 [docs/plans/2026-11-12-1143-feat-directors-cut-extraction-survivors-plan.md](docs/plans/2026-11-12-1143-feat-directors-cut-extraction-survivors-plan.md)。比赛时间线见 [PLAN.md](PLAN.md)。

## 🚀 Quick Start

```bash
git clone https://github.com/Huangyu-Shen/2026-epic-gamejam.git
cd 2026-epic-gamejam
git lfs pull
```

UE 项目在 `Game_01/`（C++ 工程，用 UE 5.8 打开 `Game_01/Game_01.uproject`）。`Game_Jam_01/` 是赛前热身项目，不在仓库里。

## 📦 Installation

- Unreal Engine 5.8
- Git 和 Git LFS（`.uasset`、`.umap` 等二进制资产走 LFS）
- Visual Studio，带 C++ 游戏开发工作负载
- [Claude Code](https://claude.com/claude-code)，以及 Unreal MCP 插件（编辑器内开启 Auto Start Server；地址见 `.mcp.json`）
- 在 Windows 上运行 `.claude/` 里的 hooks，需要 Git Bash、Python 和 jq

## 📖 Usage

- 在仓库根目录启动 Claude Code，才能加载 `CLAUDE.md`、hooks 和权限规则。
- 项目配置在 `project.yaml`，包括引擎、模式和命名规范。
- 设计和需求计划放在 `docs/plans/`，引擎 API 参考放在 `docs/engine-reference/unreal/`。
- 在 `jam-start` 标签之前的提交只有工具、配置和脚本。游戏内容从这个标签之后开始。
- 设计文档：总 PRD 在 `design/prd/master-prd.md`，各系统的分 PRD 在 `design/gdd/`。

## 🛠 Tech Stack

| 层 | 选择 |
|-------|--------|
| 引擎 | Unreal Engine 5.8 |
| 语言 | C++ 为主，Blueprint 为辅 |
| AI 工具 | Claude Code + Unreal MCP |
| 工作流 | Claude Code Game Studios（agent、skill、hooks） |
| 版本控制 | Git + Git LFS，托管在 GitHub |

## 📄 License

暂未指定许可证。
