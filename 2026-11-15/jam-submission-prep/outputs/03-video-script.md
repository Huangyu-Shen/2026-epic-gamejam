# 录制脚本（可直接照读）

> **怎么用**：只有 *Italic* 引号里的英文是**要念出来的台词**；中文是动作与时机提示，不要念。
> 每条台词都标了时间码，跟着 OBS 计时或后期对齐即可。成片目标 **52 秒**。
> 参数、软件设置见 [02-video-plan.md](02-video-plan.md)。字幕文件：[03-video-script.srt](03-video-script.srt)。

---

## 0. 开录前 60 秒检查

- [ ] 包是 Development 提交包（官方要求 development build），双击 `Windows\Game_01.exe` 启动，**没有**加任何启动参数（`-OfferProbe` 之类会带出调试痕迹）
- [ ] 分辨率 1920×1080，无边框，画面铺满
- [ ] 已替换成"有进度"的存档（有石头/木头可升级，剪辑室有影片卡）
- [ ] OBS：两条音轨（游戏 / 麦克风），MKV，1080p60，预览无黑边
- [ ] 通知已关，鼠标已隐藏，QQ/微信/Discord 已退
- [ ] 心里过一遍按键：`WASD` 移动、`Space` 闪避、`E` 搜箱、`1-3` 拿、`4-6` 剪、`Esc` 菜单

---

## 1. 主脚本（52 秒）

> 官方 Tips 要求"前 5 秒就是 gameplay"，所以第 0 镜是 **3 秒冷开场，不说话**，只留音效与音乐。
> 台词从第 3 秒才开始。

| # | 时间码 | 画面 / 你的操作 | 照读台词（英文） |
|---|---|---|---|
| 0 | 0:00–0:03 | **冷开场**：割草最猛的一段，怪群围上来，按一次 `Space` 闪避。**不念台词。** | —（只有游戏音效） |
| 1 | 0:03–0:08 | 切营地全景，鼠标不动，等相机停稳。可缓慢小幅移动鼠标让镜头有呼吸感。 | *"Every run of Director's Cut starts in camp."* |
| 2 | 0:08–0:13 | 点击铁匠铺（或诊所）升级；等顶部绿色提示 `<Building> upgraded to level N` 出现。 | *"Upgrade a building. The camp grows stronger every run."* |
| 3 | 0:13–0:17 | 滚到左侧面板下方 `EDITING ROOM`，点一张卡的 `Add to the run`，等 `Card added to the next run`。 | *"Slot a film card: harder run, bigger payout."* |
| 4 | 0:17–0:20 | 点左下角绿色 `START RUN`，进入 L_Greybox。向前走两步。 | *"Then you're dropped into the horde."* |
| 5 | 0:20–0:27 | 升级面板弹出。**先**点红色 `Cut`（或按 4/5/6），停 1 秒；**再**点绿色 `TAKE`（或按 1–3）。 | *"Every level-up is an editing decision: take one upgrade... or cut one first."* |
| 6 | 0:27–0:33 | 顶部出现金色 `CUT: <name> now belongs to the Boss`（或 `THE BOSS TAKES ...`）。不要动，让它停满 3 秒。 | *"What you cut leaves your pool for good — and becomes the boss's weapon."* |
| 7 | 0:33–0:39 | 下一次升级：选**和刚才同名**的升级，触发 `2-STAR UP!`，左上 loadout 行出现 `**`。 | *"Three copies make a star. Two stars hit harder."* |
| 8 | 0:39–0:44 | 走到箱子按 `E`，等 `Searched: +N wood` / `+N stone`；右上 BAG 与左上 Heat 同时变化。 | *"Loot raises your heat. Higher heat, stronger enemies."* |
| 9 | 0:44–0:49 | 走进绿色撤离圈，站住不动，等底部绿色进度条 `EXTRACTING` 走满，出现 `EXTRACTED`。 | *"Reach extraction alive, and you keep it all."* |
| 10 | 0:49–0:52 | 结算页停留 → 点 `Return to Camp` → 仓库 `STORAGE` 出现新解锁项 → 定格。 | *"Director's Cut. Solo, in four days."* |

**结尾字卡（可选，叠在最后一镜上，不念）**

```
DIRECTOR'S CUT
Made solo in 4 days · Unreal Engine 5.8 · Claude Code
```

> 若第 7 步没有出现同名升级：先把这一局打完，重开一局再录 B 段。**不要**为了凑升星去用调试参数，画面会带出调试痕迹。

---

## 2. 60 秒加长版（时间宽松时用）

在第 9 步之后插一句，把"永久解锁"讲清楚：

| 时间码 | 画面 | 照读台词 |
|---|---|---|
| 0:44–0:49 | 走进撤离圈，进度条走满，出现 `EXTRACTED` | *"Reach extraction alive, and you keep it all."* |
| 0:49–0:54 | 结算页显示 `Unlocked for every later run: <option>` | *"Rescue a lost film reel, and it unlocks a new option for every run after."* |
| 0:54–0:57 | 回营地，`STORAGE` 列表出现新项，镜头停留 | *"One run edits the next."* |
| 0:57–0:59 | 定格 + 字卡 | （不念，让字卡收尾） |

---

## 3. 45 秒短版（时间不够 / 平台限长时用）

删掉营地升级与影片卡两段，**保留 3 秒冷开场**：

| 时间码 | 画面 | 照读台词 |
|---|---|---|
| 0:00–0:03 | 冷开场：割草 + 闪避 | —（只有游戏音效） |
| 0:03–0:07 | 营地全景 | *"Every run of Director's Cut starts in camp."* |
| 0:07–0:10 | 点 START RUN | *"Then you're dropped into the horde."* |
| 0:10–0:17 | 升级：先剪后拿 | *"Every level-up is an editing decision: take one upgrade, or cut one first."* |
| 0:17–0:23 | Boss 提示 | *"What you cut leaves your pool for good — and becomes the boss's weapon."* |
| 0:23–0:29 | 同名升级 → 2-STAR UP | *"Three copies make a star. Two stars hit harder."* |
| 0:29–0:34 | 搜箱 + Heat | *"Loot raises your heat. Higher heat, stronger enemies."* |
| 0:34–0:40 | 撤离 → EXTRACTED | *"Reach extraction alive, and you keep it all."* |
| 0:40–0:45 | 回营地 → 字卡 | *"Director's Cut. Solo, in four days."* |

---

## 4. 临场备用句（说错时替换，不要重录整段）

- 卡壳时："One run edits the next."（通用过渡句，任何镜头都能接）
- 剪的提示没出现时："Whatever you cut, the boss keeps."（同义，更短）
- 升星没触发时："Three copies make a star."（只讲机制，不承诺画面）
- 结尾忘词时："Director's Cut. Solo, in four days."（永远是安全收尾）

---

## 5. 后期对齐

1. 把每段 MKV 按 §1 的时间码拼成 52 秒；
2. 台词轨单独一条，逐句对到画面上（宁早 0.1 秒，不要晚）；
3. 解说处游戏音乐降 6–8 dB，句间恢复；
4. 烧字幕 + 另存 SRT 一起上传；
5. 录完立刻用 `scripts\archive-materials.ps1` 归档原始素材（见 [README.md](README.md)）。
