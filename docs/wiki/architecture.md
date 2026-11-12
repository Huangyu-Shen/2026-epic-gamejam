# 整体架构

## 1. 仓库的四层模型

仓库并非一个纯代码工程，而是「**游戏代码** + **设计文档** + **AI 协作工具链** + **生产管理**」四层并存的仓库。理解它要分四层看：

```
┌────────────────────────────────────────────────────────────┐
│ L1 游戏代码层   Game_01 5.8/  (Unreal Engine 5.8, C++)      │
│   真正参与构建、打包成游戏的代码与资产                          │
├────────────────────────────────────────────────────────────┤
│ L2 设计文档层   design/ + docs/plans/ + docs/engine-reference/│
│   需求源头：总 PRD → 各系统 GDD → 引擎 API 参考                │
├────────────────────────────────────────────────────────────┤
│ L3 AI 工具链层  .claude/ + CLAUDE.md + project.yaml + .mcp.json│
│   Claude Code Game Studios(CCGS) 的 agent/skill/hook/脚本     │
├────────────────────────────────────────────────────────────┤
│ L4 生产管理层   production/  (epic/story/QA/会话状态)         │
│   史诗、故事、验收证据、会话检查点                             │
└────────────────────────────────────────────────────────────┘
```

四层之间的因果方向是 **自上而下**：设计（L2）决定要做什么 → 工具链（L3）编排 agent 去实现 → 代码（L1）落地 → 生产管理（L4）记录进度与验收。

## 2. 目录结构（顶层）

| 路径 | 层 | 职责 |
|---|---|---|
| `Game_01 5.8/` | L1 | UE 工程：`Source/`（C++ 源码）、`Content/`（资产）、`Config/`（配置）。目录名带空格是因为它是 5.8 引擎的重新关联副本，见 [build-and-run.md](build-and-run.md) 第 2 节 |
| `Game_Jam_01/` | — | 赛前热身项目，gitignored，**永不提交/复制** |
| `design/` | L2 | 游戏设计：`prd/master-prd.md`（范围闸门）+ `gdd/`（11 个系统 GDD）+ `game-brief.md` |
| `docs/` | L2 | `plans/`（需求计划）、`engine-reference/`（引擎 API 快照）、迁移/协作指南 |
| `.claude/` | L3 | CCGS 工具链：`agents/`、`skills/`、`hooks/`、`rules/`、`scripts/`、`docs/` |
| `production/` | L4 | `epics/`、`qa/`、`session-state/`、`session-logs/` |
| `CLAUDE.md` | L3 | 主配置：技术栈、协作协议、工作风格、编码规范（会话自动加载） |
| `project.yaml` | L3 | 机器可读配置：引擎、专家 agent、命名规范、命令、模式（配置唯一事实源） |
| `.mcp.json` | L3 | Unreal MCP 连接配置（`http://127.0.0.1:8000/mcp`） |
| `PLAN.md` | L2 | 比赛时间线与阶段计划（Phase 0–5） |

## 3. 游戏代码层的系统架构

游戏逻辑围绕 **UE Subsystem（子系统）+ Actor + ActorComponent** 组织，遵循「少 Actor 各自 Tick，多集中在子系统统一驱动」的性能导向设计：

### 3.1 三类运行时对象

| 类型 | 代表 | 作用 |
|---|---|---|
| **WorldSubsystem / TickableWorldSubsystem** | `UEnemyManagerSubsystem`、`UEnemySpawnSubsystem`、`ULootHeatSubsystem`、`URunOutcomeSubsystem` | 全局单例、跨对象协调、集中 Tick（通过 `GetWorld()->GetSubsystem<T>()` 访问） |
| **GameInstanceSubsystem** | `UCampStockSubsystem` | 跨关卡/跨局持续存在的全局状态（营地物资） |
| **Actor / ActorComponent** | `AHeroCharacter`、`AEnemy`、`ALootContainer`、`AExtractionZone` + `UHealthComponent`、`UAutoWeaponComponent`、`ULootBagComponent` | 场景中的实体与其能力组件 |

### 3.2 核心运行时数据流

```
[刷怪]  UEnemySpawnSubsystem  --SpawnHorde/SpawnBoss--> AEnemy
               │ 读取热度压力(GetSpawnPressureMultiplier)
               ▼
[热度]  ULootHeatSubsystem  ◄── 拾取遗物升温 / 使用遗物降温 ── ULootBagComponent
               │
               ▼
[敌人]  AEnemy --Register--> UEnemyManagerSubsystem（每 Tick 追击/推开/接触伤害）
               │ 接触伤害 → AHeroCharacter.ReceiveEnemyDamage → UHealthComponent.ApplyDamage
               ▼
[死亡]  UHealthComponent.OnDied → AHeroCharacter.HandleDied → URunOutcomeSubsystem.ReportPlayerDied
[撤离]  AExtractionZone 读条满 → URunOutcomeSubsystem.ReportExtracted
               │
               ▼
[结算]  FRunSettlement::Compute → UCampStockSubsystem.AddIron/AddRelics
               │ 清空背包(ClearBag)、重置热度(ResetHeat)、广播 OnRunEnded
               ▼
[下一局] URunOutcomeSubsystem.StartNewRun / AHeroCharacter.StartNewRunKey 复位全部状态
```

## 4. 设计文档层架构

- **范围闸门**：`design/prd/master-prd.md` 是唯一范围裁决者；`game-brief.md` 是它的一页摘要，冲突以 PRD 为准。
- **系统拆分**：`design/gdd/*.md` 共 11 个系统 GDD（营地、剪辑、敌人刷怪、撤离死亡、游戏流程、战利品热度、地图关卡、玩家战斗、局内成长、UI、美术音频）。
- **来源计划**：`docs/plans/2026-11-12-1143-feat-directors-cut-extraction-survivors-plan.md`（含便利贴与 Must/Should/Could 三档）。
- **引擎参考**：`docs/engine-reference/unreal/` 是版本锁定的 UE API 快照（VERSION.md + modules + plugins），供 skill 按需读取。

## 5. AI 工具链层架构（CCGS）

`.claude/` 是「Claude Code Game Studios」框架（来源 `Donchitos/Claude-Code-Game-Studios`，框架 1.2.0）的副本，提供一套游戏工作室工作流：

- **agents（49 个）**：按角色分工的子代理（game-designer、gameplay-programmer、unreal-specialist、ue-gas-specialist 等），由 `project.yaml` 的 `specialists` 选择启用集合。
- **skills（74 个斜杠命令）**：按阶段组织的工作流命令（`/start`、`/brainstorm`、`/create-stories`、`/dev-story`、`/story-done` 等）。
- **hooks（18 个 shell 脚本）**：会话/提交/写入的钩子，用于校验、日志、防呆（需 Git Bash + Python + jq）。
- **rules（14 个）**：领域编码规则（engine-code、gameplay-code、scene-files、data-files、skill-authoring 等）。
- **scripts（38 个）**：观察型辅助脚本（**只输出事实，不输出判决**——见 CLAUDE.md 约定）。
- **docs**：框架自身文档（coordination-rules、coding-standards、skills-reference、director-gates、templates 等）。

## 6. 生产管理层架构

- `production/epics/directors-cut/`：1 个 EPIC.md + 7 个 story（story-001 ~ story-007），对应 MVP 的 7 个特性。
- `production/qa/evidence/`：每个 story 的验收截图证据 + `stamp.txt`。
- `production/session-state/active.md`：**会话检查点**——"文件即记忆"，新会话/压缩/崩溃后先读它。
- `production/session-logs/`：会话审计日志（gitignored）。

## 7. 关键设计原则

1. **范围纪律**：只在 `master-prd.md` 的 Must 档内工作，Should/Could 档在 Must 跑通前不开工。
2. **C++ 优先**：玩法逻辑走 C++，Blueprint 只做数据与表现（减少 AI 改 Blueprint 节点图的不稳定性）。
3. **灰盒优先**：先用引擎方块/胶囊保证可读性，美术/音频最后替换。
4. **性能导向**：敌人本身不 Tick，所有敌人移动/碰撞集中在 `UEnemyManagerSubsystem` 一个 Tick 内（复用 scratch buffer 避免分配）。
5. **数据驱动**：战利品/敌人参数通过 `UDataAsset`（`ULootConfig`）+ `UDataTable` 配置，无内容也能跑（内联默认值）。
6. **验证优先**：CLAUDE.md「Working Style」要求"先验证再声称完成"（build/运行/读输出）。
