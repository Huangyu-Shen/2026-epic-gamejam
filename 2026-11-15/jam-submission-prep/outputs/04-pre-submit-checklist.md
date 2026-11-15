# 提交前自检清单（打印/复制到 `production/qa/smoke-2026-11-15.md`）

> 用法：每做完一项打勾并写证据（截图名、日志行、数字）。**没证据就等于没做。**
> 这份清单合并了 story 013 的验收标准、**Official Rules PDF 原文**（19 页，最后更新 2026-11-12）、**itch.io jam 页面原文**（2026-11-15 抓取）和打包/上传的实际检查项。标注 `规则 §x` 的条目都能在 `dist\official-rules.txt` 里查到原句。
> 自动检查先跑：`scripts\preflight.ps1`（只读，不通过就别往下走）。

---

## 0. 记录栏（提交时用）

| 字段 | 值 |
|---|---|
| 构建 commit | `________________` |
| 分支 | `________________` |
| 包目录 | `Saved\Packages\________________` |
| 构建配置 | Development（官方要求）/ Shipping（可选） |
| 打包时间 | `____-__-__ __:__` |
| 整包大小 | `______ MB` |
| 去 pdb 后 | `______ MB` |
| 队名（必填） | `Knight On Night`（表单） / `KnightOnNight`（文件名） |
| Epic 账号邮箱（必填） | `________________` |
| zip 文件名 | `KnightOnNight_DirectorsCut.zip`（必须 `TeamName_GameName`） |
| zip 大小 | `______ MB`（上限 1000） |
| zip SHA-256 | `________________` |
| 视频链接 | `________________` |
| 视频时长 / 分辨率 | `____ s / ____×____` |
| itch.io 上传完成 | `____-__-__ __:__` |
| jam 提交完成 | `____-__-__ __:__` |
| 截止时间（北京） | 2026-11-16 09:59（页面：11/15 5:59 PM PST） |

---

## 1. 冻结与仓库

- [ ] 功能冻结：不再改玩法；只修 crash/阻断性 bug
- [ ] `git status` 无未提交的**源文件**改动（Fab 资源包未跟踪是预期的）
- [ ] 构建用的就是记录栏里的 commit（`git rev-parse --short HEAD` 对得上）
- [ ] 没有把 `Saved/`、`*.pdb`、zip、视频提交进 Git
- [ ] `.uasset/.umap` 走 LFS（`git lfs ls-files | wc -l` 有数量）
- [ ] 仓库是 public，README 与最终游戏一致

## 2. 打包

- [ ] 编辑器/Live Coding 已关闭
- [ ] `DefaultEngine.ini` 无 `[Zen] AutoLaunch=False`
- [ ] `DefaultGame.ini` 含 `L_Camp` + `L_Greybox` 两条 `MapsToCook`
- [ ] `DefaultGame.ini` 含 `/Game/Data`、`Free_Sounds_Pack/cue`、`PathOfAdventure/Loop/CUE` 三条 `DirectoriesToAlwaysCook`
- [ ] Development 包构建成功（页面原文要求 "development build"，也是本项目唯一验证过的配置）
- [ ] 如另打了 Shipping：两包都能进营地、都能玩（可选，不做也合规）
- [ ] 包内 `Windows\Game_01.exe` 存在
- [ ] 日志中**没有** `LOOTCFG source=c++ defaults`
- [ ] 日志中**没有** cook 阶段 "Failed to delete oplog" / "Failed to load map"

## 3. 包内验收（每项都要亲手做）

- [ ] 删掉包内 `Saved\`，双击启动 → 营地正常显示
- [ ] 营地 → START RUN → 打完/撤离 → 回营地，**连续两轮**不崩
- [ ] 有存档时启动正常（不新建存档也不报错）
- [ ] 两张地图都加载了（营地 + `L_Greybox`）
- [ ] 数值不是默认值（升级有变化、掉落在合理范围）
- [ ] 有声音：攻击、命中、拾取、升级、撤离（戴耳机听一遍）
- [ ] 文字全英文，无中文残留
- [ ] 没有调试文字/探针输出/灰盒替身出现在正常游玩画面
- [ ] `K` 键等调试键在提交包里无反应（`DebugGate` 在非编辑器下应恒关）
- [ ] 分辨率 1920×1080 下 HUD 不重叠、不超出屏幕
- [ ] 帧率：150 敌人时 ≥60 FPS（按 PRD 第 5 节；开武器实测）

## 4. zip 与换机测试

- [ ] zip 文件名是 `KnightOnNight_DirectorsCut.zip`（`TeamName_GameName` 结构；表单里的队名是全名 `Knight On Night`）
- [ ] zip 根目录就是 `Game_01.exe`（不是多套一层 `Windows\`）
- [ ] zip 内无 `*.pdb`、无 `Saved\`
- [ ] zip ≤ 1 GB，数字已记录
- [ ] 解压到**新文件夹**（最好换一台机器或另一个 Windows 账户）能启动
- [ ] 在新位置的存档路径正常（`<解压目录>\Game_01\Saved\SaveGames\`）
- [ ] SmartScreen 提示时的说明已想好（"更多信息 → 仍要运行"）

## 5. 视频

- [ ] 时长 30–60 秒（规则 §1(e) 硬性）
- [ ] 分辨率 ≥1920×1080（规则 §7(c) 硬性：`Resolution of the Video must match 1920x1080 or higher`）
- [ ] 前 5 秒就是 gameplay（官方 Tips："Show gameplay in the first five seconds"）
- [ ] 画面里有：一次**剪**、一次**升星**、一次**撤离**
- [ ] 全英文（字幕、标题卡、解说）——规则 §7(i) 硬性要求 `All Submission content (including in-game text, audio and the Video) ... must be in English`
- [ ] 没有调试信息、没有中文窗口、没有版权外链音乐
- [ ] 有字幕（至少 SRT 随视频上传），静音看也能懂
- [ ] 已上传，链接可匿名访问（无痕窗口打开一次）
- [ ] 链接已填进 `docs/submission/submission-text.md`

## 6. 文案与文档

- [ ] `docs/submission/submission-text.md` 无 `TO ADD` / `CHECK` 残留
- [ ] 工具清单与实际一致（UE 5.8.x 具体版本、模型名）
- [ ] 外部素材表与最终包一致（用到的都在，删掉的没写进来）
- [ ] CC-BY 署名在游戏内 Esc 菜单可见
- [ ] Devlog 有 Day 3、Day 4 条目，数字是最新的
- [ ] Devlog 链接**退出登录**能打开且图片能加载
- [ ] 提交前声明的"赛前准备"部分与实际仓库历史一致

## 7. 奖项专项材料（详见 [05-awards-and-student-track.md](05-awards-and-student-track.md)）

**Glass Box（$2,500，必做）**

- [ ] devlog 长文已发布，英文，1500 词以上
- [ ] 内容覆盖：主题解读、核心机制、四天排期、至少 3 个具体决策（含打包失败这类真实细节）
- [ ] 有截图或 GIF
- [ ] **退出登录 / 无痕窗口**能打开（Glass Box 参评的硬性前提）
- [ ] 文末有 itch 游戏页链接与视频链接
- [ ] 链接已复制到记录栏

**Best Student Project（$7,500，规则未要求任何证明）**

- [ ] itch 项目描述里有 `Student Submission — currently enrolled at [学校]` 一句
- [ ] 一句话确认（05 §3.1）已发出 / 或决定不发：`已发 / 不发`，渠道：`____________________`
- [ ] 学生证（正反面）已备
- [ ] 在读证明（含姓名、学号、学校、当前学期）已备
- [ ] 本学期成绩单（可选）已备
- [ ] 一页英文说明（姓名 / 学校 / 学位阶段 / 预计毕业 / Epic 邮箱 / itch 用户名）已备
- [ ] 全部文件在 `outputs\dist\student-proof\`，PDF，文件名无中文无空格
- [ ] 未把学生证贴到公开页面
- [ ] 明白 §9 互斥：若作品被评 Best Overall，学生奖自动失效，无法选择

**Solo Driver（$2,500，自动参评）**

- [ ] jam 表单里成员只有你一人
- [ ] 没有把 AI 助手、化名、外号写成成员
- [ ] 队名与表单、zip 文件名、itch 页面三处一致

**明确放弃**

- [ ] Pocket Powerhouse：不做移动端构建（`WASD`/`1-6` 在手机上不成立，且移动打包从未验证）
- [ ] Best Team（需 2 人以上）、Walkie-Talkie（仅 UEFN）：规则上不够格
- [ ] Render Royalty：不重做美术，只确保视频调色统一、封面图用最好看的一帧

## 8. itch.io 项目页（自己账号下）

- [ ] 项目已创建，`KnightOnNight_DirectorsCut.zip` 已上传（大小与记录栏一致）
- [ ] 标题、短描述、主题说明、操作说明已填（**全英文，规则 §7(i) 硬性**）
- [ ] 描述里写明 `Student Submission — currently enrolled at [school]`
- [ ] 封面图（630×500）与 3–5 张截图
- [ ] 视频嵌入或链接
- [ ] 页面可见性/定价：免费，允许直接下载
- [ ] 匿名窗口打开页面，确认能下载并解压

## 9. 提交到 jam（页面 + 规则原文的必填项）

> 页面原文：Team name / each member's name + the email linked to their Epic Games account / Game name / Gameplay video link (30–60 s) / Dev log link (optional, but required for Glass Box)。
> **规则 §1(c)**：`include the full name and email address of all Entrants ... identifying one of them as the team owner (the "Team Owner")`。
> **规则 §2**：`Limit one (1) Submission per team.` **规则 §17**：必须由 Entrant 本人提交，`not by an AI agent or other automated means`。

- [ ] 队名：`Knight On Night`（单人也要填，填队名而不是个人名）
- [ ] 成员姓名 + **绑定 Epic 账号的邮箱**（不是常用邮箱）；成员只有你一人
- [ ] 指定 **Team Owner**：你自己（奖金、税表都由这个身份承接）
- [ ] 年龄 18+，Epic 账号有效且不是 Cabined Account
- [ ] 游戏名：Director's Cut
- [ ] 视频链接（30–60 秒，**≥1920×1080**，可匿名访问）
- [ ] Devlog 链接（可选；冲 **Glass Box** 就必填——已填，且必须随作品一起提交）
- [ ] 外部素材与许可清单（规则 §15(c)：`noted upon submission`；AI 生成的素材不算 sourced asset，不列在这里）
- [ ] AI 使用披露（规则未强制字段，建议写：工具、用途、哪些环节）
- [ ] 赛前产出申报（规则 §7(h)：只写了工具/agent 配置/MCP/prompt 库/脚本，且能与 Git 历史对上）
- [ ] 只报一个引擎（UE，不选 UEFN）
- [ ] 全部内容为英文（游戏内文本、音频、视频、报名信息——规则 §7(i)）
- [ ] 学生身份：itch 项目描述已写 `Student Submission`；**不需要提交证明**
- [ ] **你本人**点击提交（规则 §17 禁止 AI 或自动化代提交）
- [ ] 保存提交确认截图 / 邮件
- [ ] 明白这是**唯一一次提交**：点完即冻结，不再重传

## 10. 提交后

- [ ] 记录提交时间，与截止时间（北京 11/16 09:59）留出 ≥2 小时余量
- [ ] 把记录栏内容写进 `production/qa/smoke-2026-11-15.md`
- [ ] 备份 zip 与视频到第二个位置（移动硬盘 / 网盘）
- [ ] 保留 jam 版本（页面："Keep the jam version as your submitted build"）；**规则 §2 只允许一次提交，提交即冻结**
- [ ] 不要在提交后继续改包：规则 §2 不允许再次提交，赛后改动只能作为 jam 之后的版本，不参与评审
- [ ] 记下提交时用的邮箱：**规则 §14** 会在 2027-01-29 前后发获奖通知，收到后 **72 小时**内必须回 Epic Account ID + Prize Release（含垃圾箱）

---

## 附：一键自动检查覆盖了什么

`scripts\preflight.ps1` 自动检查：编辑器是否在跑、Git 状态与 sha、LFS、磁盘空间、Zen 配置、两张地图与三个必 cook 目录、插件是否限定 Editor、最新包大小与构建配置、zip 是否存在且 ≤1 GB、视频是否 30–60 秒且 ≥1920 宽、提交文案是否还有占位符。

**它不能替你做的**：听声音、看画面、判断好不好玩、换机运行、点提交。
