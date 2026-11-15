// Director's Cut - Story 003 (design/game-brief.md, MVP feature 1). Replaces AStandInCharacter.
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "HeroCharacter.generated.h"

class UAutoWeaponComponent;
class UCameraComponent;
class UHealthComponent;
class ULootBagComponent;
class URunHudWidget;
class USpringArmComponent;
class UStaticMeshComponent;

/** Dodge tuning: a short fast dash with i-frames and a cooldown. */
USTRUCT(BlueprintType)
struct GAME_01_API FDodgeTuning
{
	GENERATED_BODY()

	/** Dash length in seconds. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dodge", meta = (ClampMin = "0.01"))
	float Duration = 0.18f;

	/** Dash speed (cm/s). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dodge", meta = (ClampMin = "0.0"))
	float Speed = 1800.0f;

	/** Seconds from the start of one dodge until the next may begin. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dodge", meta = (ClampMin = "0.0"))
	float Cooldown = 0.8f;

	/** Invulnerability granted at dodge start (seconds). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dodge", meta = (ClampMin = "0.0"))
	float InvulnerabilitySeconds = 0.25f;
};

/**
 * The player: top-down camera, raw-key controls, health, the bag and the auto weapon.
 *
 * Controls: WASD move, Space dodge, E search the nearest container, 1-3 cut a level-up option
 * while the offer shows. A lost film reel in the bag cannot be used. Film cards are slotted
 * in the camp editing room; the run starts from the camp (StartRun applies the cards and the
 * camp bonuses). Debug: K die. The HUD is URunHudWidget (story 007).
 */
UCLASS()
class GAME_01_API AHeroCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	AHeroCharacter();

	/** How far (cm) the E key reaches when looking for a container to search. */
	UPROPERTY(EditAnywhere, Category = "Hero")
	float InteractRange = 250.0f;

	/** Walk speed (cm/s) before the progression speed multiplier. */
	UPROPERTY(EditAnywhere, Category = "Hero")
	float BaseWalkSpeed = 600.0f;

	/** Camera distance above the pawn (cm). */
	UPROPERTY(EditAnywhere, Category = "Hero")
	float CameraDistance = 1400.0f;

	UPROPERTY(EditAnywhere, Category = "Hero")
	FDodgeTuning Dodge;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Hero")
	TObjectPtr<ULootBagComponent> Bag;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Hero")
	TObjectPtr<UHealthComponent> Health;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Hero")
	TObjectPtr<UAutoWeaponComponent> Weapon;

	/** Searches the nearest unsearched container within InteractRange. Returns true if one was searched. */
	UFUNCTION(BlueprintCallable, Category = "Hero")
	bool SearchNearestContainer();

	/** Starts a dodge in the current move direction (or facing). False if on cooldown, already dodging or unable to act. */
	UFUNCTION(BlueprintCallable, Category = "Hero")
	bool TryDodge();

	/** Contact damage from an enemy: applies the bag's damage-taken multiplier, then health. Returns damage applied. */
	UFUNCTION(BlueprintCallable, Category = "Hero")
	float ReceiveEnemyDamage(float Amount);

	UFUNCTION(BlueprintPure, Category = "Hero")
	bool IsDead() const;

	virtual void BeginPlay() override;
	virtual void Tick(float DeltaSeconds) override;
	virtual void SetupPlayerInputComponent(UInputComponent* PlayerInputComponent) override;

private:
	/** True while the run is active and the hero is alive. */
	bool CanAct() const;

	void SearchNearestContainerKey();
	void DodgeKey();
	void DebugDieKey();

	/** Esc quits the game. */
	void QuitKey();
	/** Resets the run systems and applies the camp bonuses and slotted film cards. Called once when the run map loads. */
	void StartRun();
	/** Creates the HUD and puts the game in "game + mouse" input mode so its buttons can be clicked. */
	void CreateHud();
	/** Bound to UHealthComponent::OnDied, so it has to be a UFUNCTION. */
	UFUNCTION()
	void HandleDied();

	UPROPERTY(Transient)
	TObjectPtr<URunHudWidget> Hud;

	UPROPERTY(VisibleAnywhere, Category = "Hero")
	TObjectPtr<UStaticMeshComponent> BodyMesh;

	UPROPERTY(VisibleAnywhere, Category = "Hero")
	TObjectPtr<USpringArmComponent> CameraBoom;

	UPROPERTY(VisibleAnywhere, Category = "Hero")
	TObjectPtr<UCameraComponent> Camera;

	/** Last WASD direction on the XY plane (zero when no key is held). */
	FVector MoveInput = FVector::ZeroVector;

	FVector DodgeDirection = FVector::ZeroVector;
	float DodgeTimeRemaining = 0.0f;
	double NextDodgeTime = 0.0;
};
