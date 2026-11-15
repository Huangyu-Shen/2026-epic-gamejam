// Director's Cut - Story 003 (design/game-brief.md, MVP feature 1). Story 004 turns this into a three-weapon system.
#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Progression/ProgressionTypes.h"
#include "AutoWeaponComponent.generated.h"

class UHealthComponent;
class AEnemy;
class AOrbitOrb;
class UEnemyManagerSubsystem;
struct FWeaponRow;

/** Fired every time a weapon fires: which weapon and how many enemies it hit. Story 011's audio and hit feedback bind here. */
DECLARE_MULTICAST_DELEGATE_TwoParams(FOnWeaponFired, EWeaponType /*Weapon*/, int32 /*EnemiesHit*/);

/**
 * The hero's weapon system: fires every owned weapon on its own interval, no button press.
 * Weapon stats and levels come from URunProgressionSubsystem (data tables, story 004).
 * Blade hits the nearest enemy, Bolt pierces a line, Nova bursts in a radius, Chain hits the nearest enemy and
 * jumps to others nearby (unlocked by a lost film reel, story 009).
 */
UCLASS(ClassGroup = (Combat), meta = (BlueprintSpawnableComponent))
class GAME_01_API UAutoWeaponComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UAutoWeaponComponent();

	/** Turn the weapons off (the performance probe does this so enemies stay alive). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon")
	bool bEnabled = true;

	FOnWeaponFired OnWeaponFired;

	virtual void BeginPlay() override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

private:
	/** The star reach multiplier of the weapon being fired; scales the visual's thickness so a star-up looks bigger. */
	float CurrentReach = 1.0f;

	/** Orbit: keeps the balls circling the hero and hurts what they touch. Runs every tick, not on an interval. */
	void TickOrbit(float DeltaTime, int32 Level, const FWeaponRow& Row, float DamageMultiplier);

	/** Rolls the Crit passive: returns Damage, doubled (or more) when the attack crits. */
	float RollCrit(float Damage) const;

	TArray<TObjectPtr<AOrbitOrb>> Orbs;
	float OrbitAngle = 0.0f;
	TMap<TWeakObjectPtr<AEnemy>, double> OrbitHitReady;

	/** Everything one shot of a weapon needs: who to ask for enemies, where the hero is, the weapon's row, its level and the damage rolled for this shot. */
	struct FFireContext
	{
		UEnemyManagerSubsystem* Manager;
		FVector Origin;
		const FWeaponRow& Row;
		int32 Level;
		float Damage;
	};

	/** Fires one weapon. Returns how many enemies it hit. */
	int32 FireWeapon(EWeaponType Type, int32 Level, const FWeaponRow& Row, float DamageMultiplier);

	/** One function per weapon; each returns how many enemies it hit (the Gun returns 1 when a shot was fired). */
	int32 FireBlade(const FFireContext& Context);
	int32 FireBolt(const FFireContext& Context);
	int32 FireNova(const FFireContext& Context);
	int32 FireChain(const FFireContext& Context);
	int32 FireGun(const FFireContext& Context);

	/** Damages every collected target and returns how many were hit. */
	static int32 DamageAll(const TArray<AEnemy*, TInlineAllocator<32>>& Targets, float Damage);

	UPROPERTY(Transient)
	TObjectPtr<UHealthComponent> OwnerHealth;

	TMap<EWeaponType, float> Cooldowns;
};
