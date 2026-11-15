// Director's Cut - Story 003 (design/game-brief.md, MVP feature 1).
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Enemies/EnemyTypes.h"
#include "Enemy.generated.h"

class UStaticMeshComponent;
class UMaterialInstanceDynamic;

/**
 * A greybox enemy: an engine cube with no collision, no tick and no movement component.
 * UEnemyManagerSubsystem moves and attacks for every enemy in one loop.
 */
UCLASS()
class GAME_01_API AEnemy : public AActor
{
	GENERATED_BODY()

public:
	AEnemy();

	/** Side length (cm) of the engine cube at scale 1. */
	static constexpr float EngineCubeSize = 100.0f;

	/** The row this enemy runs on. Set by Init. */
	UPROPERTY(VisibleInstanceOnly, Category = "Enemy")
	FEnemyTuning Tuning;

	/** World time (seconds) before which this enemy cannot hit again. Written by the manager. */
	double NextContactTime = 0.0;

	/** Applies a tuning row: scale, health. Call right after spawning. */
	void Init(const FEnemyTuning& InTuning);

	/** Reduces health. At 0 the kill is reported to the manager and the enemy is destroyed. */
	UFUNCTION(BlueprintCallable, Category = "Enemy")
	void ApplyDamage(float Amount);

	/** Half the body width (cm), used for the push-apart pass. */
	float GetBodyRadius() const { return Tuning.Scale * EngineCubeSize * 0.5f; }

	/** Ends the hit flash once its short time is up. Called by the manager's loop, so enemies need no tick of their own. */
	void UpdateHitFeedback(double Now);

	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

private:
	UPROPERTY(VisibleAnywhere, Category = "Enemy")
	TObjectPtr<UStaticMeshComponent> Body;

	float Health = 0.0f;

	/** Hit feedback (story 011): a white flash and a small scale punch for a tenth of a second. */
	UPROPERTY(Transient)
	TObjectPtr<UMaterialInstanceDynamic> BodyMaterial;
	FLinearColor BaseColor = FLinearColor::White;
	double HitFlashUntil = 0.0;
	bool bFlashing = false;
};
