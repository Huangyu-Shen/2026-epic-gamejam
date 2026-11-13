# Key Classes and Functions

Only the classes, signatures, edge cases and "why" you must know when reading the code are listed. Paths omit the prefix `Game_01 5.8/Source/Game_01/`, and class names link to their implementation files. For UI classes see [ui.md](ui.md); for the save see [data-and-storage.md](data-and-storage.md).

## 1. Hero `AHeroCharacter`

Implementation: [HeroCharacter.cpp](../../Game_01%205.8/Source/Game_01/Private/Player/HeroCharacter.cpp), header [HeroCharacter.h](../../Game_01%205.8/Source/Game_01/Public/Player/HeroCharacter.h).

| Member | Behavior | Edge cases |
|---|---|---|
| `bool CanAct() const` (private) | True only while a run is in progress, the hero is alive and no level-up choice is pending | Returns false when there is no `URunOutcomeSubsystem` |
| `void StartRun()` (private) | Clears enemies, resets kill count and spawning, clears heat, clears the bag, sets max health = base + Clinic bonus, resets progression, resets dodge, `StartNewRun`, applies film cards | Called once in `BeginPlay`; there is no "another run" key, the next run happens by reloading the run map |
| `void CreateHud()` (private) | Creates `URunHudWidget`, sets it to `SelfHitTestInvisible` (only buttons take clicks), shows the mouse, input mode `GameAndUI` without locking the mouse | Falls back to `GetPlayerController(0)` when the controller is null; does not create the HUD again if one exists |
| `bool TryDodge()` | Direction is the current WASD direction, or the facing direction if no key is pressed; dash 0.18 seconds, speed 1800, cooldown 0.8 seconds, invulnerability 0.25 seconds | Returns false on `!CanAct()`, while dashing, or during cooldown |
| `bool SearchNearestContainer()` | Finds the nearest unsearched `ALootContainer` within `InteractRange` (250) and calls `Interact` | Returns false when the bag is full, and the container stays searchable |
| `float ReceiveEnemyDamage(float)` | Multiplies by the bag buff's `DamageTakenMultiplier`, then hands it to `UHealthComponent::ApplyDamage` | Returns the actual damage, 0 while invulnerable |
| `void HandleDied()` | Bound to `OnDied`, calls `ReportPlayerDied` | — |
| `Tick` | Reads WASD (`IsInputKeyDown`); move speed = `BaseWalkSpeed x bag move-speed multiplier x passive move-speed multiplier`; during a dash it uses a sweeping `AddActorWorldOffset` instead of normal movement | Input is zeroed when `CanAct()` is false |

Keys: Space dodge, E search, 1 to 6 use the matching bag slot, K suicide (debug). 1 to 3 are also the shortcuts for the three-way level-up choice; `UseRelicSlot` returns immediately when `CanAct()` is false (that is, while the level-up panel is shown), so there is no conflict.

## 2. Bag and relics `ULootBagComponent`

Implementation: [LootBagComponent.cpp](../../Game_01%205.8/Source/Game_01/Private/Loot/LootBagComponent.cpp).

| Function | Behavior | Errors or edge cases |
|---|---|---|
| `EBagAddResult CanAdd(int32 Iron, bool bWithRelic)` | Does not modify anything; checks whether it fits | `Iron < 0`, or `Iron == 0` with no relic -> `InvalidItem`; over capacity -> `BagFull` |
| `TryAddLoot(int32 Iron, bool bAddRelic, ERelicType)` | Atomically adds Iron and at most one relic; if there is a relic, `AddRarePickupHeat` | On failure nothing is added, a message `Pickup refused: ...` is shown and `OnPickupRefused` is broadcast |
| `bool UseRelic(int32 Index)` | Removes the relic, adds the buff per `FRelicRow`, `ReduceHeatForRelicUse`, message `Used relic: ...` | Returns false for an invalid index; with no matching data row it only warns, adds no buff, but the relic is still consumed |
| `FLootStatModifier GetActiveModifier() const` | Multiplies the three multipliers of all active buffs | All 1.0 when there are no buffs |
| `ClearBag()` | Clears Iron, relics and buffs, stops Tick | Does not touch heat (heat is reset separately at settlement) |
| `TickComponent` | Removes expired buffs; turns off its own Tick when there are no timed buffs | Off by default, only turned on while there are buffs |

Default relic values (inline defaults in [LootConfig.cpp](../../Game_01%205.8/Source/Game_01/Private/Loot/LootConfig.cpp)): Swiftstep move speed x1.3, Warbrand damage x1.5, Aegis damage taken x0.6, each lasting 20 seconds. Containers: supply crate Iron 3 to 6, relic chance 0.2; vault Iron 10 to 20, relic chance 1.0.

## 3. Heat `ULootHeatSubsystem`

| Function | Behavior |
|---|---|
| `float GetSpawnPressureMultiplier() const` | `Lerp(SpawnPressureAtZeroHeat, SpawnPressureAtMaxHeat, heat/max heat)`, default 1.0 to 3.0 |
| `AddHeat(float)` | Clamped to 0 through `MaxHeat`; if the change is 0, `OnHeatChanged` is not broadcast |
| `AddRarePickupHeat()` / `ReduceHeatForRelicUse()` | Default +15 / -8 |
| `SetTuning` | The bag pushes its own config in at `BeginPlay`; so the heat parameters actually come from `ULootConfig`, not from `Director Tuning` |

## 4. Health `UHealthComponent`

| Function | Behavior |
|---|---|
| `float ApplyDamage(float)` | Returns 0 if already dead, invulnerable, in god mode, or `Amount <= 0`; otherwise subtracts health, broadcasts `OnDied` on death, and if not dead grants `HitInvulnerabilitySeconds` (0.25) of invulnerability |
| `Kill()` | Ignores god mode and invulnerability, zeroes health and broadcasts |
| `SetMaxHealth(float)` | Sets the maximum (at least 1), heals to full, clears invulnerability; the Clinic bonus uses it |
| `IncreaseMaxHealth(float)` | Raises the cap and heals by the same amount; the Vitality passive uses it |
| `GetBaseMaxHealth()` | Read from the `Hero` row of `HealthTuningTable` at `BeginPlay` and remembered, so the Clinic bonus does not stack |

## 5. Auto weapons `UAutoWeaponComponent`

[AutoWeaponComponent.cpp](../../Game_01%205.8/Source/Game_01/Private/Combat/AutoWeaponComponent.cpp). When `bEnabled`, no level-up choice is pending, the hero is alive and a run is in progress, `TickComponent` iterates the three weapons and fires any with level > 0 whose cooldown is ready.

Damage formula: `(Row.Damage + Row.DamagePerLevel x (level - 1) + Smithy bonus) x bag damage multiplier x passive damage multiplier`.

| Weapon | Default values (`DefaultWeapon`) | Hit rule |
|---|---|---|
| Blade | Damage 15, +8 per level, interval 0.4 seconds, range 600 | The nearest single enemy |
| Bolt | Damage 20, +10 per level, interval 0.9 seconds, range 1200, half-width 80 | A line toward the nearest enemy (or the facing direction if none), piercing |
| Nova | Damage 25, +12 per level, interval 1.6 seconds, range 350 | All enemies within the radius |

On firing, `DrawDebugLine/Circle` draws a 0.12-second indicator; the engine compiles these calls out in release builds.

## 6. Enemies

### `UEnemyManagerSubsystem::Tick`

[EnemyManagerSubsystem.cpp](../../Game_01%205.8/Source/Game_01/Private/Enemies/EnemyManagerSubsystem.cpp). Three passes:

1. **Chase and damage**: moves toward the hero in the XY plane and stops at `ContactRange x StopRangeFraction` (default 0.8), so enemies form a ring instead of stacking; when `Dist <= ContactRange` and the cooldown is ready, `ReceiveEnemyDamage` is called. After a run ends (`bCanHurt` is false) they still move, they just no longer hurt.
2. **Gridding**: positions are put into a grid by `SeparationCellSize` (default 250), which cuts pairwise checks from O(n^2) to only the 3x3 neighboring cells.
3. **Push apart**: two overlapping bodies push each other along the line between them, with speed = overlap x `SeparationStrength` (8), capped at `MaxSeparationSpeed` (400). When they coincide exactly, the `cos/sin` of the index gives a stable pseudo-random direction.

The whole Tick returns immediately while the level-up panel is shown.

### `UEnemySpawnSubsystem`

| Function | Behavior |
|---|---|
| `float GetCurrentSpawnRate() const` | `(BaseRate + RampPerMinute x RunTime/60) x heat pressure`, default 0.8 + 1.5 per minute |
| `Tick` | Accumulates the rate and spawns one Grunt for every full 1, up to `MaxAlive` (200); at the cap it clamps the accumulator to <= 1, to avoid a burst when a slot opens |
| `int32 SpawnHorde(int32 Count, bool bIgnoreCap)` | Spawns a batch on a ring around the hero (radius 1500 to 1900); returns the number actually spawned |
| `AEnemy* SpawnBoss()` | Spawns one Boss, ignoring the cap; returns null if there is no hero |

Boss defaults are written in the constructor: health 1500, move speed 180, damage 35, range 220, cooldown 1.2, scale 2.5. Grunt: health 25, move speed 330, damage 10, range 90, cooldown 0.8, scale 0.6.

## 7. In-run progression `URunProgressionSubsystem`

[RunProgressionSubsystem.cpp](../../Game_01%205.8/Source/Game_01/Private/Progression/RunProgressionSubsystem.cpp).

| Function | Behavior |
|---|---|
| `HandleEnemyKilled` | Spawns an `AXpPickup` at the kill location, value = `XpPerKill` (5), x10 for a Boss; not spawned while a level-up choice is pending |
| `Tick` | Iterates pickups in reverse: within `CollectRadius` (80) it collects and calls `GrantXp(value x Greed multiplier)`, within `MagnetRadius` (600) it pulls toward the hero at `MagnetSpeed` (900) |
| `CheckLevelUp` | Levels up when `Xp >= XpToNext`, with `XpToNext = BaseXp x Growth^(Level-1)` (30, x1.3), then `GenerateOffer` |
| `GenerateOffer` | Candidate pool = weapons and passives not at max level; shuffles and takes up to 3; if the pool is empty it does not enter the pending state |
| `PickOffer(int32 Index)` | No-op when nothing is pending or the index is invalid; after applying, checks whether there is enough for another level |
| `GetOfferDescriptions() const` | Text for the UI, such as `NEW weapon: Bolt`, `Swiftness  Lv0 -> Lv1` |
| `ResetProgression()` | Level 1, clears XP, starts with Blade at level 1, clears passives, destroys leftover pickups |

Passive defaults: Swiftness +10% move speed per stack, Might +15% damage, Vitality +20 max health with instant healing, Greed +25% XP, each up to 5 stacks.

## 8. A run `URunOutcomeSubsystem`

| Function | Behavior |
|---|---|
| `ReportExtracted(ULootBagComponent*)` | Ends with `Extracted` |
| `ReportPlayerDied(ULootBagComponent*)` | Ends with `Died` |
| `EndRun` (private) | Ignored if already ended; `FRunSettlement::Compute`; `AddIron` and `AddRelics` into the stock; `SaveNow`; clears the bag; resets heat; broadcasts `OnRunEnded` |
| `StartNewRun()` | Resets state, keeps `EarningsMultiplier` |
| `SetEarningsMultiplier(float)` | At least 0; set by `UFilmCardSubsystem` at the start of a run |

## 9. Extraction zone `AExtractionZone`

[ExtractionZone.cpp](../../Game_01%205.8/Source/Game_01/Private/Extraction/ExtractionZone.cpp). The occupant is a Pawn with a `ULootBagComponent`. Progress = progress + `DeltaSeconds / ExtractionSeconds`, and when full it calls `ReportExtracted`. On leaving: if `bResetProgressOnLeave` (default true) is true, progress is zeroed and `OnExtractionInterrupted` is broadcast, otherwise it decays by `ProgressDecayPerSecond` every second. A Pawn already standing inside when the level starts is registered late on the next frame via `SetTimerForNextTick`, because the initial overlap does not trigger an event. Progress is exposed only through `GetProgress()` and `OnExtractionProgress`, and the HUD reads the former.

## 10. Camp

### `UCampMetaSubsystem`

| Function | Behavior |
|---|---|
| `int32 GetUpgradeCost(EBuildingType)` | `floor(BaseCost x CostGrowth^current level)`, default 10 x 2^level |
| `bool TryUpgrade(EBuildingType, FText& Reason)` | Returns false with a message (`Not enough Iron (need N)`) when already at max level or Iron is insufficient; on success it spends Iron, upgrades and broadcasts `OnMetaChanged` |
| `float GetWeaponDamageBonus() const` | Smithy level x `DamagePerLevel` (default 5) |
| `float GetMaxHealthBonus() const` | Clinic level x `MaxHealthPerLevel` (default 25) |
| `int32 DismantleRelic(int32 Index)` | Removes a stored relic and adds `RelicDismantleIron` (8, hardcoded); returns 0 for an invalid index |
| `RestoreLevels(const TMap<...>&)` | Used when loading the save; each level is clamped to 0 through `MaxLevel`, and keys not in `Buildings` are discarded |
| `TArray<FName> PendingFilmCards` | Cards picked before a run, kept across levels, not saved to disk |

### `UCampStockSubsystem`

`AddIron` (non-positive ignored), `AddRelics`, `SpendIron` (returns false if insufficient and changes nothing), `RemoveRelicAt`, `RestoreState` (Iron clamped to >= 0). Every change broadcasts `OnStockChanged`.

## 11. Film cards `UFilmCardSubsystem`

| Function | Behavior |
|---|---|
| `AddCard(FName)` | False if the slots are full (2), the card is already picked, or it is not in the table |
| `ApplyActiveCards()` | `ForceHorde` calls `SpawnHorde(HordeCount, true)`, `ForceBoss` calls `SpawnBoss()`, then the multipliers of all cards are multiplied and set on `URunOutcomeSubsystem` |
| `ResetCards()` | Clears the slots and sets the multiplier back to 1.0; called automatically when a run ends |
| `DescribeCard(FName)` | For example `HordeNight: a horde of 40 grunts at the start, Iron earned x1.50` |
| Persistence | `Initialize` restores from `PendingFilmCards`; `AddCard`, `RemoveCard` and `ResetCards` write back |

The default values of the two cards are in `DT_FilmCards`: `HordeNight` (40 Grunts, x1.5) and `CutKnight` (Boss, x2.0). The screenshots and the `DescribeCard` output are authoritative for the values.

## 12. Performance probe `ADirectorGameMode`

Command-line `-PerfProbe`: after a 3-second delay the hero goes into god mode, weapons are turned off, the scheduler is turned off, `SpawnHorde(150, true)` is called, it samples for 10 seconds, writes one line `PERFPROBE enemies=... avgFPS=... minFPS=... samples=...` to `LogEnemies`, and then exits. Without this argument `Tick` is not enabled.
