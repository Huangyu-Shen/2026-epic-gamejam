# 演示视频方案（30–60 秒与 ≥1920×1080 都是硬性要求）

> **官方硬性规定有两条**：**规则 §1(e)**："a video short (between 30-60 seconds) demonstrating recorded gameplay"；**规则 §7(c)**："Resolution of the Video must match 1920x1080 or higher"。另 **规则 §7(i)** 要求视频内容必须为英文。
> 60 fps、编码器、码率、上传平台仍是我们**自定**的。
> 官方 Tips 另有一句："**Show gameplay in the first five seconds**"——所以成片必须冷开场就打架，不能先放 5 秒营地。
> 内容必须让评委看到：**一次升级时的"剪掉"**、**一次升星**、**一次撤离**。
> 素材来源：提交用的包（Development，画面即提交画面）。录制与解说的实际台词见 [03-video-script.md](03-video-script.md)。

---

## 1. 本机现状

| 项 | 现状 | 结论 |
|---|---|---|
| OBS Studio | 未安装 | 用 `winget install OBSProject.OBSStudio` 安装（**需要你执行**） |
| FFmpeg | 未安装 | 可选：`winget install Gyan.FFmpeg`，用于裁剪/压制/烧字幕 |
| Windows 录屏 | Windows 11 自带 Xbox Game Bar（Win+Alt+R） | 兜底方案，码率与音频分轨不可控 |
| 剪辑软件 | 未确认 | 推荐 Clipchamp（Win11 自带）或 DaVinci Resolve；也可用 FFmpeg 纯命令行 |
| 麦克风 | 未确认 | 没有麦克风就用**纯字幕版**（见 §7） |

---

## 2. 录制工具与参数

### 2.1 OBS 设置（推荐）

| 设置 | 值 | 原因 |
|---|---|---|
| 基础画布 / 输出分辨率 | 1920×1080 | 规则 §7(c) 的硬性下限，别用 1440p 再缩，容易糊 |
| FPS | 60 | 割草动作片，30 会顿 |
| 编码器 | NVENC HEVC 或 AV1；无 N 卡用 x264 | 质量/体积平衡 |
| 码率控制 | CQP 20（NVENC）/ CRF 18（x264） | 动作场面不掉块 |
| 录制格式 | **MKV** | 崩溃时文件仍可用；录完再 Remux 成 MP4 |
| 音频 | 48 kHz 立体声 | 与游戏音频一致 |
| 轨道 | 轨 1 游戏音、轨 2 麦克风 | 后期可单独调音量、可整轨丢弃 |
| 游戏音频源 | "应用程序音频采集"（Application Audio Capture）指向 `Game_01.exe` | 避免录进浏览器/微信提示音 |
| 光标 | 录制时隐藏鼠标（场景里有 UI 点击时可短暂显示） | 画面干净 |
| 输出目录 | `D:\project\2026-epic-ai-megajam\2026-11-15\jam-submission-prep\raw\` | 与交付物分开，便于归档脚本处理 |

### 2.2 系统与游戏准备

1. 电源计划改"高性能"，关掉所有通知（Win11：设置 → 系统 → 通知 → 勿扰）。
2. 关闭浏览器、Discord、微信等占用 GPU/网络的程序。
3. 把包的窗口分辨率设为 1920×1080 无边框。游戏 Esc 菜单里**没有**分辨率选项，要改配置文件：
   `<包目录>\Windows\Game_01\Saved\Config\Windows\GameUserSettings.ini`
   ```ini
   ResolutionSizeX=1920
   ResolutionSizeY=1080
   LastUserConfirmedResolutionSizeX=1920
   LastUserConfirmedResolutionSizeY=1080
   FullscreenMode=1
   LastConfirmedFullscreenMode=1
   PreferredFullscreenMode=1
   ```
   （现有开发包里是 2048×1152，直接改会得到非标准分辨率。）
4. 备份并替换存档，确保开局就有可用的营地进度与一张可点的影片卡：
   存档在 `<包目录>\Windows\Game_01\Saved\SaveGames\CampSave.sav`。
   录完把原存档还原，别污染你自己的进度。
5. 在 Development 包（就是提交包）里排练两遍按键（1–3 拿、4–6 剪、E 搜、Space 闪避），确认没开任何探针参数后正式录。

### 2.3 画面检查（录前 3 秒）

- HUD 左上血条/经验/热度、右上 BAG、底部操作提示都在且不重叠；
- 顶部没有调试文字、没有探针输出；
- 分辨率确实是 1920×1080（OBS 预览里游戏画面铺满、无黑边）；
- 麦克风电平在 -12 到 -6 dB 之间，不爆音。

---

## 3. 镜头与流程编排（总长约 52 秒）

| # | 时间 | 画面 | 要让评委看到什么 | 操作 |
|---|---|---|---|---|
| 1 | 0:00–0:03 | **冷开场**：割草中，怪群围上来，闪避一次 | 3 秒内就让人看到"这是什么游戏" | WASD + `Space` |
| 2 | 0:03–0:08 | 营地全景（缓慢推近） | 风格与"战后营地"基调 | 不操作，等相机稳定 |
| 3 | 0:08–0:13 | 点建筑升级 → 顶部绿色提示 | 营地会成长（可视化反馈） | 点铁匠铺或诊所 |
| 4 | 0:13–0:17 | 剪辑室：加一张影片卡 → 提示 "Card added..." | 出发前就在"导演"这一局 | 点 `Add to the run` |
| 5 | 0:17–0:20 | 点 START RUN → 进入 L_Greybox → 落地 | 进入战斗 | 向前走两步 |
| 6 | 0:20–0:27 | 升级面板出现 → 先点红色 `Cut` → 卡片变两张 → 再 `TAKE` | **主题核心**：剪掉的选项归 Boss | 按 4/5/6 剪，再按 1–3 拿 |
| 7 | 0:27–0:33 | 顶部金色提示 `THE BOSS TAKES ...` 或 `CUT: ... belongs to the Boss` | 剪的后果立刻可见 | 不动，让提示停满 3 秒 |
| 8 | 0:33–0:39 | 再一次升级 → 选同名升级 → `2-STAR UP!`、左上 loadout 出现 `**` | **升星** | 选同名项 |
| 9 | 0:39–0:44 | 搜箱子 → `+N wood` / `+N stone`，BAG 与 Heat 上升 | 搜打撤的"搜"与风险 | 按 E |
| 10 | 0:44–0:49 | 进入绿色撤离圈 → 进度条 `EXTRACTING` → 结算页 `EXTRACTED` + 解锁项 | **撤离**与永久成长 | 站在圈内不动 |
| 11 | 0:49–0:52 | 点 `Return to Camp` → 仓库里出现解锁项 → 定格标题卡 | 闭环：一局改变下一局 | 点按钮 |

> 合计 52 秒。如果节奏偏慢，优先压缩第 2、3 段（营地），**不要**压缩第 6、8、10 段——它们是评分点。
> 如果超过 60 秒：把第 5 段压到 1 秒、第 9 段压到 3 秒；**冷开场（第 1 段）必须保留**。

### 3.1 拍摄顺序（不用按成片顺序录）

1. **A 段**：营地（含升级、加卡、START RUN）——一次录完；
2. **B 段**：一局里的升级 → 剪 → 升星 → 搜箱；
3. **C 段**：撤离 → 结算 → 回营地看仓库；
4. **D 段**：标题/结尾字卡（静态 3 秒，可后期做）；
5. **E 段**：冷开场的 3 秒（从 B 段里挑最猛的一段，或单独录一场开局前 3 秒）。

每段录 2–3 条，命名 `A-take1.mkv`、`B-take2.mkv`……

---

## 4. 解说要点（写词原则）

- 一句一个卖点，**先说"剪"**（主题），再说"升星"，最后说"撤离/永久解锁"。
- 用短句，避免从句；每句 ≤ 12 词，语速约每秒 2 词。
- 不要念 UI 上的英文原文，换成评委听得懂的解释。
- 全英文；不出现中文口音词、不出现"呃/那个"。
- 总词数控制在 100–120 词（52 秒）。

完整逐句台词、时间码、镜头指令、备用短句：**[03-video-script.md](03-video-script.md)**。

---

## 5. 素材与字幕安排

| 素材 | 处理 |
|---|---|
| 游戏画面 | OBS 录制，MKV → Remux MP4，不做锐化/滤镜 |
| 游戏音乐 | 直接用游戏内音乐（Kounine，CC-BY，已在 Esc 菜单署名），**后期不要再叠外部音乐** |
| 解说 | 单独音轨；对白处把游戏音乐降 6–8 dB |
| 字幕 | 英文 SRT，随视频上传；同时**烧录**一份硬字幕（评委可能静音看） |
| 标题卡 | 0:00 前 0.5 秒或直接叠在第 1 镜上：`DIRECTOR'S CUT` + 小字 `2026 Epic AI MegaJam` |
| 结尾卡 | 最后 2–3 秒：`Made solo in 4 days · Unreal Engine 5.8 + Claude Code` |
| 字号 | 字幕 42–48 px，底部留 10% 安全区；标题 72 px 以上 |

字幕规范：
- 每行 ≤ 42 字符，最多 2 行，每屏 ≥ 1.2 秒；
- 与解说**逐句对齐**，不要一句拆成三屏；
- 专有名词保留游戏内写法：`Director's Cut`、`2-STAR UP`、`EXTRACTED`。

---

## 6. 剪辑与导出

1. OBS → 文件 → Remux Recordings，把 MKV 转 MP4（不变码）。
2. 在 Clipchamp / Resolve 里按 §3 表拼接，删掉等待与失误。
3. 导出：MP4 / H.264 / 1920×1080 / 60 fps（或 30 fps）/ 16–20 Mbps / AAC 192 kbps。
4. 自查：时长 30–60 秒、分辨率 ≥1920×1080、有声音、无中文、无版权外链。
5. 用 FFmpeg 复核（装了才有）：
   ```powershell
   ffprobe -v error -select_streams v:0 -show_entries stream=width,height,r_frame_rate -show_entries format=duration -of default=noprint_wrappers=1 ".\DirectorsCut-gameplay.mp4"
   ```

### 6.1 上传

- YouTube：可见性 **不公开（Unlisted）**，语言 English，标题 `Director's Cut — 2026 Epic AI MegaJam (gameplay)`，简介写一句玩法 + 仓库链接 + devlog 链接。
- 或直接传 itch.io 页面（itch 支持嵌入 YouTube 链接）。
- 记录最终链接，填进 `docs/submission/submission-text.md` 的 "Gameplay video: TO ADD"。

### 6.2 视频与奖项的关系（判断；规则只规定时长与分辨率，未说视频影响哪些奖）

规则只规定时长（§1(e) 30–60 秒）与分辨率（§7(c) ≥1920×1080）、语言（§7(i) 英文），没有说视频影响哪些奖。但 7 项评分表里 Theme、Fun factor、Visuals、Audio 都靠视频传达，所以：

| 目标奖项 | 视频里必须有的东西 |
|---|---|
| Best Overall, UE / Solo Driver | 前 5 秒割草、一次剪卡、一次升星、一次撤离——已成片结构 |
| Glass Box（$2,500，靠 devlog） | 视频**不能替代** devlog，但 devlog 里要嵌这段视频；剪辑时顺手截 6–8 张 1080p 帧给 devlog 用 |
| Render Royalty（$2,500，已定不专门投入） | 只做两件免费的事：全片统一一个 LUT / 调色预设；封面图用最好看的一帧 |
| Best Student Project | 无额外要求 |

**结论：不要为奖项改镜头表。** 唯一新增动作是导出时多存一版无字幕母版（`DirectorsCut-master.mp4`）和 6–8 张高清截图——devlog 与 itch 页面都要用。

---

## 7. 没有麦克风时的方案

纯字幕版同样可用（**规则 §7(i) 要求视频内容全英文**，所以字幕与标题卡用英文；没有语音只是不录解说轨，不违反该条）。做法：
1. 不录解说轨；
2. 把 §4/03 的台词做成 SRT，烧录进画面；
3. 游戏音乐保持在 -18 LUFS 左右，作为背景；
4. 字幕停留时间按句长给足（≥ 字数 ÷ 15 秒）。

---

## 8. 常见问题

| 问题 | 原因 | 处理 |
|---|---|---|
| 录出来全黑 | 用了"显示器采集"且游戏独占全屏 | 改"游戏采集"或窗口化无边框（`FullscreenMode=1`） |
| 掉帧 / 卡顿 | CQP 太低或 x264 预设太重 | NVENC 用 CQP 20；x264 用 veryfast |
| 声音只有游戏没解说 | 只开了一条轨 | 音频设置里勾选两条轨，麦克风单独一条 |
| 字幕被平台裁掉 | 贴到底边 | 底部留 10% 安全区 |
| 时长 63 秒 | 营地段落太长 | 按 §3 的压缩顺序删 |
| 视频里出现调试信息 | 用了带探针参数的启动方式 | 正常双击 `Game_01.exe`，不加任何 `-Probe` / `-DevCheats` |
