# Key Classes and Functions

Only the classes, signatures, edge cases and "why" you must know when reading the code are listed. Paths omit the prefix `Game_01 5.8/Source/Game_01/`, and class names link to their implementation files. For UI classes see [ui.md](ui.md); for the save see [data-and-storage.md](data-and-storage.md).

## 1. Hero `AHeroCharacter`

Implementation: [HeroCharacter.cpp](../../Game_01%205.8/Source/Game_01/Private/Player/HeroCharacter.cpp), header [HeroCharacter.h](../../Game_01%205.8/Source/Game_01/Public/Player/HeroCharacter.h).

Public tuning fields: `InteractRange` 250 cm, `BaseWalkSpeed` 600 cm/s, `CameraDistance` 1400 cm, `Dodge` (`FDodgeTuning`), and the components `Bag` (`ULootBagComponent`), `Health` (`UHealthComponent`), `Weapon` (`UAutoWeaponComponent`). `IsDead()` forwards to `Health`.

| Member | Behavior | Edge cases |
|---|---|---|
| `bool CanAct() const` (private) | True only while a run is in progress, the hero is alive and no level-up choice is pending | Returns false when there is no `URunOutcomeSubsystem` |
| `void StartRun()` (private) | Clears enemies, resets kill count and spawning, clears heat, clears the bag, sets max health = base + Clinic bonus (this also refills health), resets progression, resets dodge, `StartNewRun`, applies film cards | Called once in `BeginPlay`; there is no "another run" key, the next run happens by reloading the run map |
| `void CreateHud()` (private) | Creates `URunHudWidget`, sets it to `SelfHitTestInvisible` (only buttons take clicks), shows the mouse, input mode `GameAndUI` without locking the mouse | Falls back to `GetPlayerController(0)` when the controller is null; returns early (and sets no input mode) if there is no controller or the HUD already exists |
| `bool TryDodge()` | Direction is the current WASD direction, or the facing direction if no key is pressed; dash 0.18 seconds, speed 1800, cooldown 0.8 seconds (counted from the dodge start), invulnerability 0.25 seconds (`GrantInvulnerability`: the longer window wins) | Returns false on `!CanAct()`, while dashing, or during cooldown |
| `bool SearchNearestContainer()` | Finds the nearest unsearched `ALootContainer` within `InteractRange` (distance on the XY plane) and calls `Interact` | Returns false when no container is in range, or when `Interact` refuses (bag too full for the rolled loot: the container stays searchable) |
| `float ReceiveEnemyDamage(float)` | Hands the damage straight to `UHealthComponent::ApplyDamage` (there are no bag buffs any more) | Returns the actual damage, 0 while invulnerable. The comment in the header still mentions a "damage-taken multiplier" and is stale |
| `void HandleDied()` | Bound to `OnDied`, calls `ReportPlayerDied(Bag)` | - |
| `Tick` | Reads WASD (`IsInputKeyDown`); move speed = `BaseWalkSpeed x passive move-speed multiplier`; during a dash it uses a sweeping `AddActorWorldOffset` (walls stop it) instead of normal movement | Input is zeroed when `CanAct()` is false, but a dash already under way finishes anyway |

Keys: Space dodge, E search (only when `CanAct()`), K suicide (debug, `Health->Kill()`, ignores god mode), Esc (pause menu: `UPauseMenuWidget`), 1 to 3 take one of the three level-up options (`ChooseOption`) and 4 to 6 cut one of them (`CutFromOffer`); both do nothing unless an offer is pending. Lost film reels cannot be used, so there is no bag-slot key; `UseRelicSlot` was removed in story 008 (keys 4 to 6 came back in v3 as the cut keys).

## 2. Bag and lost film reels `ULootBagComponent`

Implementation: [LootBagComponent.cpp](../../Game_01%205.8/Source/Game_01/Private/Loot/LootBagComponent.cpp).

Capacity: Iron takes `ceil(Iron / IronPerSlot)` slots and each reel takes one slot; defaults are 6 slots and 10 Iron per slot (`FLootTuning`).

| Function | Behavior | Errors or edge cases |
|---|---|---|
| `EBagAddResult CanAdd(int32 Iron, bool bWithRelic)` | Does not modify anything; checks whether it fits | `Iron < 0`, or `Iron == 0` with no reel -> `InvalidItem`; over capacity -> `BagFull` |
| `TryAddLoot(int32 Iron, bool bAddRelic, ERelicType)` | Atomically adds Iron and at most one lost film reel; if there is a reel, `AddRarePickupHeat`; broadcasts `OnBagChanged` | On failure nothing is added, a message `Pickup refused: ...` is shown and `OnPickupRefused` is broadcast |
| `TryAddIron(Iron)` / `TryAddRelic(ERelicType)` | Inline shortcuts: `TryAddLoot(Iron, false, ...)` and `TryAddLoot(0, true, type)` | Same errors |
| `GetRelics()` / `GetRelicCount()` | The reels in the bag (a copy), and how many | A reel is read-only cargo: nothing in a run can use or consume it |
| `GetUsedSlots()`, `GetCapacitySlots()`, `IsFull()` | Slot arithmetic above; `IsFull` is `used >= capacity` | - |
| `ClearBag()` | Clears Iron and the reels, broadcasts `OnBagChanged` | Does not touch heat (heat is reset separately at settlement) |
| `GetConfig()` | `Config` if assigned, otherwise a lazily created `ULootConfig::CreateFromSettings(this)` kept in a transient field | Never null |
| `BeginPlay` | Pushes the tuning into `ULootHeatSubsystem` and logs `LOOTCFG source=table or c++ defaults, capacity=..., reelRows=...` | `source=c++ defaults` means the `LootTuningTable` is unset or its row `Default` was not found |

Story 008 removed `UseRelic`, `GetActiveModifier`, `GetActiveBuffs`, the `OnRelicUsed` and `OnBuffsChanged` delegates, `FLootStatModifier`, `FActiveRelicBuff` and the bag's Tick (none of these names occurs in `Source/` any more). The bag no longer ticks at all.

Default reel rows (inline defaults in [LootConfig.cpp](../../Game_01%205.8/Source/Game_01/Private/Loot/LootConfig.cpp); `DT_Reels` is the table that overrides them, and since story 009 it maps the same way): Fast Cut (`ERelicType::Swiftstep`) unlocks the option `Haste`, Wide Shot (`Warbrand`) unlocks `Magnet` and Chain Scene (`Aegis`) unlocks `Chain`. The option id is the row name of the weapon or passive it unlocks (see section 7). Every row has `DropWeight` 1. `FRelicRow::IsDroppable` is the drop rule: the row names an option, the camp has not unlocked that option, and the bag does not already hold that reel type. A row whose `UnlocksOption` is `None` never drops. `ULootConfig::PickRandomRelic(unlocked, bag, out)` picks among the droppable rows by `DropWeight` (negative weights count as 0; if every weight is 0 the pick is uniform) and returns false when nothing can drop. Containers: supply crate Iron 3 to 6, reel chance 0.2; vault Iron 10 to 20, reel chance 1.0 (the field is still called `RelicChance`). Consequence with the shipped data: a reel type stops dropping once its option is unlocked or its reel is already in the bag, and once all three options are unlocked no container drops a reel, not even a vault.

Where the numbers come from: `ULootConfig::CreateFromSettings` reads three tables from `Director Tuning`: `LootTuningTable` (`DT_LootTuning`, the row named `Default`, an `FLootTuning`), `LootContainerTable` (`DT_LootContainers`) and `ReelTable` (`DT_Reels`). A table that is not set, or (reel and container tables) has no rows, keeps the C++ defaults above. Container and reel rows are matched by their `ContainerType` / `RelicType` field, not by row name (the shipped rows are named `SupplyCrate`, `Vault`, `FastCut`, `WideShot`, `ChainScene`, but the names do not matter); for reels the first row of a type wins. A container table that has rows but none for a container's type does not fall back per type: `FindContainerRow` fails and that container cannot be searched. A `ULootConfig` data asset assigned to the bag's `Config` takes precedence, but no such asset exists in `Content/Data`.

### `ALootContainer`

[LootContainer.cpp](../../Game_01%205.8/Source/Game_01/Private/Loot/LootContainer.cpp). `Interact(AActor*)` returns true only when the search completed. It returns false for a null or already-searched container (the latter posts "Already searched"), an instigator with no bag (a controller is also accepted and its pawn's bag used), a missing container row, or a bag that refuses the loot.

- The roll happens once and is remembered (`bRolled`), so retrying after `BagFull` cannot re-roll: Iron is `RandRange(IronMin, max(IronMin, IronMax))`, then a reel is rolled with `FRand() < RelicChance` and `PickRandomRelic(camp's unlocked options, bag's reels)`.
- If the remembered reel type has meanwhile entered the bag from another container, the reel is dropped from the roll and only the Iron is added; if that leaves 0 Iron the container opens empty.
- A roll with no reel and 0 Iron opens an empty container without touching the bag.
- On success it posts `Searched: +N Iron + lost reel X` and broadcasts `OnSearched`. `ConfigOverride` (a `ULootConfig`) beats the bag's config.
- `ShowOpened` flattens the cube to a quarter of its height as the "opened" cue.

## 3. Heat `ULootHeatSubsystem`

[LootHeatSubsystem.cpp](../../Game_01%205.8/Source/Game_01/Private/Loot/LootHeatSubsystem.cpp).

| Function | Behavior |
|---|---|
| `float GetSpawnPressureMultiplier() const` | `Lerp(SpawnPressureAtZeroHeat, SpawnPressureAtMaxHeat, heat/max heat)`, default 1.0 to 3.0 |
| `AddHeat(float)` | Clamped to 0 through `MaxHeat`; if the change is 0, `OnHeatChanged(NewHeat, Delta)` is not broadcast |
| `AddRarePickupHeat()` | Adds `HeatPerRarePickup` (default +15, `MaxHeat` default 100) when a lost film reel is picked up. `ReduceHeatForRelicUse()` and `HeatReductionOnRelicUse` were removed: nothing lowers heat during a run, and `ResetHeat()` (which is `AddHeat(-Heat)`, so it broadcasts when heat was above 0) runs at the start of a run and at the end of one |
| `SetTuning` | The bag pushes its own config in at `BeginPlay` (and the current heat is re-clamped to the new `MaxHeat`); the config is built from the `LootTuningTable` in `Director Tuning` (row `Default`), or from the C++ defaults when that row is missing |

## 4. Health `UHealthComponent`

[HealthComponent.cpp](../../Game_01%205.8/Source/Game_01/Private/Combat/HealthComponent.cpp).

| Function | Behavior |
|---|---|
| `float ApplyDamage(float)` | Returns 0 if already dead, invulnerable, in god mode (`bGodMode`), or `Amount <= 0`; otherwise subtracts `min(Amount, Health)` and broadcasts `OnDamaged`, broadcasts `OnDied` on death, and if not dead grants `HitInvulnerabilitySeconds` (0.25) of invulnerability |
| `Kill()` | Ignores god mode and invulnerability, zeroes health, broadcasts `OnDamaged` and `OnDied`; no-op if already dead |
| `SetMaxHealth(float)` | Sets the maximum (at least 1), heals to full, clears invulnerability; the Clinic bonus uses it |
| `IncreaseMaxHealth(float)` | Raises the cap and heals by the same amount (ignored for `Amount <= 0`); the Vitality passive uses it |
| `GrantInvulnerability(float)` | Invulnerable until `now + Seconds`, unless a longer window is already running; used by the dodge |
| `GetBaseMaxHealth()` | Read from the `Hero` row of `HealthTuningTable` (an `FHealthTuning`: max health and hit invulnerability) at `BeginPlay` and remembered, so the Clinic bonus does not stack; `BeginPlay` also calls `ResetHealth()` |

## 5. Auto weapons `UAutoWeaponComponent`

[AutoWeaponComponent.cpp](../../Game_01%205.8/Source/Game_01/Private/Combat/AutoWeaponComponent.cpp). When `bEnabled`, no level-up choice is pending, the hero is alive and a run is in progress, `TickComponent` iterates the four weapons (`EWeaponType::Count`) and fires any with level > 0 whose cooldown is ready; the Chain has level 0 until the player has unlocked it and picked it. A weapon's "level" is the number of copies the run holds (see section 7). Cooldowns only count down while the component is allowed to fire, so a soft-pause also pauses them. The cooldown restarts after every fire attempt, including a Blade that found no target, and is `Row.Interval / max(attack speed multiplier, 0.1)`: the Haste passive (`URunProgressionSubsystem::GetAttackSpeedMultiplier`) divides every weapon's interval.

`FireWeapon` returns how many enemies the shot hit, and `TickComponent` then broadcasts `OnWeaponFired(EWeaponType, int32 hits)` (`FOnWeaponFired`, a native multicast delegate declared in [AutoWeaponComponent.h](../../Game_01%205.8/Source/Game_01/Public/Combat/AutoWeaponComponent.h)) after every fire attempt, including one that hit nothing. Today only the `-OfferProbe` binds it; it is there for the hit feedback and audio of a later story.

Damage formula: `(Row.Damage + Row.DamagePerLevel x (level - 1) + Smithy bonus) x passive damage multiplier x star damage multiplier`. Stars also widen the reach: `Row.Range`, `Row.LineHalfWidth` and `Row.ChainRange` are multiplied by the star range multiplier (1.25 at two stars, 1.6 at three) before firing, so a star-up is visible on screen.

| Weapon | C++ fallback values (`DefaultWeapon` in [RunProgressionSubsystem.cpp](../../Game_01%205.8/Source/Game_01/Private/Progression/RunProgressionSubsystem.cpp)) | Hit rule |
|---|---|---|
| Blade | Damage 15, +8 per level, interval 0.4 seconds, range 600 | The nearest single enemy within range (distance on the XY plane) |
| Bolt | Damage 20, +10 per level, interval 0.9 seconds, range 1200, half-width 80 | A line toward the nearest enemy within range (or the facing direction if none), piercing, length = range |
| Nova | Damage 25, +12 per level, interval 1.6 seconds, range 350 | All enemies within the radius (= range) |
| Chain | Damage 12, +6 per level, interval 1.1 seconds, range 700, `ChainJumps` 3, `ChainRange` 350, `bStartsLocked` true | The path is picked first: the nearest enemy within `Range`, then up to `ChainJumps` times the nearest not-yet-hit enemy within `ChainRange` of the last one chosen (distance on the XY plane). Only then is damage dealt to every enemy on the path, because a kill removes an enemy from the manager's list. A green debug line is drawn from the hero through the path |

The values the game actually uses are in `DT_Weapons` (rows `Blade`, `Bolt`, `Nova`, `Chain`, keyed by name, not by the row's own type field) whenever that table loads; the shipped numbers differ from the fallbacks above for the older rows and the assets are binary, so read them in the editor. The values exported when story 009 was finished: Blade damage 20, +10 per level, interval 0.35, range 220; Bolt 12, +7, 0.55, range 900, half-width 40; Nova 14, +8, 0.8, range 320; Chain 12, +6, 1.1, range 700, 3 jumps, chain range 350.

On firing, `DrawDebugLine/Circle` draws a 0.12-second indicator (0.3 seconds for the Chain); the engine compiles these calls out in release (Shipping) builds.

## 6. Enemies

### `AEnemy`

[Enemy.cpp](../../Game_01%205.8/Source/Game_01/Private/Enemies/Enemy.cpp). A cube with no collision and no tick; `UEnemyManagerSubsystem` moves it. `Init(FEnemyTuning)` sets health and scale and colours it (Grunt red, Boss purple). `ApplyDamage` does nothing once health is 0; at 0 it calls `NotifyEnemyKilled` and then `Destroy()`. `EndPlay` unregisters from the manager only on a real destroy, not on world teardown.

### `UEnemyManagerSubsystem`

[EnemyManagerSubsystem.cpp](../../Game_01%205.8/Source/Game_01/Private/Enemies/EnemyManagerSubsystem.cpp). Public helpers: `FindNearestEnemy(Location, Range)` (XY distance, inclusive), `GetEnemies()`, `GetAliveCount()`, `GetKillCount()`, `ResetKillCount()`, `ClearAll()` (destroys everything without counting kills) and the `OnEnemyKilled(Kind, Location)` delegate that `URunProgressionSubsystem` listens to. `FSwarmTuning` comes from the `Swarm` row of `SwarmTuningTable`.

`Tick` returns at once while the level-up panel is shown, when there is no hero pawn, or when there are no enemies. Otherwise three passes:

1. **Chase and damage**: moves toward the hero in the XY plane and stops at `ContactRange x StopRangeFraction` (default 0.8), so enemies form a ring instead of stacking; when `Dist <= ContactRange` and the cooldown is ready, `ReceiveEnemyDamage` is called. After a run ends (`bCanHurt` is false) they still move, they just no longer hurt.
2. **Gridding**: positions are put into a grid by `SeparationCellSize` (default 250), which cuts pairwise checks from O(n^2) to only the 3x3 neighboring cells. The cell size must be at least the largest pair of body radii (two Bosses are 250 apart, exactly the default).
3. **Push apart**: two overlapping bodies push each other along the line between them, with speed = overlap x `SeparationStrength` (8), capped at `MaxSeparationSpeed` (400). When they coincide exactly, the `cos/sin` of the index gives a stable pseudo-random direction.

### `UEnemySpawnSubsystem`

[EnemySpawnSubsystem.cpp](../../Game_01%205.8/Source/Game_01/Private/Enemies/EnemySpawnSubsystem.cpp). Tables: `EnemyTuningTable` rows `Grunt` and `Boss` (the `Kind` field is forced, so a forgotten cell cannot turn the Boss into a Grunt) and `SpawnTuningTable` row `Spawn`.

| Function | Behavior |
|---|---|
| `float GetCurrentSpawnRate() const` | `(BaseRate + RampPerMinute x RunTime/60) x heat pressure`, default 0.8 + 1.5 per minute |
| `Tick` | Does nothing while the level-up panel is shown, while the scheduler is off, with no hero, or when the run is not active (`RunTime` only advances in the active case). Accumulates the rate and spawns one Grunt for every full 1, up to `MaxAlive` (200); at the cap it clamps the accumulator to <= 1, to avoid a burst when a slot opens |
| `int32 SpawnHorde(int32 Count, bool bIgnoreCap)` | Spawns a batch on a ring around the hero (radius 1500 to 1900, `RingMinRadius`/`RingMaxRadius`); returns the number actually spawned, 0 if there is no hero |
| `AEnemy* SpawnBoss()` | Spawns one Boss, ignoring the cap; returns null if there is no hero |
| `SetSchedulerEnabled(bool)` / `ResetRun()` | The probes turn the scheduler off; `ResetRun` clears run time and the accumulator but removes no enemies |

Boss defaults are written in the constructor: health 1500, move speed 180, damage 35, range 220, cooldown 1.2, scale 2.5. Grunt: health 25, move speed 330, damage 10, range 90, cooldown 0.8, scale 0.6.

## 7. In-run progression `URunProgressionSubsystem`

[RunProgressionSubsystem.cpp](../../Game_01%205.8/Source/Game_01/Private/Progression/RunProgressionSubsystem.cpp), the star rules in [ProgressionTypes.h](../../Game_01%205.8/Source/Game_01/Public/Progression/ProgressionTypes.h). Brief v3 (pick one of three, optional cut; v2 was "cut one, keep two"): a level-up shows three options; the player takes one (`ChooseOption`) and may first cut one (`CutFromOffer`, at most once per level-up). Options neither taken nor cut stay in the pool. The taken option adds one **copy** of a weapon or passive; copies are also its level. `CutOffer` and `PickOffer` no longer exist.

| Function | Behavior |
|---|---|
| `HandleEnemyKilled` | Spawns an `AXpPickup` at the kill location, value = `XpPerKill` (5), x10 for a Boss; not spawned while a level-up choice is pending |
| `Tick` | Does nothing while an offer is pending. Iterates pickups in reverse: within `CollectRadius` (80, 3D distance) it collects and calls `GrantXp(value x Greed multiplier)`, within `GetMagnetRadius()` it pulls toward the hero at `MagnetSpeed` (900). `GetMagnetRadius() = XpCurve.BaseMagnetRadius + Magnet passive total` (`BaseMagnetRadius` is a field of `FXpCurveRow`, 600 in the C++ fallback and in the shipped `DT_Progression`; it is no longer a hardcoded member of the subsystem) |
| `GrantXp(float)` / `CheckLevelUp` | Adds XP; when `Xp >= XpToNext` (and no offer is pending) it subtracts `XpToNext` (leftover XP carries over), raises the level, sets `XpToNext = BaseXp x Growth^(Level-1)` with the new level (30, x1.3), then `GenerateOffer` |
| `GenerateOffer` | Pool = every weapon and passive that is available (see `IsWeaponAvailable` / `IsPassiveAvailable` below), not cut for good, and holds fewer than `max(row MaxLevel, CopiesForThreeStar)` copies (9 with the defaults, so the row's `MaxLevel` 5 does not cap anything unless it is raised above 9). The pool is shuffled. If the pool has fewer than 3 options there is nothing to cut: every remaining option is applied at once and no offer is shown (leftover XP is not re-checked until the next `GrantXp`; the NEW markers are not taken on this path). Otherwise `UCampStockSubsystem::TakeNewOptions()` is called: every pool entry whose option id is in that list gets `FOfferOption::bIsNewUnlock = true` and is swapped to the front, so an option a lost film reel unlocked is in the first three offered; the list is emptied by the call. Then the first three are offered and the game soft-pauses |
| `GetUnlockedOptions()` (private) | The camp stock's unlocked ids, read through `World->GetGameInstance()`; returns an empty list when the world has no game instance or no stock (editor and commandlet worlds), so an editor world sees only the options that start unlocked |
| `IsWeaponAvailable` / `IsPassiveAvailable` (private) | `FOptionUnlock::IsAvailable(row.bStartsLocked, row name, unlocked)`: an option is available unless its row has `bStartsLocked` and the camp has not unlocked the id equal to the **row name** (`Chain`, `Haste`, `Magnet`; `OptionId()` gives the same string). `FOptionUnlock::IsAvailable` is a pure static function in [ProgressionTypes.h](../../Game_01%205.8/Source/Game_01/Public/Progression/ProgressionTypes.h); an unlocked id that matches no row changes nothing |
| `GetPoolSize() const` | How many weapons and passives are available right now (locked ones not counted): 13 on a new save (weapons Blade, Bolt, Nova, Gun, Orbit; passives Swiftness, Might, Vitality, Greed, Reach, Armor, Regen, Crit), plus one for each of Chain, Haste, Magnet that has been unlocked, up to 16 |
| `ChooseOption(int32 Index)` | Takes offer entry Index, resumes play and applies the option; the others stay in the pool. No-op when nothing is pending or the index is invalid. Then `CheckLevelUp` runs again |
| `CutFromOffer(int32 Index)` / `CanCutFromOffer()` | A cut is allowed once per level-up, while `PermanentCutsLeft > 0` and the offer still has more than one entry. The cut option leaves the offer (the cards shift), is appended to the cutting-room floor (`GetCuttingRoomFloor`, read by `UBossRejectsSubsystem`), joins `CutWeapons` or `CutPassives` so it leaves the pool for the run, and a `CUT: X now belongs to the Boss` toast shows. The offer stays open until the player takes an option. Without the Cut Knight card there is no boss, so the cut only shrinks the pool |
| `ApplyOption` (private) | Adds one copy (capped as above). A weapon raises its level; a passive's effect is recomputed, and for Vitality the change in the bonus (including the jump of a star-up) is added to the hero's max health and healed. A star-up of something already owned posts `X is now N-STAR!` (a brand-new weapon is not a star-up) |
| `GetOfferDescriptions() const` | Text for the UI (`DescribeOption`), empty when nothing is pending: `NEW weapon: Bolt` for a weapon the run does not own, or `Swiftness  Lv0 -> Lv1   (2 more to 2-STAR)`, or `>> 2-STAR!` when that copy completes a star. An option with `bIsNewUnlock` is prefixed `NEW! `: a passive reads `NEW! Haste  Lv0 -> Lv1   (2 more to 2-STAR)`, and a weapon that is both new to the camp and new to the run reads `NEW! Chain  (unlocked weapon)` |
| `GetLoadoutSummary()` | One line, e.g. `Blade Lv3**   Might Lv1*` (one asterisk per star) |
| `GetAttackSpeedMultiplier()` / `GetMagnetRadius()` | `1 + Haste total` (every weapon interval is divided by it) and `BaseMagnetRadius + Magnet total`; `GetMoveSpeedMultiplier`, `GetDamageMultiplier`, `GetXpGainMultiplier` and `GetMaxHealthBonus` work the same way for the other four passives |
| `ResetProgression()` | Level 1, `XpToNext = BaseXp`, clears XP, starts with Blade at level 1, clears passives, the offer and all cuts, sets `PermanentCutsLeft = Stars.PermanentCutsForPool(GetPoolSize())` = `max(pool - MinPoolSize, 0)` (13 - 4 = 9 on a new save; every unlocked option adds one option and one cut, 12 at most), destroys leftover pickups, and logs `PROGRESSION pool=N permanentCuts=N` |

Star rules, `FStarTuning` (in `Director Tuning` > `StarTuning`, not in a data table; the functions are pure and unit tested in `StarTuningTest.cpp`): two stars at 3 copies, three stars at 9; weapon damage x2.0 / x4.0 and reach x1.25 / x1.6; passive total x1.5 / x2.5; `MinPoolSize` 4 (options that always stay in the pool). Helpers: `StarsForCopies`, `CopiesToNextStar`, `WeaponDamageMultiplier`, `WeaponRangeMultiplier`, `PassiveMultiplier`, `PermanentCutsForPool`.

Locked options: in the C++ fallbacks (`DefaultWeapon`, `DefaultPassive`) Chain, Haste and Magnet have `bStartsLocked` true, and the shipped `DT_Weapons` / `DT_Passives` rows are meant to do the same (not readable here). They enter the pool only after a lost film reel unlocks them; see `UCampStockSubsystem` in section 10 and the reel rows in section 2.

Passive fallbacks (`DefaultPassive`, overridden by `DT_Passives` when it loads): Swiftness +10% move speed per stack, Might +15% damage, Vitality +20 max health with instant healing, Greed +25% XP, Haste +12% attack speed (`AttackSpeedAdd`), Magnet +150 cm XP pickup radius (`MagnetRadiusAdd`). The shipped table values exported at the end of story 009 are Swiftness +0.08, Might +0.15, Vitality +20, Greed +0.25, Haste +0.12 and Magnet +150 per stack; the XP curve row is base 30, growth 1.3, 5 XP per kill, magnet base 600. A passive's total effect = `stacks x per-stack amount x star multiplier`, and the speed, damage, XP and attack-speed multipliers are `1 + total` (Vitality and Magnet use the total itself).

## 8. A run `URunOutcomeSubsystem`

[RunOutcomeSubsystem.cpp](../../Game_01%205.8/Source/Game_01/Private/Extraction/RunOutcomeSubsystem.cpp). `bRunActive` is true from world start.

| Function | Behavior |
|---|---|
| `ReportExtracted(ULootBagComponent*)` | Ends with `Extracted` |
| `ReportPlayerDied(ULootBagComponent*)` | Ends with `Died` |
| `EndRun` (private) | Ignored if already ended (the bag must not be null). `FRunSettlement::Compute` (extracted: `IronGained = max(0, floor(bag Iron x multiplier))` and every reel is banked; died: the whole bag is reported as lost); `AddIron` into the stock (ignores 0); on extraction `FReelUnlocks::OptionsForReels` maps the reels carried out to their options (rows from the bag's config) and `UnlockOption` unlocks each new one (stored in `FRunResult::OptionsUnlocked`); dying unlocks nothing; `SaveNow` (the stock change events have already saved once or twice, this one is the explicit "after every run" save); clears the bag; resets heat; broadcasts `OnRunEnded` |
| `StartNewRun()` | Resets state and `LastResult`, keeps `EarningsMultiplier` |
| `SetEarningsMultiplier(float)` | At least 0; set by `UFilmCardSubsystem` at the start of a run (and back to 1 when the run ends, after settlement has used it) |

## 9. Extraction zone `AExtractionZone`

[ExtractionZone.cpp](../../Game_01%205.8/Source/Game_01/Private/Extraction/ExtractionZone.cpp). The occupant is the first Pawn that has a `ULootBagComponent`; others are ignored while it is inside. Progress = progress + `DeltaSeconds / ExtractionSeconds` (default 5 s), and when full the bar is zeroed and `ReportExtracted` is called. On leaving: if `bResetProgressOnLeave` (default true) is true, progress is zeroed and `OnExtractionInterrupted` is broadcast (only if the bar was above 0), otherwise the bar drains by `ProgressDecayPerSecond` (a fraction of the full bar per second, default 0.5) until it reaches 0. When the run has ended (for example by death) the bar is dropped; the zone keeps ticking only if the occupant is still inside, so a new run fills without re-entering. A Pawn already standing inside when the level starts is registered late on the next frame via `SetTimerForNextTick`, because the initial overlap does not trigger an event. The tuning (`FExtractionTuning`) lives on the actor; there is no data table for it. Progress is exposed only through `GetProgress()` and `OnExtractionProgress`, and the HUD reads the former. The zone only ticks while someone is inside or the bar is draining.

## 10. Camp

### `UCampMetaSubsystem`

[CampMetaSubsystem.cpp](../../Game_01%205.8/Source/Game_01/Private/Camp/CampMetaSubsystem.cpp). Building rows come from `BuildingTable` (rows `Smithy` and `Clinic`, keyed by name).

| Function | Behavior |
|---|---|
| `int32 GetUpgradeCost(EBuildingType)` | `floor(BaseCost x CostGrowth^current level)`, default 10 x 2^level (max level 5) |
| `bool TryUpgrade(EBuildingType, FText& Reason)` | Returns false with a message (`<Building> is already at max level`, or `Not enough Iron (need N)`) when already at max level or Iron is insufficient; on success it spends Iron, upgrades and broadcasts `OnMetaChanged` |
| `float GetWeaponDamageBonus() const` | Smithy level x `DamagePerLevel` (default 5) |
| `float GetMaxHealthBonus() const` | Clinic level x `MaxHealthPerLevel` (default 25) |
| `RestoreLevels(const TMap<...>&)` | Used when loading the save; each level is clamped to 0 through `MaxLevel`, and keys not in `Buildings` are discarded; broadcasts `OnMetaChanged` |
| `GetLevels()`, `ResetCamp()`, `IsMaxLevel`, `GetBuildingRow` | Levels copy for saving; reset to level 0 (and broadcast); max-level test; the row with its defaults |
| `TArray<FName> PendingFilmCards` | Cards picked before a run, kept across levels, not saved to disk |

### `UCampStockSubsystem`

[CampStockSubsystem.cpp](../../Game_01%205.8/Source/Game_01/Private/Extraction/CampStockSubsystem.cpp). `AddIron` (non-positive ignored), `UnlockOption(FName)` (returns true only when the option is new; `None` and repeats return false; a new option is also appended to `NewOptions`), `IsOptionUnlocked` (no caller: the progression subsystem reads the whole list through `GetUnlockedOptions`), `GetUnlockedOptions` (in unlock order), `SpendIron` (a non-positive amount returns true and changes nothing; returns false and changes nothing if the stock is insufficient), `RestoreState(Iron, TArray<FName> Unlocked, TArray<FName> NewOptions = {})` (Iron clamped to >= 0, options de-duplicated and `None` dropped; a NEW marker is kept only if that option is unlocked). Every change broadcasts `OnStockChanged`. `AddRelics`, `RemoveRelicAt` and `GetWarehouseRelics` were removed in story 008; `UCampMetaSubsystem::DismantleRelic` and `RelicDismantleIron` went with them.

The "NEW" marker (story 009): `NewOptions` lists the options unlocked since the player last saw an offer. `TakeNewOptions()` returns the list and empties it (`MoveTemp`); `URunProgressionSubsystem::GenerateOffer` calls it, so the first offer of the next run puts those options first and marks them. `GetNewOptions()` is the read-only view that the save uses. Taking does not broadcast and does not write the save, so after a quit the markers come back once more from the file, which is harmless. The unlocked list now drives play: `GenerateOffer` and `GetPoolSize` read it through `GetUnlockedOptions`, and the reel drop rules read it too.

### `UCampSaveSubsystem` and `UCampSaveGame`

[CampSaveSubsystem.cpp](../../Game_01%205.8/Source/Game_01/Private/Flow/CampSaveSubsystem.cpp), [CampSaveGame.h](../../Game_01%205.8/Source/Game_01/Public/Flow/CampSaveGame.h). One slot, named `CampSave` unless the command line has `-SaveSlot=Name` (`GetSlotName()`, read once, so a probe or a clean-save test never touches the real save).

- `Initialize` loads the slot after the stock and meta subsystems exist and restores them with `bApplyingLoad` set (so restoring does not save), then listens to `OnStockChanged` and `OnMetaChanged`: every change writes the slot. A slot that exists but cannot be loaded logs a warning and the camp starts fresh.
- `SaveNow()` writes `SaveVersion = CurrentSaveVersion` (2), Iron, `UnlockedOptions`, `NewOptions` (the not-yet-seen NEW markers, added in story 009 without a version bump: an older version 2 file simply has none) and `BuildingLevels`; returns false if the write fails. Load passes `Save->NewOptions` as the third argument of `RestoreState`; `DeleteSaveAndReset` calls `RestoreState(0, {})`, so it clears them too. Film cards and relics are not saved.
- `SaveVersion` defaults to 1 on purpose: a save stores only values that differ from the class defaults, so a v1 file has no `SaveVersion` and reads back as 1. A save below version 2 is migrated at load: each `WarehouseRelics` entry becomes the option its reel row names (`FReelUnlocks::OptionsForReels`). `WarehouseRelics` is read from v1 saves only and never written.
- `DeleteSaveAndReset()` deletes the slot, empties the stock and resets the buildings with saving suppressed.

### Camp scene actors

`ACampGameMode` (default pawn `ACampPawn`) spawns two `ACampBuilding`s (Smithy at -350,0,60; Clinic at 350,0,60) and one decorative `ACampWarehouse` at 0,-350,60, but only if the level contains none of that kind. `ACampPawn` shows the camp screen and a left click on a building calls `TryUpgrade`. `ACampBuilding::SetLevel` makes the cube taller (`Z = 1.2 + 0.5 x level`) and tints it. The warehouse has no function since story 008.

## 11. Film cards `UFilmCardSubsystem`

[FilmCardSubsystem.cpp](../../Game_01%205.8/Source/Game_01/Private/Editing/FilmCardSubsystem.cpp). A world subsystem, so it is rebuilt with every level. The card list comes only from `FilmCardTable` (there is no C++ fallback list: with no table, no card can be added).

| Function | Behavior |
|---|---|
| `AddCard(FName)` | False if the slots are full (2), the card is already picked, or it is not in the table |
| `RemoveCard(FName)` | True if it was slotted |
| `ApplyActiveCards()` | `ForceHorde` calls `SpawnHorde(HordeCount, true)`, `ForceBoss` calls `SpawnBoss()`, then the multipliers of all cards are multiplied and set on `URunOutcomeSubsystem` |
| `ResetCards()` | Clears the slots and sets the multiplier back to 1.0; called automatically when a run ends (`OnRunEnded`) |
| `DescribeCard(FName)` | For example `HordeNight: a horde of 40 grunts at the start, Iron earned x1.50`; a boss card says `a boss at the start`; an unknown id returns the id |
| Persistence | `Initialize` restores from `PendingFilmCards` (only cards that exist in the table, at most 2); `AddCard`, `RemoveCard` and `ResetCards` write back |

`DT_FilmCards` defines the two cards `HordeNight` and `CutKnight`; the numbers live in the asset (the `FFilmCardRow` fallbacks are 40 grunts and x1.5). The screenshots and the `DescribeCard` output are authoritative for the values.

## 12. Performance probe `ADirectorGameMode`

[DirectorGameMode.cpp](../../Game_01%205.8/Source/Game_01/Private/Game/DirectorGameMode.cpp). Command-line `-PerfProbe`: after a 3-second delay the hero goes into god mode, weapons are turned off, the scheduler is turned off, `SpawnHorde(150, true)` is called, it samples for 10 seconds, writes one line `PERFPROBE enemies=... avgFPS=... minFPS=... samples=...` to `LogEnemies`, and then exits. Without this argument `Tick` is not enabled.

Two more probes follow the same pattern (timer steps, screenshots, log lines, then exit); both use `OfferProbe.StartDelaySeconds` (3 s) and `StepSeconds` (1.5 s) for their timer:

- `-OfferProbe` (`ADirectorGameMode::StepOfferProbe`): puts the hero in god mode, turns the scheduler off, forces a level-up, plays 6 more (`OfferProbe.LevelUps`), then one more. It cuts the first offered option that is not the Chain (`PickCutIndex`; index 0 if all three are Chain, which cannot happen), so a Chain weapon on offer is always kept. It saves `1-first-offer.png`, `2-loadout-after-cuts.png` and `3-late-offer.png` to `Saved/OfferProbe/` and logs `OFFERPROBE <label> level=.. pool=.. cutsLeft=.. cutSoFar=.. attackSpeed=.. magnetRadius=.. loadout=[..]` lines (pool is `GetPoolSize()`, attackSpeed and magnetRadius show Haste and Magnet). After the last level-up it resolves the offer, spawns a 15-enemy horde (`SpawnHorde(15, true)`), binds `OnWeaponFired`, and, if the run owns the Chain, waits up to eight steps for a Chain attack that hits two or more enemies. In the frame of that attack it saves `4-chain-fire.png` and logs `OFFERPROBE chain fired: hit N enemies in one attack`; the last line is `OFFERPROBE chain: owned=0|1 multiHitShot=0|1`. Whether the Chain shows up depends on the save: it needs the `Chain` option unlocked in the slot in use.
- `-ReelProbe` or `-ReelProbe=die` (`ADirectorGameMode::StepReelProbe`): puts 25 Iron and reels in the bag (hero in god mode, spawning off), then ends the run by extraction, or by death with `=die`. `-ReelCount=N` (clamped to 1 through 3, default 1) chooses how many reel types go in, in the order Fast Cut, Wide Shot, Chain Scene. It saves screenshots to `Saved/ReelProbe/` (`1-reel-in-bag.png`, then `2-result-extracted.png` or `3-result-died.png`) and logs `REELPROBE <label> reelsInBag=... heat=... unlocked=[...] anotherReelCanDrop=... outcome=...` lines. It changes the real save unless you also pass `-SaveSlot=`.

`-CampShot` belongs to `ACampGameMode::TakeCampShot` instead: in the camp map it waits 4 seconds, logs `CAMPSHOT iron=... unlocked=[...]`, saves `Saved/ReelProbe/camp-storage.png` and exits 1.5 seconds later.

## 8. Added in stories 011 and 012 (Day 3 night)

Source files are under `Game_01 5.8/Source/Game_01/`. These entries were written from the source when they were added and have not been re-verified line by line like the sections above.

| Class | File | What it does |
|---|---|---|
| `UGameAudioSubsystem` | `Public/Audio/GameAudioSubsystem.h` | Plays the 11 named sounds (`EGameSound`) and the camp or run music (`EMusicTrack`). Which asset plays is the `Sounds` map in Director Tuning. At most `MaxCrowdSoundsPerSecond` crowd sounds start per second |
| `AWeaponEffect` | `Public/Combat/WeaponEffect.h` | A short-lived beam (Blade, Bolt, Chain) or 16-piece ring (Nova) built from engine shapes and the material `M_FlatColor`; replaces the old DrawDebug lines, which a shipping package does not draw |
| `AGunBullet`, `AOrbitOrb` | `Public/Combat/GunBullet.h` | The Gun's bullet (moves in steps of at most 40 cm, hits the first enemy) and one ball of the Orbit weapon (moved by `UAutoWeaponComponent::TickOrbit`) |
| `UBossRejectsSubsystem` | `Public/Enemies/BossRejectsSubsystem.h` | Story 012. Registered by `UEnemySpawnSubsystem::SpawnBoss`; reads the cut list every tick; turns each cut weapon into a boss attack (ball, ring on the floor with a wind-up, or a ring of balls) and each cut passive into a boss stat. Numbers are `BossRejects` in Director Tuning (code defaults) |
| `UPlayAreaSubsystem` | `Public/Game/PlayAreaSubsystem.h` | Finds the floor at level start (the large thin static meshes; `Floor_0`, -4000 to 4000 on both axes) and clamps the hero (`AHeroCharacter::Tick`), the enemy manager's apply loop and the spawn ring to it. Does nothing if no floor is found. The camp map is not covered |

Other changes: `EWeaponType` gained `Gun` and `Orbit`; `EPassiveType` gained `Reach`, `Armor`, `Regen` and `Crit`; `FWeaponRow` gained `ProjectileSpeed` and `ProjectileCount`; `FPassiveRow` gained `RangeAdd`, `DamageReductionAdd`, `RegenPerSecondAdd`, `CritChanceAdd` and `CritMultiplier`. `AEnemy::AddHealth` exists for the boss; enemies flash white and scale up for 0.1 s when hit.
