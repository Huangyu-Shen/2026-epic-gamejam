# 界面（UI）

故事 007 的产出。全部用 C++ 写成，没有 widget 蓝图：每个控件在 `RebuildWidget()` 里用 `WidgetTree->ConstructWidget<T>()` 搭树。这样界面和代码在同一处审查，缺点是改外观要重新编译。路径省略前缀 `Game_01 5.8/Source/Game_01/`。

## 1. 文件

| 文件 | 内容 |
|---|---|
| [Public/UI/DirectorUi.h](../../Game_01%205.8/Source/Game_01/Public/UI/DirectorUi.h)、[Private/UI/DirectorUi.cpp](../../Game_01%205.8/Source/Game_01/Private/UI/DirectorUi.cpp) | 共用构件：面板、文字、进度条、按钮、提示区、调色板 |
| [RunHudWidget](../../Game_01%205.8/Source/Game_01/Private/UI/RunHudWidget.cpp) | 跑图界面 |
| [CampWidget](../../Game_01%205.8/Source/Game_01/Private/UI/CampWidget.cpp) | 营地界面 |
| [HudMessageSubsystem](../../Game_01%205.8/Source/Game_01/Private/UI/HudMessageSubsystem.cpp) | 屏幕提示的数据源 |

## 2. 共用构件 `DirectorUi`

一套样式（浅色字、半透明深色面板），之后换美术只改这里。

| 名称 | 签名 | 行为 |
|---|---|---|
| `MakePanel` | `UPanelWidget* MakePanel(UWidgetTree*, UCanvasPanel*, const FUiPlacement&, bool bScroll = false, UWidget** OutRoot = nullptr)` | 在画布上加一个带边框的面板，返回放内容的竖排框（或滚动框）；`OutRoot` 返回最外层边框，用来整块显示或隐藏 |
| `FUiPlacement` | `Anchor`、`Alignment`、`Position`、`Size`、`bFillHeight`、`BottomMargin` | `Size` 某分量为 0 表示按内容撑开；`bFillHeight` 为真时锚点固定在左侧、纵向从 `Position.Y` 拉伸到距底 `BottomMargin`，此时忽略 `Anchor` 和 `Alignment` |
| `AddText` | `UTextBlock* AddText(Tree, Parent, Text, Size = 18, Color)` | 自动换行，字号通过 `FSlateFontInfo` 设置 |
| `AddBar` | `UProgressBar* AddBar(Tree, Parent, Fill, Width, Height)` | 外面包一个 `USizeBox` 固定尺寸 |
| `AddButton` | `UUiActionButton* AddButton(Tree, Parent, Caption, Action, Index = 0)` | 按钮带一个文字标签，已调用 `Init` |
| `ReturnFocusToGame` | `void ReturnFocusToGame()` | `SetAllUserFocusToGameViewport()`；点完按钮必须调，否则按钮保持键盘焦点，后面按 Space（闪避）会"按下"它 |

`UUiActionButton`（继承 `UButton`）：点击后广播 `OnAction(FName Action, int32 Index)`，然后调 `ReturnFocusToGame()`。拥有者把 `OnAction` 绑定到自己的 `HandleAction`，用 `Action` 名字分辨是哪个按钮，用 `Index` 带下标（建筑类型、遗物下标、卡片下标）。

`FToastPanel`：顶部居中的提示区，4 行文字，每帧 `Update(GameInstance)` 从 `UHudMessageSubsystem::GetActive()` 取尚未过期的消息显示，没有则整块隐藏。

**为什么用 `RebuildWidget` 而不是 `NativeOnInitialized`**：没有蓝图的 `UUserWidget` 子类，`WidgetTree` 在 `RebuildWidget` 时已经可用，且早于 `NativeConstruct`，是搭树最稳的时机。

## 3. 跑图界面 `URunHudWidget`

创建者：[`AHeroCharacter::CreateHud`](../../Game_01%205.8/Source/Game_01/Private/Player/HeroCharacter.cpp)。每帧 `NativeTick` 读子系统刷新，不保存游戏状态。

| 区域 | 位置 | 内容 | 数据来源 |
|---|---|---|---|
| 状态 | 左上 | 血量文字和红条、等级与 XP 文字和蓝条、热度文字（含刷怪压力倍率）和橙条 | `UHealthComponent`、`URunProgressionSubsystem`、`ULootHeatSubsystem` |
| 背包 | 右上 | `已用/容量 slots  Iron N`，下面列 `[1] Aegis` 这样的遗物 | `ULootBagComponent` |
| 提示 | 顶部居中 | 搜刮、拾取被拒、使用遗物等短消息 | `UHudMessageSubsystem` |
| 撤离条 | 底部居中 | `EXTRACTING - stay in the zone` 加绿条；只有进度 > 0 才显示 | `AExtractionZone::GetProgress()`（第一次找不到时用 `TActorIterator` 取第一个） |
| 操作提示 | 左下 | `WASD move  Space dodge  E search a crate  1-6 use a relic` | 静态文字 |
| 升级面板 | 居中 | 标题加 3 个按钮；只有 `IsOfferPending()` 且一局进行中才显示；多余的按钮折叠 | `URunProgressionSubsystem::GetOfferDescriptions()` |
| 结算页 | 居中 | `EXTRACTED` 或 `YOU DIED`、数字、`Return to Camp` 按钮；一局结束后显示 | `URunOutcomeSubsystem::GetLastResult()` |

动作：`Offer`（`PickOffer(Index)`）、`Camp`（`OpenLevel(L_Camp)`）。

没有搜索进度条：容器搜索是瞬间完成的（`ALootContainer::Interact` 同步返回），只做了撤离进度条。

## 4. 营地界面 `UCampWidget`

创建者：[`ACampPawn::BeginPlay`](../../Game_01%205.8/Source/Game_01/Private/Camp/CampPawn.cpp)。左侧一个撑满高度的滚动面板（距底留 90 像素），左下角另一个固定面板放 START RUN，所以它永远点得到。

面板内容：Iron、最近一次操作的结果文字、建筑（名称、等级、当前加成、升级按钮，满级时禁用）、仓库（每个遗物一个拆解按钮）、剪辑室（每张卡一行说明加"加入/移出"按钮）。

**状态签名**：`MakeSignature()` 把提示文字、Iron、仓库遗物、建筑等级、待用卡片拼成一个字符串；`NativeTick` 发现它变了才调 `Rebuild()` 清空重建所有行。这样不需要给每个控件绑变化事件，缺点是每次变化整个面板重建，按钮的悬停状态会丢。

动作：`Upgrade`（`TryUpgrade`）、`Dismantle`（`DismantleRelic`）、`Card`（加入或移出）、`Start`（`OpenLevel(L_Greybox)`）。

3D 场景里点建筑也能升级（`ACampPawn::OnClick`，用 `GetHitResultUnderCursor`），结果走 `UHudMessageSubsystem` 的提示区。

## 5. 输入模式

两个界面都用 `FInputModeGameAndUI`（`DoNotLock`、`SetHideCursorDuringCapture(false)`）并显示鼠标：界面根控件设为 `SelfHitTestInvisible`，只有按钮和面板吃点击，其余点击穿透到 3D 场景（营地点建筑要用）。

## 6. 屏幕提示 `UHudMessageSubsystem`

[HudMessageSubsystem.h](../../Game_01%205.8/Source/Game_01/Public/UI/HudMessageSubsystem.h)。GameInstance 子系统，保存最近 4 条消息。

| 函数 | 行为 |
|---|---|
| `static void Post(const UObject* WorldContext, const FString& Text, const FLinearColor& Color, float Seconds = 2.5f)` | 没有 World 或 GameInstance（比如编辑器预览世界）时什么都不做 |
| `TArray<FHudMessage> GetActive() const` | 过滤掉已过期的，从旧到新 |

时间用 `FPlatformTime::Seconds()`（真实时间），所以游戏暂停时消息也会过期。调用点：`ULootBagComponent`（拾取被拒、使用遗物）、`ALootContainer`（搜刮、已搜过）、`AHeroCharacter`（无遗物）、`ACampPawn`（点击升级）。

这套消息取代了引擎的 `AddOnScreenDebugMessage`，后者固定画在屏幕左上角，会压在血条上。
