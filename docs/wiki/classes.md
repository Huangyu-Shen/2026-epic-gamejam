# Key Classes and Functions

Only the classes, signatures, edge cases and "why" you must know when reading the code are listed. Paths omit the prefix `Game_01 5.8/Source/Game_01/`, and class names link to their implementation files. For UI classes see [ui.md](ui.md); for the save see [data-and-storage.md](data-and-storage.md).

Two naming facts that explain many signatures below:

- **Iron is now wood.** Containers give two resources, wood (supply crate, a tree stump) and stone (vault, a boulder). The code still calls wood "Iron" in many places (`IronCount`, `AddIron`, `SpendIron`, `FRunResult::BagIron`, `FLootContainerRow::IronMin`, `UCampSaveGame::Iron`, `FBuildingRow::BaseCost` comments). The old names were kept so the save file and the data tables still load. Every `GetWood*` accessor returns the same field as the `*Iron*` one. Stone has its own new fields (`StoneCount`, `Stone`, `BagStone`, `StoneGained`, `StoneLost`).
- **Lost film reels** are still `ERelicType` / `FRelicRow` / `RelicChance` in code (the enum names `Swiftstep`, `Warbrand`, `Aegis` are kept for save compatibility; display names are Fast Cut, Wide Shot, Chain Scene).

## 1. Hero `AHeroCharacter`

Implementation: [HeroCharacter.cpp](../../Game_01%205.8/Source/Game_01/Private/Player/HeroCharacter.cpp), header [HeroCharacter.h](../../Game_01%205.8/Source/Game_01/Public/Player/HeroCharacter.h).

Public tuning fields: `InteractRange` 250 cm, `BaseWalkSpeed` 600 cm/s, `CameraDistance` 1400 cm (spring-arm length, pitch -60, no collision test, absolute rotation so the camera does not turn with the pawn), `HeroModelScale` 1.1, `HeroModelYaw` -90 (the pack's hero faces +Y), `Dodge` (`FDodgeTuning`: duration 0.18, speed 1800, cooldown 0.8, invulnerability 0.25), and the components `Bag` (`ULootBagComponent`), `Health` (`UHealthComponent`), `Weapon` (`UAutoWeaponComponent`). `IsDead()` forwards to `Health`.

Model: the constructor looks for the skeletal mesh `SK_RPGHeroPolyart` and the two animations (`Anim_Run_IP_RPGHero`, `Anim_Walk_IP_RPGHero`) of the RPG Hero Squad pack. If all three load, the engine cylinder (blue greybox body) is removed, the model is placed at the capsule's feet, no collision, no shadow, and a static sword and shield are attached to the `Sword` and `Shield` sockets (a missing sword or shield mesh just leaves that one out). Without the pack the blue cylinder stays. `BeginPlay` puts the model in single-node animation mode on the walk animation at play rate 0 (a frozen pose). `Tick` switches to the run animation when speed is above 20 cm/s or a dash is running, with play rate `clamp(speed / BaseWalkSpeed, 0.6, 1.6)`.

| Member | Behavior | Edge cases |
|---|---|---|
| `bool CanAct() const` (private) | True only while a run is in progress, the hero is alive and no level-up choice is pending | Returns false when there is no `URunOutcomeSubsystem` |
| `void StartRun()` (private) | Clears enemies, resets kill count and spawning, clears heat, clears the bag, sets max health = base + Clinic bonus (this also refills health), resets progression, resets dodge, `StartNewRun`, applies film cards | Called once in `BeginPlay`; there is no "another run" key, the next run happens by reloading the run map |
| `void CreateHud()` (private) | Creates `URunHudWidget` (set to `SelfHitTestInvisible`, so only buttons take clicks) and the `UPauseMenuWidget` (context `Run`, z-order 50, same visibility); shows the mouse; input mode `GameAndUI` without locking the mouse | Falls back to `GetPlayerController(0)` when the controller is null; returns early (and sets no input mode) if there is no controller or the HUD already exists |
| `bool TryDodge()` | Direction is the current WASD direction, or the facing direction if no key is pressed; the actor is turned toward it. Dash `Dodge.Duration`, speed `Dodge.Speed`, cooldown counted from the dodge start, `GrantInvulnerability` (the longer window wins) | Returns false on `!CanAct()`, while dashing, or during cooldown |
| `bool SearchNearestContainer()` | Finds the nearest unsearched `ALootContainer` within `InteractRange` (distance on the XY plane, inclusive) and calls `Interact` | Returns false when no container is in range, or when `Interact` refuses (bag too full for the rolled loot: the container stays searchable) |
| `float ReceiveEnemyDamage(float)` | Multiplies the damage by `1 - GetDamageReduction()` (the Armor passive, at most 75%) and then calls `UHealthComponent::ApplyDamage` | Returns the actual damage, 0 while invulnerable or in god mode. Also used by the boss's balls and slams. The comment in the header still says "the bag's damage-taken multiplier": the bag has no such thing, Armor is the source |
| `void HandleDied()` | Bound to `OnDied`, calls `ReportPlayerDied(Bag)` | - |
| `QuitKey()` (private) | Toggles the pause menu | Does nothing while a level-up offer is pending (the offer has its own buttons and keys) |
| `Tick` | 1) Clamps the position to the floor with `UPlayAreaSubsystem::ClampToArea` (margin = capsule radius, teleport without sweep). 2) Regen: while a run is active and no offer is pending, `Health->Heal(GetRegenPerSecond() x dt)`. 3) Reads WASD (`IsInputKeyDown`) only when `CanAct()`; during a dash it uses a sweeping `AddActorWorldOffset` (walls stop it) instead of normal movement. 4) Animation switch. 5) Sets `MaxWalkSpeed = BaseWalkSpeed x GetMoveSpeedMultiplier()` (so a speed change shows from the next frame) | A dash already under way finishes even when `CanAct()` turned false. Input is world axes: W is +X, D is +Y |

Keys: Space dodge, E search (only when `CanAct()`), K suicide (debug, `Health->Kill()`, ignores god mode and `CanAct`), Esc (pause menu `UPauseMenuWidget`; bound with `bExecuteWhenPaused` so it can close the menu that paused the game), 1 to 3 take one of the three level-up options (`ChooseOption`) and 4 to 6 cut one of them (`CutFromOffer`); both do nothing unless an offer is pending (and a cut only when `CanCutFromOffer()`). Lost film reels cannot be used, so there is no bag-slot key.

Oddity: `HeroModelScale` and `HeroModelYaw` are read in the constructor only. Editing them in the editor changes the property but not the already-built mesh component, so only changing the C++ default has an effect. `CameraDistance` and `BaseWalkSpeed` do work (applied in `BeginPlay` and every tick).

## 2. Bag and lost film reels `ULootBagComponent`

Implementation: [LootBagComponent.cpp](../../Game_01%205.8/Source/Game_01/Private/Loot/LootBagComponent.cpp), types in [LootTypes.h](../../Game_01%205.8/Source/Game_01/Public/Loot/LootTypes.h) (`ELootResource { Wood, Stone }`, `EBagAddResult { Success, BagFull, InvalidItem }`).

Capacity: wood takes `ceil(wood / IronPerSlot)` slots, stone takes `ceil(stone / IronPerSlot)` slots (rounded **separately**, so 1 wood plus 1 stone costs 2 slots), and each reel takes one slot; defaults are 6 slots and 10 per slot (`FLootTuning`; stone has no tuning of its own).

| Function | Behavior | Errors or edge cases |
|---|---|---|
| `EBagAddResult CanAdd(int32 Amount, bool bWithRelic, ELootResource Resource = Wood)` | Does not modify anything; checks whether the new slot total fits | `Amount < 0`, or `Amount == 0` with no reel -> `InvalidItem`; over capacity -> `BagFull` |
| `TryAddLoot(int32 Amount, bool bAddRelic, ERelicType, ELootResource Resource = Wood)` | Atomically adds the amount to the wood or stone counter and at most one lost film reel; if there is a reel, `AddRarePickupHeat`; broadcasts `OnBagChanged` | On failure nothing is added, a message `Pickup refused: ...` is shown and `OnPickupRefused` is broadcast. The `Resource` is irrelevant for a reel-only add |
| `TryAddIron(Amount)` / `TryAddRelic(ERelicType)` | Inline shortcuts: `TryAddLoot(Amount, false, ...)` (adds **wood**) and `TryAddLoot(0, true, type)` | Same errors |
| `GetWoodCount()`, `GetIronCount()`, `GetStoneCount()` | `GetIronCount` is the same field as `GetWoodCount` (only the old stand-in pawn still calls it) | - |
| `GetRelics()` / `GetRelicCount()` | The reels in the bag (a copy), and how many | A reel is read-only cargo: nothing in a run can use or consume it |
| `GetUsedSlots()`, `GetCapacitySlots()`, `IsFull()` | Slot arithmetic above; `IsFull` is `used >= capacity` | - |
| `static GetAddResultText(EBagAddResult)` | "Added", "Bag is full", "Nothing to add" | - |
| `ClearBag()` | Clears wood, stone and the reels, broadcasts `OnBagChanged` | Does not touch heat (heat is reset separately at settlement) |
| `GetConfig()` | `Config` if assigned, otherwise a lazily created `ULootConfig::CreateFromSettings(this)` kept in a transient field | Never null |
| `BeginPlay` | Pushes the tuning into `ULootHeatSubsystem` and logs `LOOTCFG source=table or c++ defaults, capacity=..., reelRows=...` | `source=c++ defaults` means the `LootTuningTable` is unset or its row `Default` was not found |

The bag does not tick. Story 008 removed `UseRelic`, `GetActiveModifier`, `GetActiveBuffs`, the `OnRelicUsed` and `OnBuffsChanged` delegates, `FLootStatModifier` and `FActiveRelicBuff`; none of these names occurs in `Source/` any more.

Default reel rows (inline defaults in [LootConfig.cpp](../../Game_01%205.8/Source/Game_01/Private/Loot/LootConfig.cpp); `DT_Reels` is the table that overrides them): Fast Cut (`ERelicType::Swiftstep`) unlocks the option `Haste`, Wide Shot (`Warbrand`) unlocks `Magnet` and Chain Scene (`Aegis`) unlocks `Chain`. The option id is the row name of the weapon or passive it unlocks (see section 7). Every row has `DropWeight` 1. `FRelicRow::IsDroppable` is the drop rule: the row names an option, the camp has not unlocked that option, and the bag does not already hold that reel type. A row whose `UnlocksOption` is `None` never drops. `ULootConfig::PickRandomRelic(unlocked, bag, out)` picks among the droppable rows by `DropWeight` (negative weights count as 0; if every weight is 0 the pick is uniform) and returns false when nothing can drop. Containers: supply crate 3 to 6, reel chance 0.2; vault 10 to 20, reel chance 1.0 (the fields are still called `IronMin`, `IronMax`, `RelicChance`). Consequence with the shipped data: a reel type stops dropping once its option is unlocked or its reel is already in the bag, and once all three options are unlocked no container drops a reel, not even a vault.

Where the numbers come from: `ULootConfig::CreateFromSettings` reads three tables from `Director Tuning`: `LootTuningTable` (`DT_LootTuning`, the row named `Default`, an `FLootTuning`), `LootContainerTable` (`DT_LootContainers`) and `ReelTable` (`DT_Reels`). A table that is not set, or (reel and container tables) has no rows, keeps the C++ defaults above. Container and reel rows are matched by their `ContainerType` / `RelicType` field, not by row name; for reels the first row of a type wins. A container table that has rows but none for a container's type does not fall back per type: `FindContainerRow` fails and that container cannot be searched. A `ULootConfig` data asset assigned to the bag's `Config` takes precedence, but no such asset exists in `Content/Data` (checked: only the `DT_*` tables are there).

### `ALootContainer`

[LootContainer.cpp](../../Game_01%205.8/Source/Game_01/Private/Loot/LootContainer.cpp). `ContainerType` (`SupplyCrate` or `Vault`) decides both the data row and the resource: a vault gives **stone**, a crate gives **wood**. `Interact(AActor*)` returns true only when the search completed. It returns false for a null or already-searched container (the latter posts "Already searched"), an instigator with no bag (a controller is also accepted and its pawn's bag used), a missing container row, or a bag that refuses the loot.

- Look: the root is the engine cube at scale 0.6. In `BeginPlay`, if the Tiny Fantasy Forest pack loaded, the cube is replaced by `SM_TreeStump` (crate, scale 1.8) or `SM_RockA` (vault, scale 0.9), the material is cleared so the pack's own material shows, and the actor is moved down by the cube's half height because the pack models have their origin at their base. Without the pack the cube stays.
- The roll happens once and is remembered (`bRolled`), so retrying after `BagFull` cannot re-roll: the amount is `RandRange(IronMin, max(IronMin, IronMax))`, then a reel is rolled with `FRand() < RelicChance` and `PickRandomRelic(camp's unlocked options, bag's reels)`.
- If the remembered reel type has meanwhile entered the bag from another container, the reel is dropped from the roll and only the resource is added; if that leaves 0 the container opens empty.
- A roll with no reel and 0 amount opens an empty container without touching the bag.
- On success it posts `Searched: +N wood` or `+N stone` (plus ` + lost reel X`) and broadcasts `OnSearched(Container, Amount, bGotRelic, RelicType)`. `ConfigOverride` (a `ULootConfig`) beats the bag's config.
- `ShowOpened` plays the `Pickup` sound and squashes the mesh to a quarter of its height as the "opened" cue; the cube is also lowered so it stays on the floor, the pack model is not (its origin is at its base).

## 3. Heat `ULootHeatSubsystem`

[LootHeatSubsystem.cpp](../../Game_01%205.8/Source/Game_01/Private/Loot/LootHeatSubsystem.cpp).

| Function | Behavior |
|---|---|
| `float GetSpawnPressureMultiplier() const` | `Lerp(SpawnPressureAtZeroHeat, SpawnPressureAtMaxHeat, heat/max heat)`, default 1.0 to 3.0 |
| `AddHeat(float)` | Clamped to 0 through `MaxHeat`; if the change is 0, `OnHeatChanged(NewHeat, Delta)` is not broadcast |
| `AddRarePickupHeat()` | Adds `HeatPerRarePickup` (default +15, `MaxHeat` default 100) when a lost film reel is picked up. Nothing lowers heat during a run; `ResetHeat()` (which is `AddHeat(-Heat)`, so it broadcasts when heat was above 0) runs at the start of a run and at the end of one |
| `SetTuning` | The bag pushes its own config in at `BeginPlay` (and the current heat is re-clamped to the new `MaxHeat`); the config is built from the `LootTuningTable` in `Director Tuning` (row `Default`), or from the C++ defaults when that row is missing |
| `GetHeat()`, `GetHeatNormalized()` | Heat and heat/`MaxHeat` clamped to 0..1 (0 if `MaxHeat` is 0) |

## 4. Health `UHealthComponent`

[HealthComponent.cpp](../../Game_01%205.8/Source/Game_01/Private/Combat/HealthComponent.cpp).

| Function | Behavior |
|---|---|
| `float ApplyDamage(float)` | Returns 0 if already dead, invulnerable, in god mode (`bGodMode`), or `Amount <= 0`; otherwise subtracts `min(Amount, Health)` and broadcasts `OnDamaged(Applied, NewHealth)`, broadcasts `OnDied` on death, and if not dead grants `HitInvulnerabilitySeconds` (default 0.25) of invulnerability |
| `Kill()` | Ignores god mode and invulnerability, zeroes health, broadcasts `OnDamaged` and `OnDied`; no-op if already dead |
| `Heal(float)` (not a UFUNCTION) | Adds up to `Tuning.MaxHealth`; ignored for `Amount <= 0` or when dead. Broadcasts nothing (the HUD polls). The Regen passive calls it every tick from `AHeroCharacter::Tick` |
| `ResetHealth()` | Full health, clears invulnerability |
| `SetMaxHealth(float)` | Sets the maximum (at least 1), heals to full, clears invulnerability; the Clinic bonus uses it |
| `IncreaseMaxHealth(float)` | Raises the cap and heals by the same amount (ignored for `Amount <= 0`); the Vitality passive uses it |
| `GrantInvulnerability(float)` | Invulnerable until `now + Seconds`, unless a longer window is already running; used by the dodge |
| `GetBaseMaxHealth()` | Read from the `Hero` row of `HealthTuningTable` (an `FHealthTuning`: max health and hit invulnerability) at `BeginPlay` and remembered, so the Clinic bonus does not stack; `BeginPlay` also calls `ResetHealth()` |

## 5. Auto weapons `UAutoWeaponComponent`

[AutoWeaponComponent.cpp](../../Game_01%205.8/Source/Game_01/Private/Combat/AutoWeaponComponent.cpp). When `bEnabled`, no level-up choice is pending, the hero is alive and a run is in progress, `TickComponent` iterates the six weapons (`EWeaponType::Count`: Blade, Bolt, Nova, Chain, Gun, Orbit) and handles every one with level > 0. The Chain has level 0 until the player has unlocked it and picked it. A weapon's "level" is the number of copies the run holds (see section 7). Cooldowns only count down while the component is allowed to fire, so a soft-pause also pauses them. For the five interval weapons (not the Orbit) the cooldown restarts after every fire attempt, including a Blade that found no target, and is `Row.Interval / max(attack speed multiplier, 0.1)`: the Haste passive divides every weapon's interval.

Per weapon, before firing, the row is copied and `Range`, `LineHalfWidth` and `ChainRange` are multiplied by the reach: `star range multiplier (1.25 at two stars, 1.6 at three) x Reach passive multiplier`. The same reach number also thickens the visual (`CurrentReach`), so a star-up is visible on screen.

Damage formula: `RollCrit((Row.Damage + Row.DamagePerLevel x (level - 1) + Smithy bonus) x Might multiplier x star damage multiplier)`. `RollCrit`: with probability `GetCritChance()` (0 to 0.9) the result is multiplied by `GetCritMultiplier()` (the Crit row's `CritMultiplier`, default 2). The Gun rolls once per shot, so all bullets of a shot crit together; the Orbit rolls once per enemy hit.

`FireWeapon` returns how many enemies the shot hit, and `TickComponent` then broadcasts `OnWeaponFired(EWeaponType, int32 hits)` (`FOnWeaponFired`, a native multicast delegate declared in [AutoWeaponComponent.h](../../Game_01%205.8/Source/Game_01/Public/Combat/AutoWeaponComponent.h)) after every fire attempt, including one that hit nothing, and plays the `Attack` sound (a crowd sound, rate-capped) only when hits > 0. The Orbit broadcasts from its own tick, only when at least one enemy was hurt that tick. Today only the `-OfferProbe` binds the delegate.

| Weapon | C++ fallback values (`DefaultWeapon` in [RunProgressionSubsystem.cpp](../../Game_01%205.8/Source/Game_01/Private/Progression/RunProgressionSubsystem.cpp)) | Hit rule |
|---|---|---|
| Blade | Damage 15, +8 per level, interval 0.4 s, range 600 | The nearest single enemy within range (XY distance). Draws a yellow beam hero to target (14 cm thick x reach, 0.12 s) |
| Bolt | Damage 20, +10, interval 0.9 s, range 1200, half-width 80 | A line toward the nearest enemy within range (or the facing direction if none), piercing, length = range; every enemy whose projection along the line is 0..range and whose sideways offset is within the half-width is hit. Cyan beam 20 cm x reach, drawn even when nothing was hit |
| Nova | Damage 25, +12, interval 1.6 s, range 350 | All enemies within the radius (= range), XY distance. Magenta ring of 16 pieces, 14 cm x reach, 0.15 s |
| Chain | Damage 12, +6, interval 1.1 s, range 700, `ChainJumps` 3, `ChainRange` 350, `bStartsLocked` true | The path is picked first: the nearest enemy within `Range`, then up to `ChainJumps` times the nearest not-yet-hit enemy within `ChainRange` of the last one chosen (XY distance). Only then is damage dealt to every enemy on the path, because a kill removes an enemy from the manager's list. One green beam per hop (10 cm x reach, 0.3 s) |
| Gun | Damage 14, +7, interval 0.7 s, range 1500, `ProjectileSpeed` 2600, `ProjectileCount` 1 | Needs an enemy within range. Fires `min(ProjectileCount + (level - 1) / 2, 5)` bullets toward the nearest enemy, fanned 9 degrees apart (see `AGunBullet`). Returns hits = 1 for any shot, even if the bullets later miss |
| Orbit | Damage 10, +5, interval 0.5 s, range 240 (the orbit radius), `ProjectileSpeed` 160 (degrees per second), `ProjectileCount` 2 | Not on a cooldown: every tick the balls (`min(ProjectileCount + (level - 1) / 2, 6)` of them, rebuilt when the count changes) move on a circle of radius `Range` around the hero, and any enemy whose XY distance to a ball is within `65 cm x reach + enemy body radius` is hurt, then cannot be hurt again for `Row.Interval` seconds (the per-enemy timer map is pruned when it passes 300 entries) |

The values the game actually uses come from `DT_Weapons` (rows `Blade`, `Bolt`, `Nova`, `Chain`, keyed by row name, not by the row's own type field) whenever that table loads. A byte search of `DT_Weapons.uasset` finds only the four names Blade, Bolt, Nova, Chain, so **Gun and Orbit have no row and always use the C++ fallbacks above** (an indirect check; the assets are binary). The values recorded when story 009 was finished for the four table rows were: Blade damage 20, +10 per level, interval 0.35, range 220; Bolt 12, +7, 0.55, range 900, half-width 40; Nova 14, +8, 0.8, range 320; Chain 12, +6, 1.1, range 700, 3 jumps, chain range 350 (not re-verified, read them in the editor).

### `AWeaponEffect`

[WeaponEffect.cpp](../../Game_01%205.8/Source/Game_01/Private/Combat/WeaponEffect.cpp). The visual of one attack, built from engine shapes with `M_FlatColor`, which replaced the earlier `DrawDebugLine/Circle` indicators (no `DrawDebug*` call remains outside the stand-in pawn; DrawDebug is compiled out of Shipping builds, this is not).

| Function | Behavior | Edge cases |
|---|---|---|
| `static SpawnBeam(World, From, To, Color, Thickness, Lifetime)` | One engine cylinder (Z axis turned along the beam) centred between the points | Nothing is spawned for a null world or a beam shorter than 1 cm |
| `static SpawnRing(World, Center, Radius, Color, Thickness, Lifetime)` | A ring of 16 flat cubes tangent to the circle (piece length `2 R tan(pi/16)` x 1.05) | Nothing for a null world or `Radius < 1` |
| `Tick` | After `Life` seconds the actor destroys itself; before that each piece shrinks in its thin dimensions by `1 - age/life` while its long dimension keeps its length, so the attack thins out instead of blinking | The boss uses the same class for its slam warning and impact |

### `AGunBullet` and `AOrbitOrb`

[GunBullet.cpp](../../Game_01%205.8/Source/Game_01/Private/Combat/GunBullet.cpp). `AGunBullet::Fire(World, Origin, Direction, Speed, Range, Damage, HitRadius, Color)` spawns a small sphere (scale 0.3, no collision) that flies in a straight line. Its `Tick` moves it in steps of at most 40 cm (`MaxStepCm`) so a slow frame cannot jump over an enemy; after each step `FindNearestEnemy(location, HitRadius)` (XY distance; the Gun passes `70 cm x reach`) decides a hit: the enemy takes the damage and the bullet is destroyed, or it is destroyed when its range is used up. While a level-up offer is pending the bullet waits. It does not check whether the run has ended. `AOrbitOrb::Make(World, Location, Color, Scale)` returns a sphere with no logic of its own: `UAutoWeaponComponent::TickOrbit` moves it, and the boss's balls (`UBossRejectsSubsystem`) reuse it. `EndPlay` of the weapon component destroys the orbs. If the hero dies the component stops ticking and the orbs stay frozen in the air until the level closes.

## 6. Enemies

### `AEnemy`

[Enemy.cpp](../../Game_01%205.8/Source/Game_01/Private/Enemies/Enemy.cpp). An actor with no collision and no tick; `UEnemyManagerSubsystem` moves it. The root is an engine cube (`Body`) kept as the invisible root, and a `Visual` skeletal mesh component is its child.

| Member | Behavior | Edge cases |
|---|---|---|
| `Init(FEnemyTuning)` | Sets health and the cube's scale and colours it (Grunt red, Boss purple) through a dynamic material | Call right after spawning |
| `SetVisual(USkeletalMesh*, UAnimSequence*, YawOffset, ModelScale)` | Removes the cube mesh, shows the skeletal mesh at `-50 cm` (relative to the cube, so the model's feet are at the cube's bottom), applies the yaw and scale, loops the move animation from a random time so a crowd does not step in unison | A null mesh keeps the cube. The model scale is **relative to the cube's scale**, so the world size is `Tuning.Scale x ModelScale`. The component only ticks its pose when rendered and uses update-rate optimisation (150 animated bodies) |
| `ApplyDamage(float)` | Does nothing once health is 0. At 0 or below: `NotifyEnemyKilled`, `EnemyDeath` sound, `Destroy()`. Otherwise: `EnemyHit` sound and a hit flash: the cube turns white and the body scales to 1.2x for 0.1 s | The flash colour only affects the cube, which is gone when a model is shown, so a model enemy shows only the scale punch. The flash ends in `UpdateHitFeedback`, which only the manager loop calls (so it is frozen while a level-up offer pauses the manager) |
| `AddHealth(float)` | Adds health above the starting value; the boss gains health from the player's cuts | - |
| `GetBodyRadius()` | `Tuning.Scale x 100 x 0.5` (cube based, even with a model) | Used by the push-apart pass, the Orbit and the play-area clamp |
| `EndPlay` | Unregisters from the manager only on a real destroy, not on world teardown | - |

### `UEnemyManagerSubsystem`

[EnemyManagerSubsystem.cpp](../../Game_01%205.8/Source/Game_01/Private/Enemies/EnemyManagerSubsystem.cpp). Public helpers: `FindNearestEnemy(Location, Range)` (XY distance, inclusive), `GetEnemies()`, `GetAliveCount()`, `GetKillCount()`, `ResetKillCount()`, `ClearAll()` (destroys everything without counting kills) and the `OnEnemyKilled(Kind, Location)` delegate that `URunProgressionSubsystem` listens to. `FSwarmTuning` comes from the `Swarm` row of `SwarmTuningTable`.

`Tick` returns at once while the level-up panel is shown, when there is no hero pawn, or when there are no enemies. Otherwise three passes:

1. **Chase and damage**: moves toward the hero in the XY plane and stops at `ContactRange x StopRangeFraction` (default 0.8), so enemies form a ring instead of stacking; the yaw turns to the movement direction; when `Dist <= ContactRange` and the cooldown is ready, `ReceiveEnemyDamage` is called. The enemy's contact cooldown is spent even when the hero was invulnerable or in god mode and took nothing. After a run ends (`bCanHurt` is false) they still move, they just no longer hurt.
2. **Gridding**: positions are put into a grid by `SeparationCellSize` (default 250), which cuts pairwise checks from O(n^2) to only the 3x3 neighboring cells. The cell size must be at least the largest pair of body radii (two Bosses are 250 apart, exactly the default).
3. **Push apart** and apply: two overlapping bodies push each other along the line between them, with speed = overlap x `SeparationStrength` (8), capped at `MaxSeparationSpeed` (400). When they coincide exactly, the `cos/sin` of the index gives a stable pseudo-random direction. The new position is clamped to the play area (`UPlayAreaSubsystem`, margin = body radius), set together with the yaw, and `UpdateHitFeedback` is called.

### `UEnemySpawnSubsystem`

[EnemySpawnSubsystem.cpp](../../Game_01%205.8/Source/Game_01/Private/Enemies/EnemySpawnSubsystem.cpp). Tables: `EnemyTuningTable` rows `Grunt` and `Boss` (the `Kind` field is forced, so a forgotten cell cannot turn the Boss into a Grunt) and `SpawnTuningTable` row `Spawn`.

| Function | Behavior |
|---|---|
| `float GetCurrentSpawnRate() const` | `(BaseRate + RampPerMinute x RunTime/60) x heat pressure`, default 0.8 + 1.5 per minute |
| `Tick` | Does nothing while the level-up panel is shown, while the scheduler is off, with no hero, or when the run is not active (`RunTime` only advances in the active case). Accumulates the rate and spawns one Grunt for every full 1, up to `MaxAlive` (200); at the cap it clamps the accumulator to <= 1, to avoid a burst when a slot opens |
| `int32 SpawnHorde(int32 Count, bool bIgnoreCap)` | Spawns a batch on a ring around the hero (radius 1500 to 1900, `RingMinRadius`/`RingMaxRadius`), clamped to the play area; returns the number actually spawned, 0 if there is no hero |
| `AEnemy* SpawnBoss()` | Spawns one Boss, ignoring the cap, and registers it with `UBossRejectsSubsystem::SetBoss`; returns null if there is no hero |
| `SetSchedulerEnabled(bool)` / `ResetRun()` | The probes turn the scheduler off; `ResetRun` clears run time and the accumulator but removes no enemies |

Boss defaults are written in the constructor: health 1500, move speed 180, damage 35, range 220, cooldown 1.2, scale 2.5. Grunt: health 25, move speed 330, damage 10, range 90, cooldown 0.8, scale 0.6.

**Models** (`FEnemyVisualAsset`: `Mesh`, `Move` animation, `YawOffset`, `Scale`): the constructor looks up the Monster for Survival Game pack, `/Game/MonsterForSurvivalGame/Mesh/Polyart/<Name>_SK` with a run animation per monster. Grunts pick one at random from the ones that were found: Slime, Mushroom (smile version), Cactus, TurtleShell, Beholder, each at model scale 1.2, yaw offset -90 (the pack faces +Y). The Boss is the ChestMonster at scale 0.85, yaw offset -90. A monster whose mesh is found but whose animation is not shows a still pose. With no pack, every list is empty and the cubes stay. `SpawnOnRing` rests the cube on the floor under the hero (hero Z minus capsule half height plus half the cube height), spawns with `AlwaysSpawn`, calls `Init`, then `SetVisual`, then registers with the manager.

### `UBossRejectsSubsystem` (story 012, the Cut Knight boss)

[BossRejectsSubsystem.cpp](../../Game_01%205.8/Source/Game_01/Private/Enemies/BossRejectsSubsystem.cpp), types in [BossRejectsSubsystem.h](../../Game_01%205.8/Source/Game_01/Public/Enemies/BossRejectsSubsystem.h). A tickable world subsystem that does nothing until `SpawnBoss` calls `SetBoss(boss)` (in practice the Cut Knight film card). It turns the run's cutting-room floor (`URunProgressionSubsystem::GetCuttingRoomFloor`) into boss power. The numbers are `FBossRejectTuning` in `Director Tuning` > `BossRejects` (`DefaultGame.ini` has no entry for it, so the constructor defaults apply).

| Function | Behavior | Edge cases |
|---|---|---|
| `SetBoss(AEnemy*)` | Clears old attacks and counters, remembers the boss's base move speed and contact damage, and sets `Processed = 0` | Cuts made before the boss appeared are applied at once when it appears, because the floor is read from index 0 again |
| `Tick` | If the boss is gone it clears every ball and ring, forgets the boss and ignores later cuts. Needs a run in progress and a hero. First applies every new floor entry (`ApplyCut`), even while an offer is open (so the choice bites the moment it is made); then returns while an offer is pending or the hero is dead. Then fires each owned attack when its timer is due, moves the balls and resolves the slams | A ball hits when its XY distance to the hero is under `ShotHitRadius` (90); a slam hits when the hero is within the radius when the wind-up ends. All damage goes through `ReceiveEnemyDamage`, so Armor and invulnerability apply |
| `ApplyCut` | A cut **weapon** adds one copy of that attack (first copy: "gets a new attack", more: damage `x (1 + RepeatDamageStep x (copies - 1))`, 25% of base). A cut **Swiftness** adds 8% move speed, **Might** 15% contact damage (each capped at +100% = `MaxStatBonus`), **Haste** multiplies every attack interval by 0.9 per cut (never below 0.5 in total). **Vitality** gives +150 health, every other passive (Greed, Magnet, Reach, Armor, Regen, Crit) +100 health, all health capped at 1500 in total. A HUD toast `THE BOSS TAKES X - it ...` is posted | A weapon with no entry in `Attacks` falls to the generic health branch |

Default attacks (`FBossAttackTuning`, all can be seen coming and dodged): Blade, Shot every 3.0 s, damage 12, ball speed 800; Bolt, Shot 4.0 s, 18, 1500; Chain, Shot 1.8 s, 7, 700; Gun, Shot 1.3 s, 9, 1000; Nova, Slam every 5.0 s, 25 damage, radius 450, 1.0 s wind-up (a red ring is drawn on the floor under the hero, a white ring flashes on impact); Orbit, Ring every 6.0 s, 8 damage, 8 balls at 450 speed. A ball flies straight (default range 2400 cm) toward where the hero was when it was fired, drawn as an `AOrbitOrb` of scale 0.45.

## 7. In-run progression `URunProgressionSubsystem`

[RunProgressionSubsystem.cpp](../../Game_01%205.8/Source/Game_01/Private/Progression/RunProgressionSubsystem.cpp), the star rules in [ProgressionTypes.h](../../Game_01%205.8/Source/Game_01/Public/Progression/ProgressionTypes.h). Brief v3 (pick one of three, optional cut; v2 was "cut one, keep two"): a level-up shows three options; the player takes one (`ChooseOption`) and may first cut one (`CutFromOffer`, at most once per level-up). Options neither taken nor cut stay in the pool. The taken option adds one **copy** of a weapon or passive; copies are also its level. `CutOffer` and `PickOffer` no longer exist (the header's class comment still describes the v2 rule "cut one, keep two").

There are six weapons (`EWeaponType`: Blade, Bolt, Nova, Chain, Gun, Orbit) and ten passives (`EPassiveType`: Swiftness, Might, Vitality, Greed, Haste, Magnet, Reach, Armor, Regen, Crit). `FOfferOption` carries `bIsWeapon`, the weapon or passive, `bIsNewWeapon`, `CurrentLevel` (copies held before the pick) and `bIsNewUnlock`.

| Function | Behavior |
|---|---|
| `HandleEnemyKilled` | Spawns an `AXpPickup` at the kill location, value = `XpPerKill` (5), x10 for a Boss; not spawned while a level-up choice is pending |
| `Tick` | Does nothing while an offer is pending. Iterates pickups in reverse: within `CollectRadius` (80, 3D distance) it collects, calls `GrantXp(value x Greed multiplier)` and plays the `Pickup` sound, within `GetMagnetRadius()` it pulls toward the hero at `MagnetSpeed` (900). `GetMagnetRadius() = XpCurve.BaseMagnetRadius + Magnet passive total` (`BaseMagnetRadius` is a field of `FXpCurveRow`, 600 in the C++ fallback and in the shipped `DT_Progression`) |
| `GrantXp(float)` / `CheckLevelUp` | Adds XP; when `Xp >= XpToNext` (and no offer is pending) it subtracts `XpToNext` (leftover XP carries over), raises the level, sets `XpToNext = BaseXp x Growth^(Level-1)` with the new level (30, x1.3), then `GenerateOffer` |
| `GenerateOffer` | Pool = every weapon and passive that is available (see `IsWeaponAvailable` / `IsPassiveAvailable` below), not cut for good, and holds fewer than `max(row MaxLevel, CopiesForThreeStar)` copies (9 with the defaults, so the row's `MaxLevel` 5 does not cap anything unless it is raised above 9). The pool is shuffled (Fisher-Yates). If the pool has fewer than 3 options there is nothing to cut: every remaining option is applied at once and no offer is shown (no level-up sound; leftover XP is not re-checked until the next `GrantXp`; the NEW markers are not taken on this path). Otherwise `UCampStockSubsystem::TakeNewOptions()` is called: every pool entry whose option id is in that list gets `bIsNewUnlock = true` and is swapped to the front, so an option a lost film reel unlocked is in the first three offered; the list is emptied by the call. Then the first three are offered, the `LevelUp` sound plays and the game soft-pauses |
| `GetUnlockedOptions()` (private) | The camp stock's unlocked ids, read through `World->GetGameInstance()`; returns an empty list when the world has no game instance or no stock (editor and commandlet worlds), so an editor world sees only the options that start unlocked |
| `IsWeaponAvailable` / `IsPassiveAvailable` (private) | `FOptionUnlock::IsAvailable(row.bStartsLocked, row name, unlocked)`: an option is available unless its row has `bStartsLocked` and the camp has not unlocked the id equal to the **row name** (`Chain`, `Haste`, `Magnet`). `FOptionUnlock::IsAvailable` is a pure static function in [ProgressionTypes.h](../../Game_01%205.8/Source/Game_01/Public/Progression/ProgressionTypes.h); an unlocked id that matches no row changes nothing |
| `GetPoolSize() const` | How many weapons and passives are available right now (locked ones not counted): 13 on a new save (weapons Blade, Bolt, Nova, Gun, Orbit; passives Swiftness, Might, Vitality, Greed, Reach, Armor, Regen, Crit), plus one for each of Chain, Haste, Magnet that has been unlocked, up to 16 |
| `ChooseOption(int32 Index)` | Takes offer entry Index, resumes play and applies the option; the others stay in the pool. No-op when nothing is pending or the index is invalid. Then `CheckLevelUp` runs again |
| `CutFromOffer(int32 Index)` / `CanCutFromOffer()` | A cut is allowed once per level-up, while `PermanentCutsLeft > 0` and the offer still has more than one entry. The cut plays the `Cut` sound; the cut option leaves the offer (the cards shift), is appended to the cutting-room floor (`GetCuttingRoomFloor`, read by `UBossRejectsSubsystem`), joins `CutWeapons` or `CutPassives` so it leaves the pool for the run, and a `CUT: X now belongs to the Boss` toast shows. The offer stays open until the player takes an option. Without the Cut Knight card there is no boss, so the cut only shrinks the pool. No-op when not allowed or the index is invalid |
| `ApplyOption` (private) | Adds one copy (capped as above). A weapon raises its level; a passive's effect is recomputed, and for Vitality the change in the bonus (including the jump of a star-up) is added to the hero's max health and healed. A star-up of something already owned posts `X is now N-STAR!` and plays the `StarUp` sound (a brand-new weapon is not a star-up) |
| `GetOfferDescriptions() const` | Text for the UI (`DescribeOption`), empty when nothing is pending: `NEW weapon: Bolt` for a weapon the run does not own, or `Swiftness  Lv0 -> Lv1   (2 more to 2-STAR)`, or `>> 2-STAR!` when that copy completes a star. An option with `bIsNewUnlock` is prefixed `NEW! `: a passive reads `NEW! Haste  Lv0 -> Lv1   (2 more to 2-STAR)`, and a weapon that is both new to the camp and new to the run reads `NEW! Chain  (unlocked weapon)` |
| `GetOfferCards() const` | The same information in three pieces per card (`FOfferCardText`: `Title`, `Detail`, `Note`, `bHighlight`) for the card UI: title with `NEW!  `, detail `Lv0  >  Lv1` or `New weapon` / `Unlocked weapon`, note `2 more to 2-STAR` or `2-STAR UP!`; the card is highlighted for a new unlock or a star-up |
| `GetLoadoutSummary()` | One line, e.g. `Blade Lv3**   Might Lv1*` (one asterisk per star) |
| Passive getters | `GetMoveSpeedMultiplier`, `GetDamageMultiplier`, `GetXpGainMultiplier`, `GetAttackSpeedMultiplier` are `1 + total`; `GetMaxHealthBonus`, `GetMagnetRadius` (`BaseMagnetRadius + total`) and `GetRegenPerSecond` use the total; `GetRangeMultiplier` is `1 + Reach total`; `GetDamageReduction` is the Armor total clamped to 0..0.75; `GetCritChance` is the Crit total clamped to 0..0.9 and `GetCritMultiplier` the row's `CritMultiplier` (not scaled by stars) |
| `ResetProgression()` | Level 1, `XpToNext = BaseXp`, clears XP, starts with Blade at level 1, clears passives, the offer and all cuts (including the cutting-room floor), sets `PermanentCutsLeft = Stars.PermanentCutsForPool(GetPoolSize())` = `max(pool - MinPoolSize, 0)` (13 - 4 = 9 on a new save; every unlocked option adds one option and one cut, 12 at most), destroys leftover pickups, and logs `PROGRESSION pool=N permanentCuts=N` |
| `DebugGrantWeapon`, `DebugCut` | Test harness only (`-WeaponShowcase`, `-BossProbe`) |

Star rules, `FStarTuning` (in `Director Tuning` > `StarTuning`, not in a data table; the functions are pure and unit tested in `StarTuningTest.cpp`): two stars at 3 copies, three stars at 9; weapon damage x2.0 / x4.0 and reach x1.25 / x1.6; passive total x1.5 / x2.5; `MinPoolSize` 4 (options that always stay in the pool). Helpers: `StarsForCopies`, `CopiesToNextStar`, `WeaponDamageMultiplier`, `WeaponRangeMultiplier`, `PassiveMultiplier`, `PermanentCutsForPool`.

Locked options: in the C++ fallbacks (`DefaultWeapon`, `DefaultPassive`) Chain, Haste and Magnet have `bStartsLocked` true, and the shipped `DT_Weapons` / `DT_Passives` rows are meant to do the same (not readable here). They enter the pool only after a lost film reel unlocks them; see `UCampStockSubsystem` in section 10 and the reel rows in section 2.

Passive fallbacks (`DefaultPassive`, overridden by `DT_Passives` when it loads, all `MaxLevel` 5): Swiftness +10% move speed per stack, Might +15% damage, Vitality +20 max health with instant healing, Greed +25% XP, Haste +12% attack speed (`AttackSpeedAdd`, locked), Magnet +150 cm XP pickup radius (`MagnetRadiusAdd`, locked), **Reach** +12% weapon range per stack (`RangeAdd`), **Armor** 8% less contact damage per stack (`DamageReductionAdd`, total capped at 75%), **Regen** 1 health per second per stack (`RegenPerSecondAdd`), **Crit** +8% crit chance per stack, double damage (`CritChanceAdd`, `CritMultiplier` 2; total chance capped at 90%). A byte search of `DT_Passives.uasset` finds only the six old names, so Reach, Armor, Regen and Crit always use these fallbacks. The shipped table values exported at the end of story 009 are Swiftness +0.08, Might +0.15, Vitality +20, Greed +0.25, Haste +0.12 and Magnet +150 per stack; the XP curve row is base 30, growth 1.3, 5 XP per kill, magnet base 600 (not re-verified). A passive's total effect = `stacks x per-stack amount x star multiplier`, and the speed, damage, XP, attack-speed and range multipliers are `1 + total`.

[XpPickup.cpp](../../Game_01%205.8/Source/Game_01/Private/Progression/XpPickup.cpp) `AXpPickup`: a small green cube (scale 0.2), no tick, no collision; registers itself with the progression subsystem in `BeginPlay` and unregisters on a real destroy. The subsystem moves and collects it.

## 8. A run `URunOutcomeSubsystem`

[RunOutcomeSubsystem.cpp](../../Game_01%205.8/Source/Game_01/Private/Extraction/RunOutcomeSubsystem.cpp). `bRunActive` is true from world start.

| Function | Behavior |
|---|---|
| `ReportExtracted(ULootBagComponent*)` | Ends with `Extracted` |
| `ReportPlayerDied(ULootBagComponent*)` | Ends with `Died` (also used by the Esc menu's "Return to camp") |
| `EndRun` (private) | Ignored if already ended (the bag must not be null). `FRunSettlement::Compute(wood, stone, reels, multiplier, outcome)`: extracted: `IronGained = max(0, floor(wood x multiplier))`, `StoneGained` likewise, and every reel is banked; died: the whole bag (wood, stone, reels) is reported as lost. `AddIron` and `AddStone` into the stock (each ignores 0); on extraction `FReelUnlocks::OptionsForReels` maps the reels carried out to their options (rows from the bag's config) and `UnlockOption` unlocks each new one (stored in `FRunResult::OptionsUnlocked`); dying unlocks nothing; `SaveNow` (the stock change events have already saved once or more, this one is the explicit "after every run" save); clears the bag; resets heat; logs `EXTRACTED: ...` or `DIED: ...`; plays the `Extraction` or `PlayerDeath` sound; broadcasts `OnRunEnded` |
| `StartNewRun()` | Resets state and `LastResult`, keeps `EarningsMultiplier` |
| `SetEarningsMultiplier(float)` | At least 0; set by `UFilmCardSubsystem` at the start of a run (and back to 1 when the run ends, after settlement has used it) |

`FRunResult` keeps the old field names for wood: `BagIron`, `IronGained`, `IronLost`; stone is `BagStone`, `StoneGained`, `StoneLost`. The multiplier applies to wood and stone alike.

## 9. Extraction zone `AExtractionZone`

[ExtractionZone.cpp](../../Game_01%205.8/Source/Game_01/Private/Extraction/ExtractionZone.cpp). The occupant is the first Pawn that has a `ULootBagComponent`; others are ignored while it is inside. Progress = progress + `DeltaSeconds / ExtractionSeconds` (default 5 s), and when full the bar is zeroed and `ReportExtracted` is called. On leaving: if `bResetProgressOnLeave` (default true) is true (or the bar is at 0), progress is zeroed and `OnExtractionInterrupted` is broadcast (only if the bar was above 0), otherwise the bar drains by `ProgressDecayPerSecond` (a fraction of the full bar per second, default 0.5) until it reaches 0. When the run has ended (for example by death) the bar is dropped; the zone keeps ticking only if the occupant is still inside, so a new run fills without re-entering. A Pawn already standing inside when the level starts is registered late on the next frame via `SetTimerForNextTick`, because the initial overlap does not trigger an event. When a pawn enters during an active run it plays the `ExtractStart` sound and posts `EXTRACTING - stay inside the green circle`. The tuning (`FExtractionTuning`) lives on the actor; there is no data table for it. Progress is exposed only through `GetProgress()` and `OnExtractionProgress` (broadcast every tick it changes), and the HUD reads the former (it shows the furthest-filled zone when a map has several). The zone only ticks while someone is inside or the bar is draining.

Look: the root is an 800 x 800 x 200 cm box overlap volume (`OverlapOnlyPawn`); a flat green disc on the floor, a tall green beam (scale 0.6 x 14, so the exit can be found from across the map), a yellow fill disc that grows from the middle with the bar (`max(progress, 0.01) x 8` scale), and, when the Tiny Fantasy Forest pack is present, the `SM_PortalA` portal mesh standing in the zone (hidden when the pack is missing). The beam stays either way.

## 10. Camp

### `UCampMetaSubsystem`

[CampMetaSubsystem.cpp](../../Game_01%205.8/Source/Game_01/Private/Camp/CampMetaSubsystem.cpp). Building rows come from `BuildingTable` (rows `Smithy` and `Clinic`, keyed by name). The Smithy costs **stone**, the Clinic costs **wood** (`static GetCostResource`).

| Function | Behavior |
|---|---|
| `int32 GetUpgradeCost(EBuildingType)` | `floor(BaseCost x CostGrowth^current level)`, default 10 x 2^level (max level 5), in the building's resource. The row field comments still say "Iron cost" |
| `bool TryUpgrade(EBuildingType, FText& Reason)` | Returns false with a message (`<Building> is already at max level`, or `Not enough stone (need N)` / `Not enough wood (need N)`) when already at max level or the stock is insufficient; on success it spends the resource (`SpendStone` for the Smithy, `SpendIron` for the Clinic), raises the level, plays the `Upgrade` sound and broadcasts `OnMetaChanged` |
| `float GetWeaponDamageBonus() const` | Smithy level x `DamagePerLevel` (default 5), a flat amount added to every weapon before the multipliers |
| `float GetMaxHealthBonus() const` | Clinic level x `MaxHealthPerLevel` (default 25) |
| `RestoreLevels(const TMap<...>&)` | Used when loading the save; each level is clamped to 0 through `MaxLevel`, and keys not in `Buildings` are discarded; broadcasts `OnMetaChanged` |
| `GetLevels()`, `ResetCamp()`, `IsMaxLevel`, `GetBuildingRow` | Levels copy for saving; reset to level 0 (and broadcast); max-level test; the row with its defaults |
| `TArray<FName> PendingFilmCards` | Cards picked before a run, kept across levels, not saved to disk |

### `UCampStockSubsystem`

[CampStockSubsystem.cpp](../../Game_01%205.8/Source/Game_01/Private/Extraction/CampStockSubsystem.cpp). Holds wood (field `Iron`, read with `GetIron()` or its alias `GetWood()`), stone (`GetStone()`) and the unlocked options. `AddIron` and `AddStone` (non-positive ignored), `SpendIron` and `SpendStone` (a non-positive amount returns true and changes nothing; returns false and changes nothing if the stock is insufficient), `UnlockOption(FName)` (returns true only when the option is new; `None` and repeats return false; a new option is also appended to `NewOptions`), `IsOptionUnlocked` (no caller: the progression subsystem reads the whole list through `GetUnlockedOptions`), `GetUnlockedOptions` (in unlock order), `RestoreState(Iron, TArray<FName> Unlocked, TArray<FName> NewOptions = {}, int32 Stone = 0)` (amounts clamped to >= 0, options de-duplicated and `None` dropped; a NEW marker is kept only if that option is unlocked). Every change broadcasts `OnStockChanged` (the header comment still says "Iron or the warehouse"). `AddRelics`, `RemoveRelicAt` and `GetWarehouseRelics` were removed in story 008.

The "NEW" marker (story 009): `NewOptions` lists the options unlocked since the player last saw an offer. `TakeNewOptions()` returns the list and empties it (`MoveTemp`); `URunProgressionSubsystem::GenerateOffer` calls it, so the first offer of the next run puts those options first and marks them. `GetNewOptions()` is the read-only view that the save uses. Taking does not broadcast and does not write the save, so after a quit the markers come back once more from the file, which is harmless.

### `UCampSaveSubsystem` and `UCampSaveGame`

[CampSaveSubsystem.cpp](../../Game_01%205.8/Source/Game_01/Private/Flow/CampSaveSubsystem.cpp), [CampSaveGame.h](../../Game_01%205.8/Source/Game_01/Public/Flow/CampSaveGame.h). One slot, named `CampSave` unless the command line has `-SaveSlot=Name` (`GetSlotName()`, read once, so a probe or a clean-save test never touches the real save).

- `Initialize` loads the slot after the stock and meta subsystems exist and restores them with `bApplyingLoad` set (so restoring does not save), then listens to `OnStockChanged` and `OnMetaChanged`: every change writes the slot. A slot that exists but cannot be loaded logs a warning and the camp starts fresh.
- `SaveNow()` writes `SaveVersion = CurrentSaveVersion` (2), `Iron` (wood), `Stone`, `UnlockedOptions`, `NewOptions` (the not-yet-seen NEW markers, added in story 009 without a version bump) and `BuildingLevels`; returns false if the write fails. `Stone` is also a field added without a version bump: an older file reads it back as 0, and an older file's `Iron` becomes wood. Load passes `Save->NewOptions` and `Save->Stone` to `RestoreState`; `DeleteSaveAndReset` calls `RestoreState(0, {})`, so it clears them too. Film cards and relics are not saved.
- `SaveVersion` defaults to 1 on purpose: a save stores only values that differ from the class defaults, so a v1 file has no `SaveVersion` and reads back as 1. A save below version 2 is migrated at load: each `WarehouseRelics` entry becomes the option its reel row names (`FReelUnlocks::OptionsForReels`). `WarehouseRelics` is read from v1 saves only and never written.
- `DeleteSaveAndReset()` deletes the slot, empties the stock and resets the buildings with saving suppressed.

### Camp scene actors

`ACampGameMode` ([CampGameMode.cpp](../../Game_01%205.8/Source/Game_01/Private/Camp/CampGameMode.cpp), default pawn `ACampPawn`) starts the camp music and lays out the camp on an **equilateral triangle** of radius 700 cm around the origin (`CampTriangleRadius`), at height 60: the **Clinic** at (700, 0) turned 180 degrees (its porch faces the camera), the **Smithy** at (-350, -606) and the decorative **Warehouse** at (-350, 606), both turned 90 degrees. Buildings are spawned deferred so `BuildingType` is set before `BeginPlay`. The two buildings are spawned only if the level contains no `ACampBuilding` at all (a hand-placed one skips both), the warehouse only if the level has no `ACampWarehouse`.

`ACampPawn` ([CampPawn.cpp](../../Game_01%205.8/Source/Game_01/Private/Camp/CampPawn.cpp)) is not walkable: a spring arm of length **2300 cm** at pitch -75 (absolute rotation, no collision test) holds the camera. It shows the cursor, creates the camp screen (`UCampWidget`) and the Esc menu (`UPauseMenuWidget`, context `Camp`, z-order 50). A left click while the menu is closed traces `ECC_Visibility` under the cursor; if it hits an `ACampBuilding` it calls `TryUpgrade` and posts a toast (green on success, the refusal reason in red otherwise). Esc and the screen's MENU button call `TogglePauseMenu`.

`ACampBuilding` ([CampBuilding.cpp](../../Game_01%205.8/Source/Game_01/Private/Camp/CampBuilding.cpp)) is built from pack models, and the cube that used to be the building is now the **click target**:

- Root `Mesh` is the engine cube with block-all collision (clickable through the visibility trace). A `ModelRoot` scene component (absolute scale and location) holds the model parts.
- Smithy: one static mesh, the factory from the WW2 RTS & SLG Production Buildings pack (model scale 0.6; the factory is 1200 cm wide at 1.0). Clinic: the Cozy Low Poly House, **five parts that share one pivot** (`SM_Foundation`, `SM_HouseMain`, `SM_Roof`, `SM_Porch`, `SM_PricksTube`; model scale 0.45). If not all five are found, none is used (a half house would look broken).
- With a model, the cube is made invisible but kept for clicks, sized to about the model's width (4 m for the clinic, 7 m for the smithy, 3 m tall), and `SetLevel` grows the model's Z scale by 8% per level (no tint). Without a model the old behaviour remains: the cube gets taller (`Z = 1.2 + 0.5 x level`) and is tinted (Smithy orange, Clinic green).
- `BeginPlay` listens to `OnMetaChanged` and syncs its level; `HandleMetaChanged` calls `SetLevel`.

`ACampWarehouse` ([CampWarehouse.cpp](../../Game_01%205.8/Source/Game_01/Private/Camp/CampWarehouse.cpp)): the barracks mesh of the WW2 pack (scale 0.55) over an invisible 6 m click cube, blue tint on the cube if no model. It has no function since story 008 (clicking it does nothing, only `ACampBuilding` is handled).

`-CampShot` belongs to `ACampGameMode::TakeCampShot`: in the camp map it waits 4 seconds, logs `CAMPSHOT iron=... unlocked=[...]` (the iron value is the wood stock), saves `Saved/ReelProbe/camp-storage.png` and exits 1.5 seconds later.

## 11. Film cards `UFilmCardSubsystem`

[FilmCardSubsystem.cpp](../../Game_01%205.8/Source/Game_01/Private/Editing/FilmCardSubsystem.cpp). A world subsystem, so it is rebuilt with every level. The card list comes only from `FilmCardTable` (there is no C++ fallback list: with no table, no card can be added).

| Function | Behavior |
|---|---|
| `AddCard(FName)` | False if the slots are full (2), the card is already picked, or it is not in the table |
| `RemoveCard(FName)` | True if it was slotted |
| `ApplyActiveCards()` | `ForceHorde` calls `SpawnHorde(HordeCount, true)`, `ForceBoss` calls `SpawnBoss()` (which also arms `UBossRejectsSubsystem`), then the multipliers of all cards are multiplied and set on `URunOutcomeSubsystem` |
| `ResetCards()` | Clears the slots and sets the multiplier back to 1.0; called automatically when a run ends (`OnRunEnded`) |
| `DescribeCard(FName)` | For example `HordeNight: a horde of 40 grunts at the start, Loot earned x1.50`; a boss card says `a boss at the start`; an unknown id returns the id |
| Persistence | `Initialize` restores from `PendingFilmCards` (only cards that exist in the table, at most 2); `AddCard`, `RemoveCard` and `ResetCards` write back |

`DT_FilmCards` defines the two cards `HordeNight` and `CutKnight`; the numbers live in the asset (the `FFilmCardRow` fallbacks are 40 grunts and x1.5). The screenshots and the `DescribeCard` output are authoritative for the values.

## 12. Probes `ADirectorGameMode`

[DirectorGameMode.cpp](../../Game_01%205.8/Source/Game_01/Private/Game/DirectorGameMode.cpp). Default pawn `AHeroCharacter`; `BeginPlay` starts the run music. Every probe is switched on by a command-line flag and exits the game when done; without any flag nothing probe-related runs (`Tick` is not enabled).

- `-PerfProbe`: after a 3-second delay the hero goes into god mode, weapons are turned off, the scheduler is turned off, `SpawnHorde(150, true)` is called, it samples for 10 seconds, writes one line `PERFPROBE enemies=... avgFPS=... minFPS=... samples=...` to `LogEnemies`, and then exits.
- `-OfferProbe` (`StepOfferProbe`; uses `OfferProbe.StartDelaySeconds` 3 s and `StepSeconds` 1.5 s): puts the hero in god mode, turns the scheduler off, forces a level-up, plays 6 more (`OfferProbe.LevelUps`), then one more. Each offer is resolved by `ResolveOffer`: when a cut is allowed it cuts the first option that is not the one it will keep (the Chain if offered, else index 0, `KeepIndex`) and then takes the kept one, so a Chain weapon on offer is always taken. It saves `1-first-offer.png`, `2-loadout-after-cuts.png` and `3-late-offer.png` to `Saved/OfferProbe/` and logs `OFFERPROBE <label> level=.. pool=.. cutsLeft=.. cutSoFar=.. attackSpeed=.. magnetRadius=.. loadout=[..]` lines. After the last level-up it spawns a 15-enemy horde and binds `OnWeaponFired`: the first hit of every owned non-Chain weapon is photographed as `fx-<Weapon>.png`, and, if the run owns the Chain, it waits up to eight steps for a Chain attack that hits two or more enemies, saves `4-chain-fire.png` and logs `OFFERPROBE chain fired: hit N enemies in one attack`; the last line is `OFFERPROBE chain: owned=0|1 multiHitShot=0|1`. Whether the Chain shows up depends on the save: it needs the `Chain` option unlocked in the slot in use. The header comment still says the probe "cuts the first option each time".
- `-ReelProbe` or `-ReelProbe=die` (`StepReelProbe`): puts 25 wood and reels in the bag (hero in god mode, spawning off), then ends the run by extraction, or by death with `=die`. `-ReelCount=N` (clamped to 1 through 3, default 1) chooses how many reel types go in, in the order Fast Cut, Wide Shot, Chain Scene. It saves screenshots to `Saved/ReelProbe/` (`1-reel-in-bag.png`, then `2-result-extracted.png` or `3-result-died.png`) and logs `REELPROBE <label> reelsInBag=... heat=... unlocked=[...] anotherReelCanDrop=... outcome=...` lines. It changes the real save unless you also pass `-SaveSlot=`.
- `-PaceProbe`: a bot plays a whole run: the hero walks in a circle (`Tick` adds movement input), it always takes the first offered option and never cuts (the header comment says it cuts, the code does not), global time dilation `-PaceSpeed=N` (clamped 1 to 8, default 4), logs `TUNING ...` lines once, then `PACEPROBE t=..s level=.. hp=.. alive=.. spawnRate=.. loadout=[..]` every 30 game seconds, and ends on death, extraction or after 600 s with `PACEPROBE END outcome=...`. `-PaceSpawnBase=`, `-PaceSpawnRamp=`, `-PaceGruntHealth=`, `-PaceGruntContact=` and `-PaceHeroHealth=` override the loaded values for that run only.
- `-WeaponShowcase`: after 3 s, god mode, `DebugGrantWeapon` Gun and Orbit at 3 copies, a 30-enemy horde, two screenshots to `Saved/Showcase/`, exit at 5 s.
- `-PauseProbe`: opens the Esc menu, presses Settings and Volume buttons, presses Return to camp twice (confirm, then leave), screenshots to `Saved/PauseProbe/`. Because the menu pauses the world and a paused world runs no timers, the later steps ride on the engine's core ticker (`FTSTicker`).
- `-EdgeProbe`: teleports the hero to (6000, 3900) outside the floor, spawns 40 Grunts and a Boss, logs `EDGEPROBE hero=... farthest enemy=...` after 2 s (checks the play-area clamp) and screenshots `Saved/EdgeProbe/hero-at-edge.png`.
- `-BossProbe`: spawns a boss 900 cm from the hero and, with `DebugCut`, "cuts" the weapons Gun, Nova and Orbit and the passive Greed, at 1, 2 and 3 seconds, photographing the balls, the slam warning and the ring of balls into `Saved/BossProbe/`.
- `-ZoneShot`: teleports the hero near the extraction zone, then into it, and screenshots `Saved/ZoneShot/1-zone-from-nearby.png` and `2-zone-filling.png`; logs `ZONESHOT progress=`.

## 13. Audio `UGameAudioSubsystem`

[GameAudioSubsystem.cpp](../../Game_01%205.8/Source/Game_01/Private/Audio/GameAudioSubsystem.cpp), header [GameAudioSubsystem.h](../../Game_01%205.8/Source/Game_01/Public/Audio/GameAudioSubsystem.h). The one place gameplay asks for sound. A world subsystem created only in game worlds (`ShouldCreateSubsystem`: `IsGameWorld`). The assets are set in `Director Tuning` > `Audio` (`Sounds` map, `CampMusic`, `RunMusic`); `DefaultGame.ini` assigns sounds from the Free Sounds Pack and the two music loops from Path of Adventure (`CUE_Village_Loop` for the camp, `CUE_Battle_Loop` for a run).

Enums: `EGameSound` has 11 values: `Attack`, `EnemyHit`, `EnemyDeath`, `Pickup`, `LevelUp`, `Cut`, `StarUp`, `Extraction`, `PlayerDeath`, `Upgrade`, `ExtractStart` (plus a hidden `Count`). `EMusicTrack` is `Camp` or `Run`. `EAudioBus` is `Master`, `Music`, `Effects`.

| Function | Behavior | Edge cases |
|---|---|---|
| `Initialize` | Loads every assigned sound synchronously (a hitch mid-fight is worse than a longer load) and logs `AUDIOCFG sounds assigned=N of 11, master=.. music=.. sfx=.. crowd cap=../s` | An empty map entry means silence |
| `Play(EGameSound)` | Counts the request; for the crowd sounds (`Attack`, `EnemyHit`, `EnemyDeath`) allows at most `MaxCrowdSoundsPerSecond` (16) starts in the last second of **real** time, further ones count as `Capped` and are dropped; logs the first request of each sound once (`asset assigned` or `no asset assigned, silent`); plays `PlaySound2D` at volume `Master x Effects` | A missing asset is a silent no-op, but the request is still counted (and a crowd request still uses up cap) |
| `static PlayIn(WorldContext, Sound)` | Safe with a null context or world (does nothing) | - |
| `StartMusic(EMusicTrack)` | Does nothing if that track is already playing; otherwise fades the other one out over 0.5 s and starts the new one at volume `Master x Music` (looping depends on the cue asset). Called by `ACampGameMode::BeginPlay` (Camp) and `ADirectorGameMode::BeginPlay` (Run) | No asset: logged as silent |
| `static GetBusVolume(EAudioBus)` / `static SetBusVolume(WorldContext, Bus, Volume)` | The player's value for this session (a static array, one entry per bus, `-1` = untouched, survives map changes but not a restart), else `MasterVolume` 0.8, `MusicVolume` 0.35, `SfxVolume` 0.7 from `Director Tuning`. `SetBusVolume` clamps to 0..1 and updates the music that is playing now | Effects pick up a new volume with their next sound. The Esc menu is the only caller. The setting's comment "there is no settings screen" is stale |
| `GetRequestCount`, `GetCappedCount` | Counters for the probes and the log | - |
| `Deinitialize` | Logs the capped total if there was one, stops the music | - |

Call sites: `Attack` (a weapon attack with hits > 0), `EnemyHit`, `EnemyDeath` (`AEnemy::ApplyDamage`), `Pickup` (XP gem collected, container opened), `LevelUp` (offer shown), `Cut`, `StarUp`, `Extraction` and `PlayerDeath` (`EndRun`), `Upgrade` (`TryUpgrade`), `ExtractStart` (entering the zone).

## 14. Play area and forest `UPlayAreaSubsystem`, `AForestDressing`

[PlayAreaSubsystem.cpp](../../Game_01%205.8/Source/Game_01/Private/Game/PlayAreaSubsystem.cpp), [ForestDressing.cpp](../../Game_01%205.8/Source/Game_01/Private/Game/ForestDressing.cpp). The maps have no walls and a void around the floor.

`UPlayAreaSubsystem::OnWorldBeginPlay` goes through every `AStaticMeshActor` and counts as **floor** any whose bounds have a smaller horizontal half-extent of at least 1000 cm and a vertical half-extent of at most 0.2 times that (large and thin; walls, props and greybox cubes do not match). Every candidate is logged (`PLAYAREA candidate ...`). The area is the **union rectangle** of all floors (XY only). With no floor found it logs a warning and does nothing, so such a map behaves as before. `ClampToArea(Location, Margin)` pulls X and Y inside the rectangle by `Margin`, Z is untouched. Callers: `AHeroCharacter::Tick` (margin = capsule radius), `UEnemyManagerSubsystem` apply loop (margin = body radius) and `UEnemySpawnSubsystem::SpawnOnRing` (ring spawn point). The older note that the floor is `Floor_0` spanning -4000 to 4000 on both axes is a map fact that this code does not state (the `-EdgeProbe` test point of 6000 / 3900 is consistent with it, not proof).

After finding the area, if the game mode is `ADirectorGameMode` or `ACampGameMode` (so the camp is **also** dressed, if its map has a floor that matches), it calls `AForestDressing::Build(World, Area, FloorTopZ, FloorActor)`, which logs `FOREST dressing built` or `skipped (pack missing)`. Only the last floor candidate found is kept as `FloorActor`/`FloorTopZ`, so with several floor pieces only the last is hidden.

`AForestDressing` (not placeable) uses meshes of the RPG Tiny Fantasy Forest pack. `Build` spawns it, destroys it again and returns false if the land mesh (`SM_LM100RND`) is missing, otherwise `Fill`s it and hides the old floor with `SetActorHiddenInGame` (hidden, not removed: it still carries the collision the hero stands on). `Fill`: one land mass (10000 cm square, scaled up to `(longest side + 2 x 1800) / 10000` when larger, placed 1 cm above the floor top); a rim outside the play area starting 900 cm past its edge (the camera can sit up to about 700 cm behind the hero, so nothing stands closer): 160 trees (`SM_TreeB`, scale 1 to 1.8), 40 rocks (`SM_RockA`) and 10 cliffs (`SM_RockCliffE`) as instanced meshes, plus 500 ground details inside the area (a third flowers `SM_FlowerA`, the rest grass `SM_GrassA`). A fixed random seed (20261115) makes it identical every launch. Nothing has collision or casts shadows.

## 15. Esc menu `UPauseMenuWidget`

[PauseMenuWidget.cpp](../../Game_01%205.8/Source/Game_01/Private/UI/PauseMenuWidget.cpp), built entirely in C++ like the other screens (see [ui.md](ui.md)). Created by `AHeroCharacter::CreateHud` (context `Run`) and `ACampPawn::BeginPlay` (context `Camp`), added at z-order 50, hidden until Esc.

| Member | Behavior | Edge cases |
|---|---|---|
| `Toggle()` / `Open()` / `Close()` | Open shows the panel; in a **run** it also pauses the whole game (`SetGamePaused`), in the camp it does not. Close unpauses and returns keyboard focus to the game | Esc also closes it through `NativeOnKeyDown`, because after a button click the focus sits on the button and the key no longer reaches the pawn |
| Buttons | Resume; Settings (shows or hides the three volume rows `Master`, `Music`, `Effects`, each `-` / `+` in steps of 10% through `UGameAudioSubsystem::SetBusVolume`; session only, not saved); in a run "Return to camp (lose bag)"; Quit game (`RequestExit`). The panel also lists the controls for the context and the credits (CC BY music by Kounine, WW2 buildings by Winyunq, and the other packs) | - |
| `PressReturnToCamp()` | A run that is still active needs **two presses**: the first changes the label to "Click again: the bag is lost" and arms for 3 real seconds (`NativeTick` lapses it); the second calls `ReportPlayerDied(Bag)` (so the bag is lost exactly as if the hero died, with the sound, the save and the heat reset), posts `Run abandoned: the bag was lost` and opens `L_Camp`. A run that has already ended goes straight back | Public for the `-PauseProbe` (`PressForProbe(Action, Index)` does the same as a click) |

The cut keys 4 to 6 and "Cut" buttons in the level-up panel are unrelated to this menu; the hero ignores Esc while an offer is pending.

## 16. Leftovers

`AStandInCharacter` and `AStandInGameMode` ([StandInCharacter.cpp](../../Game_01%205.8/Source/Game_01/Private/StandIn/StandInCharacter.cpp)) are the throwaway story-001 pawn and game mode (WASD, E, K to die, N for a new run, on-screen debug text). Nothing in `Source/` uses them any more; they still compile and its debug text still calls the bag's wood "Iron". `UHudMessageSubsystem` (the last four messages of the session, shared by the run HUD and the camp screen) and the `UUiActionButton` / `DirectorUi` building blocks belong to the UI and are described in [ui.md](ui.md).
