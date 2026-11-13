# 已知问题与技术债

只列已对照代码确认的问题。路径省略前缀 `Game_01 5.8/Source/Game_01/`。按影响排序。

## 1. 数据与存档

### 1.1 存档版本字段从不检查
- **位置**：[CampSaveGame.h](../../Game_01%205.8/Source/Game_01/Public/Flow/CampSaveGame.h) 的 `SaveVersion`，[CampSaveSubsystem.cpp](../../Game_01%205.8/Source/Game_01/Private/Flow/CampSaveSubsystem.cpp) 的 `Initialize`。
- **现象**：注释写"布局不兼容时升版本"，但读取代码只检查文件存在和类型转换，从不读 `SaveVersion`。
- **影响**：目前只有版本 1，没问题。以后改字段（例如加新建筑），旧存档会被无声地按新结构读，缺的字段变成默认值。
- **建议**：读取时比较版本，不同则迁移或明确放弃。

### 1.2 升级时会写两次存档，中间状态不一致
- **位置**：[`UCampMetaSubsystem::TryUpgrade`](../../Game_01%205.8/Source/Game_01/Private/Camp/CampMetaSubsystem.cpp)、[`UCampStockSubsystem::SpendIron`](../../Game_01%205.8/Source/Game_01/Private/Extraction/CampStockSubsystem.cpp)。
- **现象**：`SpendIron` 先广播 `OnStockChanged`，存档子系统立刻写盘，此时等级还没加；随后等级加一再广播 `OnMetaChanged` 再写一次。
- **影响**：两次写之间进程被杀，存档里是"扣了 Iron 没升级"。窗口极小（同一帧内），但存在。
- **建议**：升级流程先改完两处状态，再统一存一次。

### 1.3 拆解遗物的 Iron 写死
- **位置**：[`UCampMetaSubsystem::RelicDismantleIron`](../../Game_01%205.8/Source/Game_01/Public/Camp/CampMetaSubsystem.h)（值 8）。
- **现象**：项目的编码规范要求"玩法数值必须数据驱动"，这个值在头文件里，不在 `DT_Buildings` 里。
- **影响**：调平衡要改代码并重新编译。

### 1.4 `ULootConfig` 资产不存在
- **位置**：[`ULootBagComponent::GetConfig`](../../Game_01%205.8/Source/Game_01/Private/Loot/LootBagComponent.cpp)。
- **现象**：背包容量、热度、遗物与容器数值都来自 `ULootConfig`，工程里没有这个资产，运行时用内联默认值（见 [data-and-storage.md](data-and-storage.md#2-调参数据表)）。
- **影响**：这部分数值不能像其他 9 张表一样在 `Director Tuning` 里调；要调就得新建资产并指给英雄的 `Bag->Config`。

### 1.5 `project.yaml` 仍指向热身项目
- **位置**：[project.yaml](../../project.yaml) 的 `commands.build/test/run/smoke` 和 `engine.language`。
- **现象**：四条命令里的工程路径是 `Game_Jam_01/Game_Jam_01.uproject`，`engine.language` 写的是 `"Blueprint"`，而实际工程是 `Game_01 5.8/`、C++。
- **影响**：技能（如 `/smoke-check`、`/perf-profile`）按这些命令运行会指向不存在于仓库的工程。改 `project.yaml` 需要用户批准。

## 2. 视觉

### 2.1 英雄、敌人、经验宝石是灰色，染色无效
- **位置**：[`AHeroCharacter::BeginPlay`](../../Game_01%205.8/Source/Game_01/Private/Player/HeroCharacter.cpp)、[`AEnemy::Init`](../../Game_01%205.8/Source/Game_01/Private/Enemies/Enemy.cpp)、[`AXpPickup::BeginPlay`](../../Game_01%205.8/Source/Game_01/Private/Progression/XpPickup.cpp)。
- **现象**：这三处对圆柱或立方体创建动态材质并设置 `Color` 参数（注释写"英雄蓝、敌人红、Boss 紫、宝石绿"），但游戏里全是灰的。
- **原因**：引擎基础形状网格自带的材质没有 `Color` 参数，`SetVectorParameterValue` 静默失败。营地的 [`ACampBuilding`](../../Game_01%205.8/Source/Game_01/Private/Camp/CampBuilding.cpp) 和 [`ACampWarehouse`](../../Game_01%205.8/Source/Game_01/Private/Camp/CampWarehouse.cpp) 显式 `SetMaterial` 成 `/Engine/BasicShapes/BasicShapeMaterial`，所以它们着色正常（截图可见）。
- **旧 wiki 的说法是错的**：旧文档说"`BasicShapeMaterial` 无 `Color` 参数"，实际相反，`BasicShapeMaterial` 有，缺的是网格默认材质。
- **建议**：这三处创建动态材质前先 `SetMaterial(0, BasicShapeMaterial)`。

### 2.2 营地提示文字不会自己消失
- **位置**：[`UCampWidget`](../../Game_01%205.8/Source/Game_01/Private/UI/CampWidget.cpp) 的 `Message`。
- **现象**：面板顶部那行"升级成功/Iron 不足"一直显示到下一次操作。
- **影响**：只是观感；3D 点击走的是会过期的 `UHudMessageSubsystem` 提示。

## 3. 遗留与死代码

| 项 | 位置 | 说明 |
|---|---|---|
| 占位角色和游戏模式 | [StandIn/](../../Game_01%205.8/Source/Game_01/Public/StandIn/) | 故事 001 的一次性占位，注释写"故事 003 替换"，已无人使用，但仍参与编译 |
| `EnhancedInput` 模块 | [Game_01.Build.cs](../../Game_01%205.8/Source/Game_01/Game_01.Build.cs) | 声明了，源码没有任何 `#include` |
| 清除旧升级文字的调用 | [`ResetProgression`、`PickOffer`](../../Game_01%205.8/Source/Game_01/Private/Progression/RunProgressionSubsystem.cpp) | 对键 2200 到 2203 调 `AddOnScreenDebugMessage(…, TEXT(""))`，对应的旧调试文字早已删掉，这些调用现在没有作用 |
| `StartNewRun` 的调用方 | [HeroCharacter.cpp](../../Game_01%205.8/Source/Game_01/Private/Player/HeroCharacter.cpp) | 原来由 N 键触发，现在只在 `BeginPlay` 里调一次；游戏里没有"再来一局"，每局都是重新加载跑图地图 |
| 武器的调试绘制 | [AutoWeaponComponent.cpp](../../Game_01%205.8/Source/Game_01/Private/Combat/AutoWeaponComponent.cpp) | 每次开火 `DrawDebug*` 画 0.12 秒示意线，Shipping 构建会被引擎编译掉，所以 Shipping 包里没有武器可视反馈 |
| `DeleteSaveAndReset()` | [CampSaveSubsystem.cpp](../../Game_01%205.8/Source/Game_01/Private/Flow/CampSaveSubsystem.cpp) | 没有任何调用方 |

## 4. 架构与健壮性

- **无自动测试**：仓库里没有任何自动化测试文件。[`FRunSettlement::Compute`](../../Game_01%205.8/Source/Game_01/Private/Extraction/ExtractionTypes.cpp) 的注释写"纯函数，可不开编辑器测试"，但没有测试。`qa.level: minimal` 下测试是免的，所以这是被允许的缺口，不是违规。
- **对"没有 GameInstance 的世界"不设防**：`GetWorld()->GetGameInstance()->GetSubsystem<…>()` 在编辑器预览世界会空指针。故事 007 里 `UFilmCardSubsystem::Initialize` 因此崩溃过一次，已改成 `UGameInstance::GetSubsystem<T>(GetWorld()->GetGameInstance())`（对 null 返回 null）。同样的老写法还在 [`UAutoWeaponComponent::FireWeapon`](../../Game_01%205.8/Source/Game_01/Private/Combat/AutoWeaponComponent.cpp)、[`URunOutcomeSubsystem::EndRun`](../../Game_01%205.8/Source/Game_01/Private/Extraction/RunOutcomeSubsystem.cpp)、[`ACampBuilding`](../../Game_01%205.8/Source/Game_01/Private/Camp/CampBuilding.cpp)，它们只在游戏运行时执行，目前安全。写新的世界子系统时，不要在 `Initialize` 里直接解引用 GameInstance。
- **地图名写死**：`L_Camp`、`L_Greybox` 以字符串形式出现在 [CampWidget.cpp](../../Game_01%205.8/Source/Game_01/Private/UI/CampWidget.cpp)、[RunHudWidget.cpp](../../Game_01%205.8/Source/Game_01/Private/UI/RunHudWidget.cpp)，改名要同步。
- **依赖有环、无检查**：见 [dependencies.md](dependencies.md#1-模块之间怎么依赖)。
- **`ModelContextProtocol`、`AllToolsets` 插件对所有目标启用**：开发工具会被带进打包，发行前应限制到 Editor。
- **营地面板整块重建**：状态签名变化就清空重建，点击后悬停状态丢失；列表很长（遗物很多）时仍靠滚动框。

## 5. 版本对比

最近的发布标签 `jam-start` 早于 UE 工程，无法对比存档或配置键的兼容性。存档格式目前只有一个版本。见 [data-and-storage.md](data-and-storage.md#13-版本兼容性)。
