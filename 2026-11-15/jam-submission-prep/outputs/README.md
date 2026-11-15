# 收尾准备包（打包 · 提交 · 演示视频）

单人 2026 Epic AI MegaJam / Unreal Engine 5.8 / 主题 Director's Cut。
这里只放**准备物**：方案、清单、脚本。没有改任何玩法代码、架构或 `project.yaml`。

截止时间：**2026-11-16 01:59 英国时间 = 北京时间 2026-11-16 09:59**。

---

## 先读哪份

| 文件 | 什么时候用 |
|---|---|
| [01-package-and-submit-plan.md](01-package-and-submit-plan.md) | 打包前：打包步骤、产物与命名、体积、28 条常见坑、材料清单 |
| [02-video-plan.md](02-video-plan.md) | 录视频前：OBS 参数、系统设置、镜头编排、字幕与上传 |
| [03-video-script.md](03-video-script.md) + [03-video-script.srt](03-video-script.srt) | 录制时：照读的英文台词（52 秒主版、45/60 秒备版） |
| [04-pre-submit-checklist.md](04-pre-submit-checklist.md) | 提交当天：逐项打勾，手抄进 `production/qa/smoke-2026-11-15.md` |
| [05-awards-and-student-track.md](05-awards-and-student-track.md) | 决定冲哪些奖：全部奖项对照表、多奖互斥规则、优先级、学生身份一句话确认与材料清单 |
| `scripts\` | 能自动化的环节，见下 |

---

## 规则依据

两个来源：官方页面 https://itch.io/jam/2026-epic-ai-megajam （2026-11-15 抓取）与 **Official Rules PDF**（19 页，最后更新 2026-11-12）。逐条对照在 [01 §1.1](01-package-and-submit-plan.md)；PDF 抽出的纯文本在 `dist\official-rules.txt`，方便引用条款号。

最容易踩的原文规定：

- **"Package your game for Windows (development build)"** —— Development 包就是合规提交形态，Shipping 不是必须。
- **"Name your .zip TeamName_GameName"** 且 **"keep it under 1 GB"**。我们的队名是 **Knight On Night**，文件名写作 **`KnightOnNight_DirectorsCut.zip`**（去空格，理由见 05 §1）。
- **"8:59 PM ET on November 15, 2026"**（规则 §4）= 北京时间 **2026-11-16 09:59**；迟到直接取消资格。
- **规则 §2**："Limit one (1) Submission per team." —— **只能交一次，不能重传或改包**。
- **规则 §7(c)/§7(i)**：视频 **≥1920×1080**、所有内容（游戏内文本、音频、视频、报名信息）**必须英文**。
- **规则 §9**：学生作品 `may win either a Best Overall prize or a Best Student Project prize, but not both`；**§13** 允许"1 个主赛道 + 1 个分项"。

---

## 脚本怎么运行

全部是 PowerShell（Windows）。**在仓库根目录、普通 PowerShell 窗口里跑**，不要通过代理/沙箱执行——UBT 要写 `%LOCALAPPDATA%`，沙箱会拒绝并报 `EXIT=-532462766`。

```powershell
cd D:\project\2026-epic-ai-megajam
pwsh -File ".\2026-11-15\jam-submission-prep\outputs\scripts\<脚本名>.ps1"
```

用 Windows PowerShell 5.1 也能跑；`pwsh` 是 PowerShell 7，推荐。
如果提示"禁止运行脚本"，临时放开当前窗口：

```powershell
Set-ExecutionPolicy -Scope Process -ExecutionPolicy Bypass
```

### 1. `preflight.ps1` — 只读自检（随时跑）

```powershell
pwsh -File ".\2026-11-15\jam-submission-prep\outputs\scripts\preflight.ps1"
```

检查：编辑器是否在跑、Git sha/脏文件/LFS、磁盘空间、Zen 配置、两张地图与三个必 cook 目录、插件是否限定 Editor、最新包大小、zip 是否 ≤1 GB、视频是否 30–60 秒且 ≥1920 宽、提交文案是否还有 `TO ADD`。
输出 `OK / WARN / FAIL`；有 FAIL 时退出码 1。**不改任何东西。**

### 2. `package-shipping.ps1` — 打包

```powershell
# 提交用的包：Development（官方页面要求 development build，也是唯一验证过的配置）
pwsh -File ".\2026-11-15\jam-submission-prep\outputs\scripts\package-shipping.ps1"

# 可选：Shipping 包。从没验证过，别在收尾日首次尝试
pwsh -File ".\2026-11-15\jam-submission-prep\outputs\scripts\package-shipping.ps1" -Config Shipping
```

- 默认 `-Config Development`，包名 `submission-<日期>-<短sha>`，输出 `Game_01 5.8\Saved\Packages\<包名>\Windows\Game_01.exe`
- 日志写到 `outputs\dist\logs\`
- 编辑器开着会直接拒绝（要强制加 `-Force`）
- `-NoDebugInfo` 会传 `-nodebuginfo`（省 358 MB，本仓库未验证过该参数；失败就去掉）
- `-SkipBuild` 只 cook/stage，不重新编译
- `-EngineRoot` 换引擎路径（默认 `D:/Game/EPIC/UE_5.8`）

### 3. `make-submission-zip.ps1` — 打提交 zip

```powershell
# 默认就产出 KnightOnNight_DirectorsCut.zip
pwsh -File ".\2026-11-15\jam-submission-prep\outputs\scripts\make-submission-zip.ps1"
```

- `-TeamName` 决定文件名，官方要求 `TeamName_GameName.zip`；默认 `KnightOnNight`，会把空格去掉（不换成连字符），所以 `"Knight On Night"` 会变成 `KnightOnNight`
- 默认取 `Saved\Packages` 里最新的完整包
- 把 `Windows\` 的**内容**放在 zip 根（解压后 `Game_01.exe` 就在根目录）
- 排除 `*.pdb`、`Saved\`、`*.log`
- 打印大小、1 GB 余量、SHA-256，并写 `outputs\dist\zip-<sha>.md`
- 超过 1 GB 直接失败退出（退出码 4）

### 4. `archive-materials.ps1` — 素材归档

```powershell
# 录完先归档原始素材（默认只登记不复制）
pwsh -File ".\2026-11-15\jam-submission-prep\outputs\scripts\archive-materials.ps1"

# 成片出来后，把视频、zip、文案收进一个带清单的文件夹
pwsh -File ".\2026-11-15\jam-submission-prep\outputs\scripts\archive-materials.ps1" `
  -VideoPath "D:\edit\DirectorsCut-final.mp4"
```

产出 `outputs\dist\submission-<sha>\`：`build\`、`video\`、`text\`、`raw\`，以及 `manifest.md`、`SHA256SUMS.txt`。
原始素材默认放在 `outputs\raw\`（需要你自己建，或用 `-RawDir` 指定）；加 `-IncludeRaw` 才会把原始录像复制进归档。
装了 FFmpeg 时会自动记录并校验视频的分辨率/时长/帧率。

### 5. `clean-packages.ps1` — 清理旧包（默认只预览）

```powershell
pwsh -File ".\2026-11-15\jam-submission-prep\outputs\scripts\clean-packages.ps1"
pwsh -File ".\2026-11-15\jam-submission-prep\outputs\scripts\clean-packages.ps1" `
  -Keep "submission-2026-11-15-<sha>" -Apply
```

每个包 0.7–1.1 GB，`Saved\Packages\` 里已经堆了十几个。默认保留最新 1 个和 `-Keep` 列出的，**不加 `-Apply` 不删任何东西**。

---

## 目录约定

```
2026-11-15\jam-submission-prep\
├── outputs\                 ← 本目录：方案、清单、脚本
│   ├── 01…04 & .srt
│   ├── scripts\             ← 5 个 .ps1 + _common.ps1（被其他脚本加载，不要单独跑）
│   ├── raw\                 ← 你自己放 OBS 原始录像（可选）
│   └── dist\                ← 脚本产物：zip、日志、归档
└── （不放任何游戏资源或构建产物到 Git）
```

`outputs\dist\` 和 `outputs\raw\` 体积大，不适合提交进仓库；如果要提交，先在 `.gitignore` 里加规则——**这属于改配置文件，我没有动**。

---

## 已定的事

1. **队名**：`Knight On Night`（简称 KON）。表单填全名，zip 文件名写作 `KnightOnNight_DirectorsCut.zip`。
2. **奖项**：报 **Best Student Project**，同时争 Best Overall UE / Solo Driver / Glass Box。完整对照、多奖互斥条款与优先级见 [05](05-awards-and-student-track.md)。**学生奖不需要提交任何证明**（规则只定义了"目前在读"，核验权在主办方，且只在获奖后才可能行使）；在 itch 项目描述写一句 `Student Submission — currently enrolled at [school]` 即可。
3. **提交形态**：Development 包（官方页面写的就是 development build）。
4. **提交次数**：一次，且必须你本人点（规则 §2、§17）。

## 还需要你决定 / 动手的事

1. **学生身份确认（可选、非阻塞）**：05 §3.1 只剩一句"提交页怎么标记学生身份"；没有答复就在 itch 描述里写明学生身份并照常提交。
2. **devlog 长文**：Glass Box 的硬性前提（规则 §10 要求 `submitted together with the Submission during the Contest Period`），提交前必须有一条公开可读的英文 devlog。
3. **赛前成果申报**：规则 §7(h) 允许提前准备工具/agent 配置/MCP/prompt 库/脚本，其余内容必须在赛期内做；主办方可索要 `version control history` 核对，所以 Git 历史别改写。
4. **编辑器专属插件未限制**（可选）：`ModelContextProtocol` / `AllToolsets` / `Terminal` 没有 `TargetAllowList: ["Editor"]`。改 `.uproject` 属于项目配置变更，等你同意。

（原先的"工作区不干净"已解除：源文件改动已提交为 `ea2d74e`，已跟踪文件干净。构建前确认 HEAD 就是出货 commit，并写进 smoke 记录即可。Shipping 也不再是必选项——官方页面写的是 development build。）

---

## 与既有文档的关系

不另起一套：命令形态沿用 `project.yaml` 的 `commands.*` 与 `docs/wiki/build-and-run.md` §8；坑的来源是 `docs/devlog/day-2/05-packaging-in-five-attempts.md` 与 `docs/wiki/known-issues.md`；验收标准来自 `production/epics/directors-cut/story-013-package-and-submission.md`。
本目录只是把它们整理成**可照着执行**的顺序、清单和脚本。
