# 奖项对照、学生赛道与学生证明

> 队名：**Knight On Night**（简称 **KON**）。zip 名：**`KnightOnNight_DirectorsCut.zip`**（理由见 §1）。
> 事实来源：https://itch.io/jam/2026-epic-ai-megajam （2026-11-15 抓取）+ **Official Rules PDF**（19 页，最后更新 2026-11-12）。
> 标注说明：**规则原文**=Official Rules 英文原句直引（附条款号）；**页面原文**=jam 页面；**判断**=我们的推断，两者都没有依据。
> 上一版标"页面没写"的地方，本版已用规则原文补齐；规则里仍然没写的会明确写成"规则也没写"。

---

## 1. 队名与 zip 命名（已定）

页面原文：

> "Name your .zip **TeamName_GameName** and keep it **under 1 GB**."
> "Include: Team name, plus each member's name and the email address linked to their Epic Games account / Game name / Gameplay video link (30–60 seconds) / Dev log link"

**判断：表单里填 `Knight On Night`，文件名用 `KnightOnNight_DirectorsCut.zip`。**

理由（三条，都不是页面原文）：

1. 页面只规定 `TeamName_GameName` 的**结构**（下划线分隔），没有规定空格怎么写。空格在文件名里合法，但下载时会被编码成 `%20`，itch.io 的下载 URL 和部分解压工具都会把它弄乱。
2. 队名里没有逗号、斜杠、引号，唯一需要处理的就是空格。**去掉空格、保留首字母大写**，读起来仍是队名本身（KnightOnNight），不会变成第三个名字。
3. 游戏名 `Director's Cut` 里的撇号和空格同样处理：`DirectorsCut`。

| 用在哪 | 填什么 |
|---|---|
| jam 表单 · Team name | `Knight On Night` |
| itch.io 项目页标题 | `Director's Cut` |
| zip 文件名 | `KnightOnNight_DirectorsCut.zip` |
| 视频标题 / 字卡 | `Director's Cut — Knight On Night` |
| 可接受的替代写法 | `Knight-On-Night_DirectorsCut.zip`（**判断**：也合规，但和脚本默认值不一致，别混用） |

脚本已改成默认产出这个名字（`make-submission-zip.ps1` 的 `-TeamName` 默认 `KnightOnNight`，且会把空格去掉而不是换成连字符）。

---

## 2. 全部分项奖与主赛道奖（规则原文为主，页面原文补奖金与评审说明）

### 2.1 主赛道（Main categories）

| 奖项 | 奖金 | 评选标准原文 | 参评门槛 | 额外材料 |
|---|---|---|---|---|
| **Best Overall, Unreal Engine** | $7,500 / $5,000 / $2,500 | **规则原文 §9**："The highest scoring Qualified Unreal Engine Submission will be deemed the potential Best Overall: Unreal Engine First Place Prize winner."（按 §2.3 的 7 项分排序） | 用 UE 提交、通过预筛（Qualified Submission） | 无 |
| **Best Overall, UEFN** | $7,500 / $5,000 / $2,500 | 同上 | 用 UEFN 提交 | 不适用（我们选 UE） |
| **Best Student Project**（Unreal Engine or UEFN） | $7,500 / $5,000 / $2,500 | **规则原文 §9**：学生作品与其他作品用同一套 7 项标准打分，"The highest scoring Qualified Student Submission will be deemed the potential Best Student Project First Place Prize winner." | **规则原文 §5**："the Entrant must be currently enrolled at a higher-educational institution"；**§1(f)**：组队时"all Entrants in a team must be Student Entrants"（单人即满足） | **无**（规则未要求任何证明文件，见 §3） |

### 2.2 分项奖（Modifier categories）

| 奖项 | 奖金 | 规则原文（§10，每行即该奖的全部官方描述） | 我们是否够格 | 额外材料 |
|---|---|---|---|---|
| **Best Team** | $5,000 | `For the best overall game created by a team of two (2) or more Entrants.` | **不够格**（单人） | — |
| **Solo Driver** | $2,500 | `For the best game created by a solo developer.` | 够格 | 无，自动参评 |
| **Render Royalty** | $2,500 | `For the game with the best overall visuals, photoreal or stylized.` | 够格 | 无 |
| **Pocket Powerhouse** | $2,500 each（UE 一名 + UEFN 一名） | `For the best mobile game, with one (1) Unreal Engine winner and one (1) UEFN winner.` | **当前不够格**：只有 Windows 包 | 需要移动端可玩构建 |
| **Glass Box** | $2,500 | `For the best dev log documenting how the Entrant created their Submission. To be eligible, the dev log must be submitted together with the Submission during the Contest Period.` | 够格 | **devlog 必须随作品、在赛期内提交**（写成 jam 表单里的 Dev log link） |
| **Walkie-Talkie** | $2,500 | `For the best narrative-driven UEFN game featuring large language model ("LLM")-powered non-player characters built with the UEFN conversations feature.` | **不够格**（仅 UEFN） | — |

**分项奖怎么评（规则原文 §10）**：`Qualified Submissions will be reviewed and scored ... The highest scoring Qualified Submission in each Modifier Prize category that meets the modifier criteria will be deemed a potential Modifier Prize winner`。也就是说：**先过合规预筛，再用同一套 7 项分数在每个分项里排第一**，没有单独的评委组或单独评分表。

### 2.2.1 能不能一个作品拿多个奖（规则原文）

| 规则原文 | 条款 | 对我们的含义 |
|---|---|---|
| `Limit one (1) main category prize and one (1) Modifier Prize per Submission.` | §13 | **最多 1 个主赛道奖 + 1 个分项奖**。所以"Best Overall UE + Glass Box"可以同时拿 |
| `A potential prize winner can win only one (1) Modifier Prize category, unless no other eligible potential winner remains for the additional category or categories.` | §10 | 分项奖之间互斥：Glass Box 与 Render Royalty 不会同时给同一个作品（除非没有别的合格作品） |
| `a Student Submission may win either a Best Overall prize or a Best Student Project prize, but not both` | §9 | 学生作品**不能同时**拿 Best Overall 与学生奖。若被判为 Best Overall 获奖者，学生奖顺延给下一名 |
| `Limit one (1) Submission per team. Entrants are not permitted to submit a Submission more than once during the Contest Period.` | §2 | **只能提交一次，不能重传或修改**（见 §5.5） |

**判断**：学生奖与 Best Overall 的互斥是自动的，不由我们选。两个第一都是 $7,500，因此最坏情况没有损失；真正的风险是——若作品只够 Best Overall 第三（$2,500），它会失去学生奖第一（$7,500）。但这是评委算分的结果，我们无法干预，只能把作品分数整体做高。

### 2.3 评分标准（规则原文 §9）

`All Submissions will be pre-screened to ensure compliance with the Guidelines and Restrictions ... prior to any judging ("Qualified Submissions").`

| 维度（规则原文） | 分值 | 页面补充说明（`What judges look for`） |
|---|---|---|
| Ambition and Innovation | Up to 5 points | `Something you couldn't usually pull off in a four-day jam: a new mechanic, a bold art style, or a bigger scope` |
| Use of Contest Theme | Up to 5 points | `A fresh, clever take on the theme` |
| Fun Factor | Up to 5 points | `Gameplay that's fun and satisfying` |
| Gameplay | Up to 5 points | `Solid, original mechanics that feel good` |
| Visuals | Up to 5 points | `A strong, consistent look, photoreal or stylized` |
| Level Design | Up to 5 points | `Spaces that support the gameplay and guide the player` |
| Audio | Up to 5 points | `Sound and music that add to the experience` |

两条关键补充（规则原文）：

- 平局：`In the event of a tie, the tying Submissions will be re-scored with double weight given to Fun Factor.` → **好玩程度是平局裁判**。
- AI：`The extent of AI use disclosed in a Submission will be considered for context only and will not, on its own, increase or decrease a Submission's score.` → 用 AI 不加分也不减分，**但"披露的 AI 程度"会被看到**，如实写即可。

**判断**：7 项等权，满分 35。我们的强项是 Ambition/Theme/Gameplay，短板是 Visuals/Level Design（灰盒）。这解释了为什么 Render Royalty 不值得投入——它只看其中一项，而我们的分数差在两项。

### 2.4 续作资助与赞助商奖（规则原文 §11 / §13）

`Following the Contest, Sponsor may, in its sole discretion, invite select Entrants of Qualified Unreal Engine and UEFN Submissions to apply for development continuation grants of up to fifty thousand U.S. dollars (US $50,000)`；金额按 UE/UEFN 各自第一 $50,000、第二 $25,000、第三 $10,000。

- `Development Continuation Grants are not Contest prizes.`（不计入 $65,000）
- `Sponsor is under no obligation to award any Development Continuation Grants.`
- 条款由单独书面协议约定（`a separate written agreement`）

**判断**：没有公开申请入口，是赛后**由主办方邀请**（invite ... to apply）。不需要在提交时做任何额外动作，把主赛道做好即可。

**赞助商奖（Partner Prizes，规则 §13）**：`The ARV of any Partner Prizes is not included in this amount, is not known as of the date of these Official Rules, and will be determined once the Partner Prizes are confirmed.` 即：品类、金额、评判方式在规则定稿时都还没确定。**判断**：无法针对性准备，按"不存在"处理。

> 同节还写明奖金归属：`Each prize ... will be awarded to the Team Owner on behalf of the team`，`Cash prizes will be awarded ... via Hyperwallet`。单人参赛则你就是 Team Owner，奖金直接发给你。

### 2.5 总额核对

页面原文 `$65,000 in cash prizes`。可对上：主赛道 3×$15,000 = $45,000；分项 5,000 + 2,500×5（Best Team、Solo、Render、Glass、Walkie）+ 2,500×2（Pocket 两个赢家）= $20,000。合计 $65,000。**资助（最多 $50,000/队）不计入这 $65,000。**

---

## 3. 学生资格：规则原文说了什么

上一版这里几乎全是"页面没写"，规则 PDF 补上了。结论是：**没有"提交证明"这道手续。**

| 问题 | 规则原文 / 结论 |
|---|---|
| 谁够格 | **§5**：`To be eligible as a Student Entrant, the Entrant must be currently enrolled at a higher-educational institution.` |
| 组队时是否全员必须是学生 | **§1(f)**：`if submitting as a Student Submission, all Entrants in a team must be Student Entrants`。我们单人，天然满足 |
| 怎么证明 | **规则没写任何证明文件、渠道或格式**。只在 **§7(j)** 留了口子：`Sponsor may require verification of Submissions through a procedure solely determined by Sponsor at its sole discretion.` |
| 什么时候核验 | **§14**：获奖通知发出后，获奖者 `shall have 72 hours` 提供材料（`Epic Account ID`、`Prize Release`）。即**赛后**核验 |
| 什么时候交、交到哪 | 与作品同一次提交、同一个 jam 页面。规则里没有学生专项入口，也没有独立截止时间 |
| 怎么算分 | **§9**：学生作品用同一套 7 项标准单独排序，取前三名 |
| 和 Solo Driver 冲突吗 | 不冲突。学生奖是主赛道奖、Solo Driver 是分项奖，**§13** 允许 `one (1) main category prize and one (1) Modifier Prize per Submission` |
| 和 Best Overall 冲突吗 | **§9**：`may win either a Best Overall prize or a Best Student Project prize, but not both` |

**判断**：要做的只有两件——① 提交时确认作品被识别为 Student Submission（下面唯一待确认点）；② 学籍材料留在手边，2027 年 1 月若收到通知，72 小时内能拿出。**不需要主动发邮件求证**。

### 3.1 唯一待确认的一点：提交页面怎么标记学生身份

规则反复说 `if submitting as a Student Submission`，但没说这个身份在哪里声明——itch 的 jam 表单里有没有 "Student" 勾选项、还是靠队名/描述里写明，**规则和页面都没写**。这是唯一还值得问一句的事。

下面是一封一句话确认信（原版那 4 个关于证明方式的问题已经有答案，删掉了）：

```
Subject: 2026 Epic AI MegaJam — how to mark an entry as a Student Submission

Hi MegaJam team,

I'm entering solo as "Knight On Night" with a UE 5.8 game, "Director's Cut".
I'm currently enrolled at [school], so my entry is a Student Submission under
Section 5 of the Official Rules, and I'd like it judged for Best Student Project.

Could you tell me how that is indicated on the itch.io submission page — is
there a field to tick, or is it handled another way? I want to make sure the
entry is routed into the student pool rather than relying on assumption.

I have enrollment documentation ready if verification is requested.

Thanks,
[姓名] / [学校与学位阶段] / [Epic 账号邮箱] / [itch.io 用户名]
```

**发送时机**：这条不阻塞任何工作。页面上没有 jam 管理员邮箱（规则 §24 只有邮寄地址），只能发到 UE 论坛的 jam 帖（`epic.gm/2026-ue-megajam`）或 jam 页面评论区。**如果到提交时还没有答复，就在 itch 项目描述里写一句 "Student Submission — currently enrolled at [school]"，然后照常提交**，不要因此延后。

### 3.2 学籍材料（不提交，留着备查）

规则不要求现在提交，但 §7(j) 与 §14 保留了赛后核验的权利。材料会随学期变化，趁现在备好。
放 `outputs\dist\student-proof\`，统一 PDF，文件名用英文，别带中文和空格。

| # | 材料 | 具体要求 | 状态 |
|---|---|---|---|
| 1 | 学生证 / 校园卡 | 正面 + 背面（或带有效期的内页），姓名与在读状态清晰可辨 | 待备 |
| 2 | 在读证明（Enrollment Verification Letter） | 教务系统或注册处出具，含姓名、学号、学校、**当前学期**、全日制/非全日制；英文版优先，中文版需附简要英文说明 | 待备 |
| 3 | 本学期成绩单（可选） | 能佐证"当前在读"，比学生证更有说服力 | 待备 |
| 4 | 学校邮箱截图 | 显示 `@<学校域名>` 的收件界面，含姓名 | 待备 |
| 5 | 身份证明（可选，被要求时再给） | 护照或身份证，仅在与学籍姓名核对时提供 | 待备 |
| 6 | 一页说明（Student-Proof-README） | 姓名、学校、学号、学位阶段、预计毕业时间、Epic 账号邮箱、itch 用户名；英文 | 待写 |

注意事项（**判断**）：

- 学号可以打码，但**姓名、学校、当前学期**不能遮，否则无法核验。
- 所有文件里的姓名要和 Epic 账号、jam 表单一致；不一致就加一页说明（如护照名 vs 常用名）。
- 不要在公开的 itch 页面或 jam 评论区贴学生证——只用主办方指定的私下渠道。
- 如果学校只出中文证明，附一页自己翻译的英文对照（注明 "unofficial translation"）。

---

## 4. 以当前项目状态看，各奖项的争取难度与优先级

项目现状（来自 §1 的实测与仓库文档）：两张地图（`L_Camp` + `L_Greybox`）、玩法闭环完整（剪卡 → 变强 → Boss 拿走 → 撤离 → 解锁）、7 个 Fab 素材包 + 两个免费音效包、关卡仍是灰盒体块、devlog 有 Day 2 与 Day 4 记录、单人四天。

| 奖项 | 奖金 | 页面给的竞争面 | 我们的差距 | 难度（判断） | 优先级（判断） |
|---|---|---|---|---|---|
| Best Overall, UE | $7,500 | 页面显示 4,154 人已加入，主赛道最挤 | 完成度高，但视觉与关卡是灰盒 | 高 | **P0 主目标** |
| Best Student Project | $7,500 | 学生作品单独排序，取前三 | **不需要任何材料**；只需确认提交页怎么标记学生身份 | 中（唯一风险是被 Best Overall 判定覆盖） | **P0** |
| Solo Driver | $2,500 | 单人作品池，基本等于"所有单人参赛者" | 零差距：本来就是单人 | 中高（人多，但无门槛） | **P1 顺带** |
| Glass Box | $2,500 | 要有一条像样的 devlog | 已有 Day 2/Day 4 素材，缺一条**完整叙事**的复盘长文 | 低（愿意写就能进） | **P0 性价比最高** |
| Render Royalty | $2,500 | 和所有作品比画面 | 灰盒地形 + 商用素材，风格不统一 | 高（想做要重做美术，来不及） | **P3 不专门投入** |
| Pocket Powerhouse | $2,500 | 移动端 | 只有 Windows 包，移动打包从未验证 | 不可行（收尾日做等于自杀） | **P4 放弃** |
| Best Team | $5,000 | 2 人以上 | 单人，**规则上不够格** | — | **不适用** |
| Walkie-Talkie | $2,500 | 仅 UEFN | 引擎不对 | — | **不适用** |
| Continuation Grants | 最高 $50,000 | 从所有提交里挑 | 无独立入口，看重作本身 | 高 | **顺带**（把主赛道做到最好即可） |

---

## 5. 建议：专门投入什么、顺带满足什么

### 5.1 值得专门投入（合计约 2–3 小时）

**① Glass Box（$2,500，回报率最高）**

规则原文（§10）：`For the best dev log documenting how the Entrant created their Submission. To be eligible, the dev log must be submitted together with the Submission during the Contest Period.` 注意 "during the Contest Period" ——**赛期结束后再补的 devlog 不算**。我们已经有 Day 2 打包踩坑、Day 4 的素材，缺的是"一条能读完的故事"。

提交前要完成：
- 一条英文长文（1500–2500 词），结构：主题怎么读出来的 → 核心机制（剪卡=剪辑决定，被剪的变成 Boss 武器）→ 四天怎么排 → 三个具体的技术/设计决策（含打包失败五次这种真实细节）→ 截图与 GIF；
- 发布在一个**退出登录也能打开**的地方（itch devlog 或 GitHub `docs/devlog/` 的公开页面）；
- 把链接填进 jam 表单的 Dev log link——**这是参评的硬性前提**；
- 文末放游戏页链接与视频链接。

**② Best Student Project（$7,500，零额外材料）**

- 不必写长信：按 §3.1 发一句"怎么标记学生身份"，不回复也不影响提交；
- 把 §3.2 的材料备到本地（赛后核验用）；
- 提交时在 itch 项目描述里写明 `Student Submission — currently enrolled at [school]`；
- 记住 **§9 的互斥条款**：一旦被评为 Best Overall 获奖者，学生奖自动失效，无法选择。

**③ Best Overall, UE（$7,500，靠主赛道）**

- 不再加功能，只做打磨：第一局的引导、撤离反馈、Boss 拿走卡时的表现力；
- 视频必须在前 5 秒给出最强的画面（见 `03-video-script.md` 第 0 镜）；
- 这一步的收益同时喂给 Solo Driver 和 Grants，是**一次投入、四处生效**。

### 5.2 顺带满足即可（不额外花时间）

| 奖项 | 怎么"顺带" |
|---|---|
| Solo Driver | 表单里队名照填 `Knight On Night`，成员只有你；不需要额外材料。**规则原文 §10** 要求 `the best game created by a solo developer`，§1(a) 允许 `a team of one (1) to five (5) Entrants`——所以单人合法，但**别在成员列表里加"AI 助手"或化名**，那会把自己踢出单人组 |
| Continuation Grants | 无入口，提交质量决定一切 |
| Render Royalty | 只在两处顺手：视频调色统一、封面图用最好看的一帧。**不重做美术** |

### 5.3 明确放弃

- **Pocket Powerhouse**：需要移动端构建。UE 的 Android 打包在本仓库从未验证（`docs/wiki/build-and-run.md` §8 只有 Win64），收尾日尝试风险极高，且手机上的操作（`WASD` + `1-6`）本来就不成立。
- **Best Team**：规则要求 2 人以上。
- **Walkie-Talkie**：仅 UEFN。

### 5.4 提交前必须完成的（按时间排）

| 时间（北京时间） | 动作 | 关联奖项 |
|---|---|---|
| 现在 | 发 §3.1 的一句确认（非阻塞）；学籍材料归档 | Best Student |
| T-18h 前 | 写完并发布 devlog 长文，拿到公开链接 | Glass Box |
| T-16h 前 | 打 Development 包 + 验收 | 全部 |
| T-12h 前 | 录视频，确保前 5 秒是最强画面 | Best Overall / Solo / Render |
| T-6h 前 | 填表单：Team name=`Knight On Night`、成员=你、Epic 邮箱、游戏名、视频链接、**Dev log 链接**、外部素材清单、赛前成果申报 | Glass Box + 全部 |
| T-3h 前 | 你本人点提交（规则 §17 禁止 AI 或自动化代提交），保存确认截图 | — |

### 5.5 提交只能一次（规则原文 §2）

`Limit one (1) Submission per team. Entrants are not permitted to submit a Submission more than once during the Contest Period. Duplicate Submissions by the same person will be subject to disqualification.`

**判断**：这意味着**没有"先交一个凑数、之后再换"的操作空间**。把打包、验收、视频、devlog 全部做完，再点提交。提交后不要再动 itch 上的构建文件——即使规则只禁止"重复提交"，改包也可能被视作同一提交的变更，风险不值得冒。

---

## 6. 需要你在提交时做的三件事

1. **队名填 `Knight On Night`**，成员只填你自己（不要加 AI 或化名）。
2. **Dev log 链接一定要填**——不填就等于放弃 Glass Box，`$2,500` 的门票是免费的。
3. **学生资格**：不需要提供任何证明就能参评；只在 itch 项目描述里写明 `Student Submission`，材料留在手边备赛后核验。**不要为了等主办方回复推迟提交**。
