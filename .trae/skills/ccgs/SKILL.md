---
name: ccgs
description: Claude Code Game Studios 工作流的 TRAE 入口，索引 .claude/skills 下 74 个游戏开发流程并按步骤执行。当用户提到 story、epic、sprint、GDD、ADR、QA、代码评审、里程碑、发布检查，或要按工作室流程开发/评审游戏时使用。不用于与工作室流程无关的临时编码任务。
---

# CCGS — Claude Code Game Studios 工作流入口

本仓库把 CCGS 整套工具链以「项目模板」形式内置在 `.claude/` 下，**不需要安装**。
这个技能是 TRAE 侧的入口：把「用户想做哪一步」映射到具体流程文件，并按文件里的步骤执行。

## 目录职责

| 位置 | 内容 |
|---|---|
| `.claude/skills/<name>/SKILL.md` | 74 个流程定义，**真正要读的东西** |
| `.claude/agents/` | 49 个角色定义（编码前先读对应角色的约束） |
| `.claude/rules/` | 编码/文档规则 |
| `.claude/hooks/` | 预处理脚本（`yaml-helper.sh` 等） |
| `.claude/scripts/` | 状态查询脚本（`story-status.sh`、`artifact-check.sh` 等） |
| `.claude/docs/` | 模板与参考（`workflow-catalog.yaml` 是阶段权威表） |
| `CLAUDE.md` | 主配置，已被 TRAE 规则自动导入 |
| `project.yaml` | 机器可读配置（唯一事实源） |

## 执行协议

1. **先定位**：从下表找到对应流程，`Read .claude/skills/<name>/SKILL.md`。
2. **照做**：该文件就是操作手册，按它的 Step 顺序执行，不要凭印象简化。
3. **补执行预取命令**：`SKILL.md` 里的 `` !`bash ...` `` 是 Claude Code 的预执行语法，TRAE 不会自动跑。
   看到它就用 Git Bash 手工执行同样的命令，再把结果当作「已解析的值」使用。
4. **忽略 Claude Code 专属 frontmatter**：`allowed-tools`、`model`、`argument-hint`、`user-invocable` 不生效，也无需模拟。

### 命令执行方式

`bash` 不在 PATH，必须用 Git Bash 的绝对路径：

```powershell
& "D:\Git\Git\bin\bash.exe" -lc 'cd "/d/project/2026-epic-ai-megajam" && bash .claude/hooks/yaml-helper.sh resolve_config --keys project.stage,workflow'
```

已验证可用，输出示例：

```
workflow: minimal (rigor:minimal)
project.stage: Systems Design (project.yaml)
```

## 流程索引

| 流程 | 用途 |
|---|---|
| `start` | 首次上手，挑选起步工作流 |
| `help` | 卡住了 / 不知道下一步做什么 |
| `project-stage-detect` | 判定项目所处阶段，找缺口 |
| `onboard` | 给新加入的 agent/人生成上手指南 |
| `brainstorm` | 从零找游戏概念、换方向 |
| `prototype` | GDD 之前的可丢弃原型，产出 PROCEED/PIVOT/KILL |
| `setup-engine` | 配置并锁定引擎版本 |
| `settings` | 查看/修改项目配置 |
| `map-systems` | 概念拆成系统，排依赖与设计顺序 |
| `art-bible` | 视觉识别基线（在 map-systems 之前） |
| `design-system` | 逐节撰写某个系统的 GDD |
| `quick-design` | 小改动轻量规格，直接嵌进 story |
| `review-all-gdds` | 跨 GDD 一致性总审 |
| `design-review` | 单份设计文档完备性/可实现性评审 |
| `ux-design` | 逐节撰写界面/流程/HUD 的 UX 规格 |
| `ux-review` | UX 规格与实现评审 |
| `consistency-check` | GDD 与实体注册表的跨文档冲突扫描 |
| `content-audit` | GDD 规划量 vs 已实现量 |
| `create-architecture` | 写代码前的架构蓝图 |
| `architecture-decision` | 产出 ADR（上下文/备选/后果） |
| `architecture-review` | GDD 需求 → ADR 追溯矩阵 |
| `create-control-manifest` | 从已接受 ADR 提炼「必须做/禁止做」速查表 |
| `create-epics` | GDD + 架构 → epic（按模块划分） |
| `create-stories` | 一个 epic → 可实现的 story + 验收标准 |
| `story-readiness` | story 是否具备开工条件 |
| `dev-story` | **实现一个 story**（带测试，遵循其 ADR） |
| `story-done` | 关闭 story：逐条核验验收标准 |
| `estimate` | 任务工作量估算 |
| `sprint-plan` | 排/更新 sprint |
| `sprint-status` | sprint 快照与风险 |
| `vertical-slice` | 投产前的端到端验证切片 |
| `gate-check` | 阶段推进闸门 PASS/CONCERNS/FAIL |
| `milestone-review` | 里程碑评审，go/no-go |
| `retrospective` | sprint/里程碑复盘 |
| `scope-check` | 范围蔓延检查 |
| `balance-check` | 数值/经济/成长曲线失衡检查 |
| `perf-profile` | 性能剖析与优化建议排序 |
| `playtest-report` | 试玩报告（模板或结构化现有记录） |
| `qa-plan` | 一个 sprint 的测试计划 |
| `smoke-check` | QA 交接前的冒烟闸门 |
| `soak-test` | 长时间运行稳定性协议 |
| `regression-suite` | 测试覆盖 ↔ 关键路径映射 |
| `test-setup` | 搭测试框架与 CI（首个 sprint 前一次） |
| `test-helpers` | 生成引擎相关的测试辅助库 |
| `test-evidence-review` | 测试证据质量评审 |
| `test-flakiness` | 从 CI 日志找不稳定测试 |
| `code-review` | 架构级代码评审 |
| `tech-debt` | 技术债登记与排序 |
| `bug-report` | 结构化缺陷报告 / 代码隐患分析 |
| `bug-triage` | 存量缺陷重排优先级 |
| `hotfix` | 紧急修复通道 |
| `day-one-patch` | 首发补丁 |
| `release-checklist` | 发布前检查清单 |
| `launch-checklist` | 全部门上线就绪评审 |
| `changelog` | 从提交历史生成变更日志 |
| `patch-notes` | 面向玩家的补丁说明 |
| `security-audit` | 存档篡改/作弊/网络/数据暴露审计 |
| `localize` | 本地化流程 |
| `asset-spec` | 逐资产视觉规格 + AI 生成提示词 |
| `asset-audit` | 资产命名/体积/格式审计 |
| `adopt` | 存量项目合规审计与迁移计划 |
| `reverse-document` | 从既有实现倒推设计/架构文档 |
| `propagate-design-change` | GDD 改动后找出失效的 ADR |
| `team-combat` | 战斗组编排（设计→实现→验证） |
| `team-level` | 关卡组编排（完整区域制作） |
| `team-narrative` | 叙事组编排 |
| `team-ui` | UI 组编排（走 UX 管线） |
| `team-audio` | 音频组编排 |
| `team-live-ops` | 长线运营组编排 |
| `team-polish` | 打磨组编排（性能/表现/手感） |
| `team-qa` | QA 组完整测试周期 |
| `team-release` | 发布组执行发版 |
| `skill-test` | 校验技能文件结构 |
| `skill-improve` | 技能测试-修复-复测循环 |

## 与 TRAE 的衔接

- `CLAUDE.md` 已由 TRAE 规则导入（`AI.rules.importClaudeMd`），协作协议与编码规范自动生效。
- Unreal 侧能力走 MCP `unreal-mcp`（`list_toolsets` / `describe_toolset` / `call_tool`），可直接读写编辑器。
- 本仓库当前阶段：`Systems Design`，工作流档位 `minimal`。
- 活跃 epic 在 `production/epics/`，会话检查点在 `production/session-state/active.md`。
