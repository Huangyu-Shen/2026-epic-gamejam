# 打包与提交准备方案（UE 5.8 / Windows / itch.io）

> 本文件只做准备：不修改玩法代码、不改架构、不动 `project.yaml` 与 `.claude/`。
> 事实来源：`docs/wiki/build-and-run.md` §8、`docs/devlog/day-2/05-packaging-in-five-attempts.md`、
> `Game_01 5.8/Config/DefaultGame.ini`、`production/epics/directors-cut/story-013-package-and-submission.md`、
> 以及 2026-11-15 对仓库和 `Saved/Packages/` 的实测。标注 **未验证** 的条目表示当前仓库里没有证据。

---

## 1. 现状快照（2026-11-15 实测）

| 项目 | 现状 |
|---|---|
| 工程 | `Game_01 5.8/Game_01.uproject`，C++ 模块 `Game_01`，EngineAssociation `5.8` |
| 引擎 | `D:/Game/EPIC/UE_5.8`（`RunUAT.bat` 存在） |
| 已打过的包 | `Saved/Packages/` 下 `art-1 … art-9`、`story-008 … story-012e` 等，全部是 **Development** |
| 最新完整包 | `art-9`（2026-11-15 04:17），整包 1,066 MB |
| 包内构成（art-9） | `Game_01.pdb` 358.7 MB · `Binaries\Win64\Game_01.exe` 321.5 MB · `Content\Paks\*.ucas` 269.1 MB · 其余引擎依赖约 100 MB |
| 去掉 pdb 后 | 约 **707 MB**，对 1 GB 上限有约 300 MB 余量 |
| `Content/` 源资源 | 460 MB（未使用的 Fab 包已于 11/15 删除） |
| Shipping 包 | **从未打过**（`docs/wiki/build-and-run.md` §8 明确写了只有 Development） |
| Git | 分支 `refactor/first-batch`，HEAD `ea2d74e`（技术债第一批），**已跟踪文件干净**；`Content/` 下 7 个 Fab 包仍故意未跟踪 |
| 截止 | 页面原文 "Nov 15, 5:59 PM PST / 8:59 PM EST" = **北京时间 2026-11-16 09:59** |

> 结论：打包链路本身是通的（Development 已验证五次），真正没验证的是 **Shipping 配置**、**换机解压运行**、以及 **Shipping 下的画面与声音** 三项。

---

## 1.1 官方规则与页面核对（2026-11-15）

> 两个来源：① itch jam 页面；② **Official Rules PDF**（19 页，最后更新 2026-11-12，即 Box 上那份 `2026_Epic_AI_MegaJam_Official_Rules_EN.pdf`）。
> 标注：**规则原文**=PDF 英文原句（附条款号）；**页面原文**=itch 页面。冲突时以规则为准——参赛即视为接受规则。
> 上一版标"页面没写"的地方，本节已按规则补齐；规则仍未写的会明确写"规则也没写"。

### 1.1.1 提交什么、怎么交

| 项目 | 规则/页面原文 | 对我们的影响 |
|---|---|---|
| 提交平台 | 页面："Upload it to your own itch.io account, then submit to this jam." | 两步：先把 zip 传到自己的 itch 项目，再把项目提交到 jam。**不是**把 zip 传进 jam 页面 |
| 提交物 | **规则 §1**：`Entrants ... can submit a playable product created using Unreal Engine ("UE") or Unreal Editor for Fortnite ("UEFN") that relates to the relevant theme`；`(c) include the full name and email address of all Entrants on a submitting team, identifying one of them as the team owner (the "Team Owner")`；**§7(h)**：`All other game content, including gameplay, levels, and art assets (other than noted sourced assets), must be created during the Contest Period.` | 必须是在赛期内做出来的可玩作品；表单要填**全员姓名 + 邮箱**，并指定一名 **Team Owner**（收款人和税务主体都是他） |
| 构建形态 | 页面："Package your game for Windows (**development build**). It must be playable on Windows 10 or 11, macOS, Android, or iOS." | Development 包合规，Shipping 不是必须 |
| 自包含 | **规则 §7(f)**：`Submissions should not require any additional software to run.` | 解压双击就能跑，不装运行时 |
| 打包强制 | 页面："Don't submit unpackaged Unreal Engine projects." | 不能交工程目录 |
| zip 命名 | 页面："Name your .zip **TeamName_GameName**" | `KnightOnNight_DirectorsCut.zip` |
| 体积 | 页面："keep it **under 1 GB**" | ≤ 1 GB；超了官方建议 butler 或 Blendo Itch Uploader |
| 视频 | **规则 §1(e)**：`include a video short (between 30-60 seconds) demonstrating recorded gameplay of the Submission ("Video")`；**§7(c)**：`Resolution of the Video must match 1920x1080 or higher` | 30–60 秒是硬性；**1080p 也是硬性**（上一版以为是自定） |
| 语言 | **规则 §7(i)**：`All Submission content (including in-game text, audio and the Video) and all entry information must be in English` | **英文是硬性要求**（上一版标"页面没写"） |
| devlog | 页面："Dev log link (optional, but required for the Glass Box modifier)"；**规则 §10**：`the dev log must be submitted together with the Submission during the Contest Period` | 可选，但冲 Glass Box 就必须随作品一起提交，赛期后补不算 |
| 素材 | **规则 §1(d)**：`sourced (non-original) assets ... must be noted upon submission`；**§15(c)**：`Content the Entrants generate with AI tools is not considered a sourced asset.` | 商用素材必须逐个列出；AI 生成的不算 sourced asset，但要在 AI 披露里写 |
| 提交次数 | **规则 §2**：`Limit one (1) Submission per team. Entrants are not permitted to submit a Submission more than once during the Contest Period.` | **只能交一次，不能重传/修改** |
| 谁点提交 | **规则 §17**：每个提交必须由 Entrant 本人提交，`not by an AI agent or other automated means` | 必须你本人点 |

### 1.1.2 时间

| 项目 | 原文 | 换算 |
|---|---|---|
| 开始 | **规则 §4**：`begins at 12:00 AM Eastern Time ("ET") on November 12, 2026` | 2026-11-12 13:00 北京 |
| **截止** | **规则 §4**：`ends at 8:59 PM ET on November 15, 2026`；页面写作 "Nov 15, 5:59 PM PST / 8:59 PM EST" | **2026-11-16 09:59 北京时间**（= 01:59 UTC） |
| 迟到 | 页面："Late submissions will be disqualified." | 没有宽限 |
| 获奖通知 | **规则 §14**：`on or about January 29, 2027 ... via the email address provided with their Submission` | **提交后要盯这个邮箱到 2027 年 1 月底** |
| 回复时限 | **规则 §14**：`shall have 72 hours from the date such notice was sent` | 72 小时内交 Epic Account ID + Prize Release，超时奖金作废 |

### 1.1.3 奖项与资格

**参赛资格（规则 §5）**：`Participation is open only to legal residents of ... the Eligible Jurisdictions, who have an active, valid Epic Account registered to such Entrant and who are at least eighteen (18) years of age.` 两类人明确排除：`Cabined Accounts`，以及主办方员工/长期合作者及其家属。

**地域（规则首段）**：`VOID WHERE PROHIBITED ... (WHICH INCLUDES IRAN, IRAQ, NORTH KOREA, SOMALIA, SUDAN, SYRIA, AND THE REGIONS OF CRIMEA, DONETSK, AND LUHANSK)`；`Contest may only be entered in or from any jurisdiction located outside of the Restricted Jurisdictions (the "Eligible Jurisdictions")`。**判断**：中国大陆不在限制名单里，可以参赛；但奖金通过 Hyperwallet 发放，收款能力和税表（W-8BEN）要自己确认。

| 项目 | 规则/页面原文 |
|---|---|
| 队伍 | **§1(a)**：`a team of one (1) to five (5) Entrants`；页面：AI agents 不限数量 |
| 主赛道 | Best Overall, UE 与 UEFN 各 $7,500 / $5,000 / $2,500；**Best Student Project**：$7,500 / $5,000 / $2,500 |
| 分项奖 | Best Team $5,000（2 人以上）· Solo Driver $2,500 · Render Royalty $2,500 · Pocket Powerhouse $2,500（移动端）· Glass Box $2,500 · Walkie-Talkie $2,500（仅 UEFN） |
| 赞助商奖 | **§13**：`The ARV of any Partner Prizes is not included in this amount, is not known as of the date of these Official Rules, and will be determined once the Partner Prizes are confirmed.` 现金总额 $65,000 不含赞助商奖 |
| 多奖 | **§13**：`Limit one (1) main category prize and one (1) Modifier Prize per Submission.` → 最多 **1 个主赛道 + 1 个分项** |
| 分成 | **§13**：奖金按团队发，`will be awarded to the Team Owner on behalf of the team`；Hyperwallet 发放，税表自理 |
| 续作资助 | **§11**：赛后由主办方 `invite select Entrants ... to apply`，`not Contest prizes`，另签协议；无公开入口 |
| **学生身份怎么证明** | **规则没写任何证明文件或渠道**。只有 **§7(j)**：`Sponsor may require verification ... solely determined by Sponsor at its sole discretion`；**§14**：获奖后 72 小时内提供材料。即**赛后核验，事前不用交** |
| 学生作品定义 | **§5**：`A Submission created by a team on which every Entrant is a Student Entrant is a "Student Submission."`（单人即满足） |

**完整奖项表、多奖互斥条款、难度与优先级、学生确认邮件、材料清单：见 [05-awards-and-student-track.md](05-awards-and-student-track.md)。**

**对学生奖的含义**：不用交证明，也没有学生专项截止；只要提交时被识别为 Student Submission。唯一未定的是"在 itch 表单哪里标记学生身份"（见 1.1.5）。注意 §9：学生作品`may win either a Best Overall prize or a Best Student Project prize, but not both`。

### 1.1.4 硬性规则（规则原文）

| 规则 | 原文 | 对我们的影响 |
|---|---|---|
| 赛期 | **§1(a)**：作品必须 `created within the contest period`（11/12 13:00 – 11/16 09:59 北京） | 所有玩法、关卡、美术都要在这四天内产生 |
| 赛前产出 | **§7(h)**：`Entrants may prepare AI tools, agent configurations, Model Context Protocol ("MCP") settings, prompt libraries, and scripts before the Contest Period.` | 只有工具、agent 配置、MCP、prompt 库、脚本可以提前做；**模板与脚手架不算** |
| 核查 | **§7(h)**：`Sponsor may ask Entrants to provide these materials, or their version control history, to verify compliance.` | **不要改写或丢弃 Git 历史**；提交文案里的赛前申报要能和历史对上 |
| 主题 | **§7(g)**：`Submissions must demonstrate use of the Contest Theme.` | 主题要在玩法里可见（剪卡机制） |
| 原创性 | **§7(d)**：`Submission must not be an obvious extension of one of Sponsor's tutorials`；**§7(e)**：`must include custom gameplay that exceeds that found in Sponsor's starter templates` | 不能是教程/模板的换皮 |
| 分级 | **§8**（Content Restrictions）：`The Submission must be appropriate for a teen audience, meaning content suitable for players aged 13 and over (consistent with a "Teen" rating under the ESRB system or its equivalent)` | 不能有血腥/成人内容 |
| 外部素材 | **§15(c)**：`all sourced (non-original) assets are noted upon submission`；`such assets will not be reviewed or scored`；`Content the Entrants generate with AI tools is not considered a sourced asset.` | 7 个 Fab 包 + 两个音效包要逐条列出；AI 生成的单独在 AI 披露里写 |
| AI | **§7(a)**：`Submissions may, but are not required to, use AI tools`；**§9**：`The extent of AI use disclosed in a Submission will be considered for context only` | 用不用都行，披露多少不影响分数 |
| 商标 | **§15(d)**：`Submissions may not feature any trademarks (including logos)`（Epic 品牌素材按各自指南例外） | 别在标题/画面里用第三方商标 |
| 引擎 | 页面："Don't enter with both Unreal Engine and UEFN. Pick one." | 只报 UE |
| 保留版本 | 页面："Keep the jam version as your submitted build." | 提交后继续开发可以，但 jam 版本要留着 |

### 1.1.5 规则也没写的事

- **itch 表单里怎么标记 Student Submission**：规则反复用 `if submitting as a Student Submission`，但没说这个身份在哪声明。保险做法：在 itch 项目描述里写一句 `Student Submission — currently enrolled at [school]`（见 05 §3.1）。
- **提交后能不能编辑 itch 项目页**：规则只禁止"重复提交"。保守按**提交即冻结**处理。
- **学生核验的具体程序**：§7(j) 明确是主办方全权决定，事前无法预判。
- **视频格式/码率/托管平台**：只规定 30–60 秒与 ≥1920×1080，其余自定（我们用 H.264 MP4 + 未列出链接）。
- **表单完整字段**：页面只列了四项，实际可能有额外问题，需 Join jam 后才知道。
- **线下场与我们的关系**：Appendix A 的 Digital Jungle 现场抽奖是**独立活动**，`has no effect on the judging of any Submission`，不参与。

---

## 2. 打包前的三个前置事项

### 2.1 工作区状态（story 013 硬性要求）

story 013 的第一条验收标准是"从干净工作区、指定 commit 构建"。

2026-11-15 复查：源文件改动已提交为 `ea2d74e`（`refactor: first tech-debt batch`），已跟踪文件**现在是干净的**；剩下未跟踪的只有 `Content/` 下 7 个 Fab 包（故意不进 Git）和本准备目录 `2026-11-15/`。

所以现在只差一件事：**在构建前确认 HEAD 就是你想要出货的那个 commit，并把它写进 `smoke-2026-11-15.md`**。`preflight.ps1` 会打印当前 sha。别再在这个 commit 之后改玩法代码；要改就重新打包。

Fab 包不进 Git 是既有决定，所以"从干净工作区构建"只能理解为：**用这台机器的 Content + 干净的源文件状态**。不要尝试从 clone 出来的仓库打包，那会得到灰盒无声版本（`docs/wiki/build-and-run.md` FAQ 已记录）。

### 2.2 编辑器专属插件没有限制

`Game_01.uproject` 里 `ModelContextProtocol`、`AllToolsets`、`Terminal` 三个插件只写了 `Enabled: true`，没有 `TargetAllowList: ["Editor"]`（只有 `ModelingToolsEditorMode` 有）。`docs/wiki/build-and-run.md` 的发布前检查里也列了这一条。

风险：编辑器插件被打进包里 → 包体变大，个别插件在非编辑器目标下可能导致打包失败。**这是配置改动，不在本次准备范围**；如果要改，改的是 `.uproject`（不属于玩法代码，但属于项目配置），需要你明确同意。`preflight.ps1` 会把它报成 WARN 而不是 FAIL。

### 2.3 Shipping 不是必须的（按官方页面修正）

原方案默认打 Shipping，理由是"出货包更专业"。**但官方页面写的是 "Package your game for Windows (development build)"** ——Development 包就是官方认可、也是合规的提交形态。

而且 Development 恰恰是本项目唯一验证过五次的配置；Shipping 从没打过，`check()/ensure` 被裁剪、日志默认不落盘、探针（`-OfferProbe`、`-ReelProbe`、`-PerfProbe`）被 `HasProbeFlag()` 直接关掉，出问题很难查。

**修正后的建议：直接用 Development 包提交。** Shipping 只在两种情况下考虑：(a) 时间充足且 Development 包验收已全部通过；(b) 你明确想要更小的包体。不要为了"显得正式"在最后几小时首次尝试一个没验证过的配置。

如果打 Shipping：它仍然可以正常游玩，只是探针不可用，验证工作必须在 Development 包上先完成。

---

## 3. 打包步骤

### 3.1 前置

1. 关闭编辑器、Live Coding、以及任何打开着 `Game_01 5.8/` 下文件的 Rider/VS（否则 UBT 报 "file in use"）。
2. 确认 `Config/DefaultEngine.ini` 里没有 `[Zen] AutoLaunch=False`（历史上这个配置让 cook 和 stage 都失败过两次，现已移除，别再加回来）。
3. 确认 `Config/DefaultGame.ini` 的 `[/Script/UnrealEd.ProjectPackagingSettings]` 仍然包含：
   - `MapsToCook`：`/Game/Maps/L_Camp`、`/Game/Maps/L_Greybox`（缺了 START RUN 直接失败）
   - `DirectoriesToAlwaysCook`：`/Game/Data`、`/Game/Free_Sounds_Pack/cue`、`/Game/PathOfAdventure/Loop/CUE`（都是软引用，cooker 不会自动跟；缺了会**静默**回落 C++ 默认值 / 没有声音）

   这三条用 `preflight.ps1` 一键检查。

### 3.2 方式 A：Development 提交包（推荐；官方页面写的就是 development build）

在仓库根目录打开 PowerShell 7：

```powershell
pwsh -File ".\2026-11-15\jam-submission-prep\outputs\scripts\package-shipping.ps1" `
  -Config Development -Name verify-2026-11-15
```

等价的单行命令（与 `project.yaml` 里记录的形态一致）：

```
"D:/Game/EPIC/UE_5.8/Engine/Build/BatchFiles/RunUAT.bat" BuildCookRun -project="D:/project/2026-epic-ai-megajam/Game_01 5.8/Game_01.uproject" -noP4 -platform=Win64 -clientconfig=Development -build -cook -stage -pak -archive -archivedirectory="D:/project/2026-epic-ai-megajam/Game_01 5.8/Saved/Packages/verify-2026-11-15" -utf8output -unattended
```

这个包用来跑：
- 两张地图来回两次（营地 → 开局 → 撤离/死亡 → 回营地）；
- 删档启动 / 有档启动各一次；
- 探针：`-OfferProbe`、`-ReelProbe`、`-PerfProbe`（带 `-SaveSlot=Probe` 避免改真实进度）；
- 数据表是否真的加载：日志里**不能**出现 `LOOTCFG source=c++ defaults`。

### 3.3 方式 B：Shipping 包（可选，收尾日不要首次尝试）

> 官方页面并不要求 Shipping。只有在你已经用 Development 包完成全部验收、且还有时间余量时才做这一步。

```powershell
pwsh -File ".\2026-11-15\jam-submission-prep\outputs\scripts\package-shipping.ps1" -Config Shipping
```

脚本默认包名 `submission-2026-11-15-<短sha>`，等价命令：

```
"D:/Game/EPIC/UE_5.8/Engine/Build/BatchFiles/RunUAT.bat" BuildCookRun -project="D:/project/2026-epic-ai-megajam/Game_01 5.8/Game_01.uproject" -noP4 -platform=Win64 -clientconfig=Shipping -build -cook -stage -pak -archive -archivedirectory="D:/project/2026-epic-ai-megajam/Game_01 5.8/Saved/Packages/submission-2026-11-15-<sha>" -nodebuginfo -utf8output -unattended
```

要点：
- `-clientconfig=Shipping` 会让 UBT 先编译 `Game_01-Win64-Shipping`，所以会比 Development 慢一些，且**第一次会暴露只在 Shipping 下出现的编译错误**（如果有）。
- `-nodebuginfo` 不生成 `.pdb`，直接省掉约 358 MB。**这一条是本仓库的推断，未验证**；如果 UAT 报未知参数，去掉它，改为在打 zip 时排除 `*.pdb`（`make-submission-zip.ps1` 默认就会排除）。
- 可选项（**均未验证**，非必要不要加）：`-distribution`、`-compressed`。
- 失败时先看脚本打印的日志路径：`<包名>\Build.log` 或 `Saved/Logs/`；UAT 的 `EXIT=-532462766` 在历史上是沙箱/权限问题，不是代码问题。

### 3.4 图形界面备选

编辑器 → Platforms → Windows → Package Project，设置：
- Build Configuration = **Shipping**
- 勾选 Full Rebuild（首次 Shipping 建议）
- 输出目录填 `Game_01 5.8/Saved/Packages/submission-2026-11-15-<sha>`

GUI 与命令行产物结构相同。

---

## 4. 产物位置与命名

```
Game_01 5.8\Saved\Packages\submission-2026-11-15-<sha>\
├── Windows\
│   ├── Game_01.exe                       ← 启动器（约 0.2 MB，双击这个）
│   ├── NOTICES.txt                        ← 第三方许可声明，必须随包分发
│   ├── Manifest_*.txt
│   ├── Engine\                            ← 引擎运行时依赖
│   └── Game_01\
│       ├── Binaries\Win64\Game_01.exe     ← 真实主程序
│       ├── Binaries\Win64\Game_01.pdb     ← 调试符号，不进 zip
│       ├── Content\Paks\Game_01-Windows.{pak,ucas,utoc}
│       └── Saved\                         ← 运行时生成（存档、日志、截图）
└── Build.log / Cook 日志
```

命名约定（沿用 `Saved/Packages/<name>` 的既有习惯）：

| 对象 | 命名 |
|---|---|
| 包目录 | `submission-2026-11-15-<短sha>` |
| 提交 zip | `KnightOnNight_DirectorsCut.zip`（官方要求 `TeamName_GameName`；队名 `Knight On Night` 去空格，理由见 05 §1） |
| 视频 | `DirectorsCut-gameplay-<短sha>.mp4` |
| Smoke 记录 | `production/qa/smoke-2026-11-15.md`（story 013 指定的路径） |

`Saved/` 已在 `.gitignore` 中，包**不要提交**。zip 也不进 Git（几百 MB），放本地和 itch.io。

### 4.1 打 zip

```powershell
pwsh -File ".\2026-11-15\jam-submission-prep\outputs\scripts\make-submission-zip.ps1"
```

规则：
- **文件名必须是 `KnightOnNight_DirectorsCut.zip`**（官方："Name your .zip TeamName_GameName"）。脚本默认 `-TeamName KnightOnNight`，会把空格去掉（不换成连字符）；换队名时传 `-TeamName "..."` 即可；
- **zip 根目录直接是 `Windows\` 的内容**（`Game_01.exe` 在 zip 根），不要多一层 `Windows\`；
- 排除 `*.pdb`、`Saved\`、`*.log`；
- 保留 `NOTICES.txt` 与 `Engine\`、`Game_01\` 的目录结构；
- 输出到 `outputs\dist\`，脚本会打印压缩包大小、与 1 GB 的余量、SHA-256；
- 超过 1 GB 直接失败退出，不做"先传了再说"；官方建议超限时用 [butler](https://itch.io/docs/butler/) 或 Blendo Itch Uploader，但 707 MB 的现状用不上。

### 4.2 体积预算

| 项 | 估算 |
|---|---|
| Development 整包 | 1,066 MB（实测） |
| 去掉 pdb | 707 MB（实测） |
| Shipping 的 exe 通常明显小于 Development 的 321 MB | 预计整包 500–650 MB（**未验证**） |
| zip 压缩后 | 预计 450–600 MB（**未验证**） |
| 上限 | 1,000 MB |

余量足够，但前提是 `Content/` 里不再混进未使用的资源包——11/15 已清理过一轮。

---

## 5. 提交需要哪些文件与格式

**提交是两步**：① 把 zip 传到**你自己的 itch.io 项目页**；② 在 jam 页面把这个项目提交，并填下面这些字段（字段原文见 §1.1.1）。

| 材料 | 格式 / 要求 | 来源或备注 |
|---|---|---|
| Windows 构建 | `.zip`，**`TeamName_GameName`**，< 1 GB，解压即玩 | `KnightOnNight_DirectorsCut.zip` |
| 演示视频 | 30–60 秒 gameplay 视频**链接**（不是上传文件）；**分辨率 ≥1920×1080**（规则 §7(c) 硬性） | `02-video-plan.md` |
| 队名 | 必填；单人也要填 | `Knight On Night`（表单填这个，文件名去空格） |
| 成员姓名 + Epic 账号邮箱 | 每个成员一条；邮箱必须是绑定 Epic 账号的那个；指定一名 **Team Owner** | 规则 §1(c)：`full name and email address of all Entrants ... identifying one of them as the team owner` |
| 游戏名 | 必填 | `Director's Cut` |
| Dev log 链接 | 可选；**Glass Box 分项奖必需，且必须随作品在赛期内提交** | 规则 §10；见 05 §5 |
| 外部素材清单 | 必须逐个列出（`noted upon submission`），且这些素材不参与评分 | 规则 §15(c)；`docs/submission/submission-text.md` 已列 7 个包，按最终包核对 |
| AI 使用披露 | 规则没有强制字段（§7(a) 只说 `may, but are not required to, use AI tools`），但 §9 说 `The extent of AI use disclosed in a Submission will be considered for context only` | 建议一并写：用了哪些工具、做了什么 |
| 赛前产出申报 | 规则 §7(h)：只能提前准备工具/agent 配置/MCP/prompt 库/脚本 | 如实写；主办方可能索要 `version control history` 核对 |
| 学生身份证明 | **规则未要求提交任何证明**，赛后如需核验按 §7(j) 处理 | 见 05 §3：一句话确认 + 材料留本地 |
| 语言 | 规则 §7(i)：游戏内文本、音频、视频、所有报名信息**必须全英文** | 提交前逐项确认 |
| 仓库链接 | 规则未要求 | 加分项，保持公开 |
| 提交确认 | 截图或邮件 | 提交后立刻保存 |

游戏名、团队名、Epic 账号邮箱：**由你本人填写并提交**，代理不能代提交（官方规则）。

---

## 6. 常见坑

| # | 症状 | 原因 | 处理 |
|---|---|---|---|
| 1 | UAT 报文件被占用 | 编辑器 / Live Coding / Rider 还开着 | 全关；`preflight.ps1` 会检测 `UnrealEditor` 进程 |
| 2 | Cook 崩在 "Failed to delete oplog on the ZenServer" | `DefaultEngine.ini` 里 `[Zen] AutoLaunch=False` | 删掉该行（历史上就是它，注释里记了原因） |
| 3 | 打包成功但 START RUN 没反应 | `MapsToCook` 少了 `L_Greybox` | 两条地图都在 `DefaultGame.ini` |
| 4 | 包里数值不对（敌人太弱/收益异常） | `/Game/Data` 不在 `DirectoriesToAlwaysCook`，软引用被跳过，静默回落 C++ 默认值 | 查日志有无 `LOOTCFG source=c++ defaults`；保持该目录在列表里 |
| 5 | 包里没声音 | 两个 CUE 目录是软引用，同样会被跳过 | 保持 `Free_Sounds_Pack/cue`、`PathOfAdventure/Loop/CUE` 在列表里 |
| 6 | zip 超 1 GB | `.pdb` 约 358 MB 被打进去 | 打 zip 时排除；或用 `-nodebuginfo` |
| 7 | GitHub 拒收 / 仓库爆炸 | 单个文件 >100 MB；`.uasset/.umap` 未走 LFS | `.gitattributes` 已配置 LFS（当前 75 个文件在 LFS 中）；**包和 zip 永远不进 Git** |
| 8 | 从 clone 打包得到灰盒无声版 | Fab 资源包未跟踪，是本地资产 | 只在本机打包；README 已写明 `git lfs pull` 和资源缺失的预期 |
| 9 | Shipping 包里探针不响应 | `DebugGate.h` 在 `UE_BUILD_SHIPPING` 下恒为 false | 这是**期望行为**；验证探针请用 Development 包 |
| 10 | Shipping 没有日志可查 | Shipping 默认不写日志文件 | 排查时加 `-log` 启动一次；出货包不要带 |
| 11 | 换机解压后打不开 | 缺 VC++ 运行库，或杀软拦截未签名 exe | 首次运行可能出现 SmartScreen：说明"更多信息 → 仍要运行"；必要时附 VC++ 2015-2022 x64 运行库 |
| 12 | 打包中途磁盘满 | 每打一次包约 1 GB，`Saved/Packages/` 已堆了十几个 | 用 `clean-packages.ps1 -Keep <名字>`（默认只预览，`-Apply` 才删） |
| 13 | 编辑器专属插件进包 | `.uproject` 未限制 `TargetAllowList` | 见 §2.2，需你同意后改 |
| 14 | 提交时区算错 | 页面原文 "Nov 15, 5:59 PM PST" = 北京时间 11/16 09:59 | 按北京时间倒排，别按"今晚 24 点"理解 |
| 15 | 视频 / 文案是中文 | **规则 §7(i)**：`All Submission content (including in-game text, audio and the Video) and all entry information must be in English` | 游戏内文本、视频解说、页面文案、表单全部英文 |
| 16 | 最后几小时首次打 Shipping，出错 | 官方只要求 development build，Shipping 从没验证过 | 直接用 Development 包提交（§2.3） |
| 17 | zip 名字不合规 | 官方要求 `.zip` 命名为 `TeamName_GameName` | 直接用脚本默认的 `KnightOnNight_DirectorsCut.zip` |
| 18 | 迟交被取消资格 | "Late submissions will be disqualified."，没有宽限 | 按北京 11/16 09:59 倒排，留 2 小时缓冲 |
| 19 | 交了工程目录 / 只给源码 | "Don't submit unpackaged Unreal Engine projects." | 必须交打包后的可执行包 |
| 20 | 提交后又改 itch 页面 | **规则 §2**：只允许一次提交，重复提交会被取消资格 | 先在自己的 itch 项目页试传并验收，最后再提交到 jam |
| 21 | 文件名带空格 / 与表单不一致 | 队名 `Knight On Night` 含空格 | 表单填全名，文件名用 `KnightOnNight`；别手改成第三种写法 |
| 22 | 忘了填 dev log 链接 | 页面："required for the Glass Box modifier" | 提交前确认链接已填、退出登录能打开（等于白送的 $2,500） |
| 23 | 学生奖没被计入 | 规则未要求证明，但也没写表单在哪声明学生身份 | 在 itch 项目描述写一句 `Student Submission — currently enrolled at [school]`；材料留本地备查 |
| 24 | 在 jam 表单里多填了成员 | Solo Driver 要求 "built by a solo developer" | 只填你一个人，不要把 AI 助手或化名写成成员 |
| 25 | 提交后又想改包 | **规则 §2**：只允许一次提交，重复提交会被取消资格 | 全部做完再点提交；点完就冻结 |
| 26 | 视频低于 1080p | **规则 §7(c)**：`Resolution of the Video must match 1920x1080 or higher` | OBS 输出设 1920×1080；游戏内分辨率也别留 2048×1152 |
| 27 | 改写 Git 历史掩盖赛前工作 | **规则 §7(h)**：主办方可索要 `version control history` 核对赛期 | 保留原始历史，如实申报赛前产出 |
| 28 | 内容分级/商标踩线 | **规则 §8**：需符合 Teen 分级（`aged 13 and over`）；**规则 §15(d)**：`Submissions may not feature any trademarks (including logos)` | 检查 UI、素材里没有第三方标志，没有血腥内容 |

---

## 7. 顺序建议（T 为截止时间，北京时间 11/16 09:59）

1. **现在（T-20h）**：确认出货 commit，跑 `preflight.ps1`；按 05 §3.1 发一句学生身份确认（非阻塞），学籍材料归档；
2. **T-20h**：打 Development 提交包（官方认可的配置）；
3. **T-18h**：验收：两张地图两轮 + 探针 + 删档/有档启动 + 声音与画面；
4. **T-18h**：写完并发布 devlog 长文，拿到公开链接（Glass Box 的硬性前提，见 05 §5.1）；
5. **T-16h**：打 zip（默认名即可），解压到新目录/另一台机器运行一次，记录大小与 sha；
6. **T-12h**：录视频（见 `02-video-plan.md`），此时包已冻结，画面就是最终画面；
7. **T-9h**：剪到 30–60 秒、上传、拿到链接；
8. **T-7h**：上传 itch.io，页面填完；
9. **T-5h**：`04-pre-submit-checklist.md` 全勾；学生材料归档到 `outputs\dist\student-proof\`；
10. **T-3h**：你本人提交（Team name=`Knight On Night`，成员只有你，**dev log 链接填上**——页面写 optional，但冲 Glass Box 就是硬性，见 05 §5.1），保存确认截图；
11. 留至少 2 小时缓冲（story 013 的要求）；
12. **提交后**：不要改包、不要再提交。规则 §14：2027-01-29 前后会发获奖通知到提交时填的邮箱，收到后 **72 小时内**必须回 Epic Account ID + Prize Release，所以这个邮箱要一直能收信（包括垃圾箱）。

---

## 8. 关联文件

- **官方页面**：https://itch.io/jam/2026-epic-ai-megajam （2026-11-15 抓取）
- **Official Rules**：`2026_Epic_AI_MegaJam_Official_Rules_EN.pdf`（19 页，最后更新 2026-11-12）；抽出的纯文本在 `outputs\dist\official-rules.txt`，便于后续引用条款号
- 提交前自检清单：[04-pre-submit-checklist.md](04-pre-submit-checklist.md)
- 奖项对照与学生赛道：[05-awards-and-student-track.md](05-awards-and-student-track.md)
- 演示视频方案：[02-video-plan.md](02-video-plan.md)
- 录制脚本：[03-video-script.md](03-video-script.md)
- 脚本说明：[README.md](README.md)
- 既有打包文档：`docs/wiki/build-and-run.md` §8
- 打包踩坑记录：`docs/devlog/day-2/05-packaging-in-five-attempts.md`
- story 013：`production/epics/directors-cut/story-013-package-and-submission.md`
