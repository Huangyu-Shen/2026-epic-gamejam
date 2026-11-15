// Director's Cut - Story 012. The Cut Knight boss fights with whatever the player cut in this run.
#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "Progression/ProgressionTypes.h"
#include "BossRejectsSubsystem.generated.h"

class AEnemy;
class AActor;
class AHeroCharacter;
struct FOfferOption;

/** The three ways a boss attack reaches the hero. All of them can be seen coming and dodged. */
UENUM(BlueprintType)
enum class EBossAttackKind : uint8
{
	Shot UMETA(DisplayName = "Shot"), // one slow ball flying at the hero
	Slam UMETA(DisplayName = "Slam"), // a ring marked on the floor that hurts when its wind-up ends
	Ring UMETA(DisplayName = "Ring"), // balls flying outward in all directions
};

/** One boss attack, borrowed from a cut weapon. Damage here is its own value, not the player weapon's. */
USTRUCT(BlueprintType)
struct GAME_01_API FBossAttackTuning
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Boss")
	EBossAttackKind Kind = EBossAttackKind::Shot;

	/** Seconds between uses. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Boss", meta = (ClampMin = "0.2"))
	float Interval = 3.0f;

	/** Damage to the hero per hit, before Armor. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Boss", meta = (ClampMin = "0.0"))
	float Damage = 10.0f;

	/** Shot and Ring: ball speed (cm/s). Kept low enough to dodge (the hero walks at 600). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Boss", meta = (ClampMin = "0.0"))
	float Speed = 800.0f;

	/** Shot and Ring: how far a ball flies before it disappears (cm). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Boss", meta = (ClampMin = "0.0"))
	float Range = 2400.0f;

	/** Slam: radius of the marked ring (cm). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Boss", meta = (ClampMin = "0.0"))
	float Radius = 450.0f;

	/** Slam: seconds between the ring appearing and the damage. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Boss", meta = (ClampMin = "0.2"))
	float WindUp = 1.0f;

	/** Ring: how many balls fly out. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Boss", meta = (ClampMin = "1"))
	int32 Count = 1;
};

/** What each cut gives the boss. Fills in its own defaults; Project Settings > Game > Director Tuning can change them. */
USTRUCT(BlueprintType)
struct GAME_01_API FBossRejectTuning
{
	GENERATED_BODY()

	FBossRejectTuning();

	/** The attack a cut weapon gives the boss. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Boss")
	TMap<EWeaponType, FBossAttackTuning> Attacks;

	/** Cutting the same weapon again adds this share of the base damage to that attack. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Boss", meta = (ClampMin = "0.0"))
	float RepeatDamageStep = 0.25f;

	/** Swiftness cut: share added to the boss's move speed. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Boss", meta = (ClampMin = "0.0"))
	float MoveSpeedPerCut = 0.08f;

	/** Might cut: share added to the boss's contact damage. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Boss", meta = (ClampMin = "0.0"))
	float ContactDamagePerCut = 0.15f;

	/** Haste cut: every boss attack interval is multiplied by this (never below 0.5 in total). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Boss", meta = (ClampMin = "0.1", ClampMax = "1.0"))
	float AttackIntervalPerHaste = 0.9f;

	/** Vitality cut: health the boss gains. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Boss", meta = (ClampMin = "0.0"))
	float HealthPerVitalityCut = 150.0f;

	/** Any cut with no better mapping: a small health bonus, so no cut is ignored. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Boss", meta = (ClampMin = "0.0"))
	float GenericHealthPerCut = 100.0f;

	/** The most a stat bonus (move speed, contact damage) can reach, as a share of the base value. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Boss", meta = (ClampMin = "0.0"))
	float MaxStatBonus = 1.0f;

	/** The most health all cuts together can add. Keeps the boss beatable. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Boss", meta = (ClampMin = "0.0"))
	float MaxBonusHealth = 1500.0f;

	/** Distance (cm, floor plane) at which a ball hits the hero. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Boss", meta = (ClampMin = "10.0"))
	float ShotHitRadius = 90.0f;
};

/**
 * Watches the run's cut list and turns each cut into something the boss does to the hero: a cut weapon becomes
 * an attack, a cut passive raises a boss stat. Only exists in effect while a boss was registered by SetBoss
 * (the Cut Knight film card). It never gives anything to a dead boss.
 */
UCLASS()
class GAME_01_API UBossRejectsSubsystem : public UTickableWorldSubsystem
{
	GENERATED_BODY()

public:
	/** Called by the spawner when the boss appears. Every cut already on the cutting-room floor is applied on the next tick. */
	void SetBoss(AEnemy* InBoss);


	virtual void Tick(float DeltaTime) override;
	virtual TStatId GetStatId() const override { RETURN_QUICK_DECLARE_CYCLE_STAT(UBossRejectsSubsystem, STATGROUP_Tickables); }
	virtual void Deinitialize() override;

private:
	/** The three parts of a tick: start the attacks that are due, fly the balls, count down the slams. */
	void FireDueAttacks(const FBossRejectTuning& Tuning, AHeroCharacter& Hero);
	void MoveShots(const FBossRejectTuning& Tuning, AHeroCharacter& Hero, float DeltaTime);
	void CountDownSlams(AHeroCharacter& Hero, float DeltaTime);

	struct FShot
	{
		TWeakObjectPtr<AActor> Visual;
		FVector Direction = FVector::ForwardVector;
		float Speed = 0.0f;
		float DistanceLeft = 0.0f;
		float Damage = 0.0f;
	};

	struct FSlam
	{
		FVector Center = FVector::ZeroVector;
		float Radius = 0.0f;
		float Damage = 0.0f;
		float TimeLeft = 0.0f;
	};

	void ApplyCut(const FOfferOption& Cut);
	void RefreshBossStats();
	void FireAttack(EWeaponType Weapon, const FBossAttackTuning& Attack, float Damage, const AHeroCharacter& Hero);
	void SpawnShot(const FBossAttackTuning& Attack, float Damage, const FVector& Direction);
	void ClearAttacks();

	TWeakObjectPtr<AEnemy> Boss;
	bool bHadBoss = false;
	int32 Processed = 0;

	float BaseMoveSpeed = 0.0f;
	float BaseContactDamage = 0.0f;
	int32 SwiftnessCuts = 0;
	int32 MightCuts = 0;
	int32 HasteCuts = 0;
	float HealthGiven = 0.0f;

	/** Copies cut of each weapon (the boss has that attack while the count is above 0). */
	TMap<EWeaponType, int32> Copies;
	TMap<EWeaponType, double> NextUse;
	TArray<FShot> Shots;
	TArray<FSlam> Slams;
};
