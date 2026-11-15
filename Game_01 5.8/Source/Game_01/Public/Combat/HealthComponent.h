// Director's Cut - Story 003 (design/game-brief.md, MVP feature 1).
#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/DataTable.h"
#include "HealthComponent.generated.h"

/** Health tuning for one actor. Usable as a DataTable row; the initializers are the fallback. */
USTRUCT(BlueprintType)
struct GAME_01_API FHealthTuning : public FTableRowBase
{
	GENERATED_BODY()

	/** Starting and maximum health. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Health", meta = (ClampMin = "1.0"))
	float MaxHealth = 100.0f;

	/** Seconds of invulnerability granted after every hit that lands (stops a crowd from stacking damage). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Health", meta = (ClampMin = "0.0"))
	float HitInvulnerabilitySeconds = 0.25f;
};

/** Fired once when health reaches zero. */
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnHealthDied);
/** Fired for every hit that lands. Amount is the damage actually applied. */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnHealthDamaged, float, Amount, float, NewHealth);

/**
 * Health with a brief invulnerability window and a god-mode switch.
 * The owner applies any damage-taken multipliers before calling ApplyDamage.
 */
UCLASS(ClassGroup = (Combat), meta = (BlueprintSpawnableComponent))
class GAME_01_API UHealthComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UHealthComponent();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Health")
	FHealthTuning Tuning;

	/** While true no damage lands (used by the performance probe and debugging). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Health")
	bool bGodMode = false;

	UPROPERTY(BlueprintAssignable, Category = "Health")
	FOnHealthDied OnDied;

	UPROPERTY(BlueprintAssignable, Category = "Health")
	FOnHealthDamaged OnDamaged;

	/**
	 * Applies damage. Ignored while dead, in god mode or invulnerable. A surviving hit starts the
	 * hit invulnerability window. Returns the damage actually applied (0 if ignored).
	 */
	UFUNCTION(BlueprintCallable, Category = "Health")
	float ApplyDamage(float Amount);

	/** Drops health to zero regardless of god mode or invulnerability, and fires OnDied. No-op if already dead. */
	UFUNCTION(BlueprintCallable, Category = "Health")
	void Kill();

	/** Restores full health and clears invulnerability (new run). */
	UFUNCTION(BlueprintCallable, Category = "Health")
	void ResetHealth();

	/** Restores health up to the maximum. Does nothing when dead. */
	void Heal(float Amount);

	/** Raises max health and heals by the same amount (story 004 Vitality passive). */
	UFUNCTION(BlueprintCallable, Category = "Health")
	void IncreaseMaxHealth(float Amount);

	/** Sets max health to NewMax and refills health, clearing invulnerability (story 006 Clinic bonus at run start). */
	UFUNCTION(BlueprintCallable, Category = "Health")
	void SetMaxHealth(float NewMax);

	/** Max health captured from the tuning table at BeginPlay, before any run bonuses. */
	UFUNCTION(BlueprintPure, Category = "Health")
	float GetBaseMaxHealth() const { return BaseMaxHealth; }

	/** Makes the owner immune for Seconds (the longer of this and any window already running wins). */
	UFUNCTION(BlueprintCallable, Category = "Health")
	void GrantInvulnerability(float Seconds);

	UFUNCTION(BlueprintPure, Category = "Health")
	bool IsDead() const { return Health <= 0.0f; }

	UFUNCTION(BlueprintPure, Category = "Health")
	bool IsInvulnerable() const;

	UFUNCTION(BlueprintPure, Category = "Health")
	float GetHealth() const { return Health; }

	UFUNCTION(BlueprintPure, Category = "Health")
	float GetMaxHealth() const { return Tuning.MaxHealth; }

	virtual void BeginPlay() override;

private:
	UPROPERTY(VisibleInstanceOnly, Transient, Category = "Health", meta = (AllowPrivateAccess = "true"))
	float Health = 0.0f;

	/** Tuning-table max health, captured once at BeginPlay. */
	UPROPERTY(VisibleInstanceOnly, Transient, Category = "Health", meta = (AllowPrivateAccess = "true"))
	float BaseMaxHealth = 100.0f;

	/** World time (seconds) until which damage is ignored. */
	double InvulnerableUntil = 0.0;
};
