// Director's Cut - Story 003 (design/game-brief.md, MVP feature 1). Story 004 turns this into a three-weapon system.
#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Progression/ProgressionTypes.h"
#include "AutoWeaponComponent.generated.h"

class UHealthComponent;
class ULootBagComponent;

/**
 * The hero's weapon system: fires every owned weapon on its own interval, no button press.
 * Weapon stats and levels come from URunProgressionSubsystem (data tables, story 004).
 * Blade hits the nearest enemy, Bolt pierces a line, Nova bursts in a radius.
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

	virtual void BeginPlay() override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

private:
	void FireWeapon(EWeaponType Type, int32 Level, const FWeaponRow& Row, float DamageMultiplier);

	UPROPERTY(Transient)
	TObjectPtr<ULootBagComponent> OwnerBag;

	UPROPERTY(Transient)
	TObjectPtr<UHealthComponent> OwnerHealth;

	TMap<EWeaponType, float> Cooldowns;
};
