// Director's Cut - Story 003 (design/game-brief.md, MVP feature 1). Story 004 adds more weapons.
#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "AutoWeaponComponent.generated.h"

class UHealthComponent;
class ULootBagComponent;

/** Tuning for the starting weapon. */
USTRUCT(BlueprintType)
struct GAME_01_API FWeaponTuning
{
	GENERATED_BODY()

	/** Damage per hit, before the bag's damage multiplier. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon", meta = (ClampMin = "0.0"))
	float Damage = 15.0f;

	/** Seconds between hits. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon", meta = (ClampMin = "0.01"))
	float Interval = 0.4f;

	/** Reach (cm, XY plane) from the owner. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon", meta = (ClampMin = "0.0"))
	float Range = 600.0f;

	/** How long the debug hit line stays on screen. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon", meta = (ClampMin = "0.0"))
	float HitLineSeconds = 0.12f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon", meta = (ClampMin = "0.0"))
	float HitLineThickness = 6.0f;
};

/**
 * The starting weapon: every Interval it hits the nearest enemy in Range, no button press.
 * Reads the owner's ULootBagComponent for the damage multiplier, and stays quiet while the
 * run is over or the owner is dead.
 */
UCLASS(ClassGroup = (Combat), meta = (BlueprintSpawnableComponent))
class GAME_01_API UAutoWeaponComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UAutoWeaponComponent();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon")
	FWeaponTuning Tuning;

	/** Turn the weapon off (the performance probe does this so enemies stay alive). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon")
	bool bEnabled = true;

	virtual void BeginPlay() override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

private:
	UPROPERTY(Transient)
	TObjectPtr<ULootBagComponent> OwnerBag;

	UPROPERTY(Transient)
	TObjectPtr<UHealthComponent> OwnerHealth;

	float CooldownRemaining = 0.0f;
};
