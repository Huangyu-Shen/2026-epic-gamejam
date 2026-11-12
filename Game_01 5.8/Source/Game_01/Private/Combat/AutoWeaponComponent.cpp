#include "Combat/AutoWeaponComponent.h"
#include "Combat/HealthComponent.h"
#include "DrawDebugHelpers.h"
#include "Enemies/Enemy.h"
#include "Enemies/EnemyManagerSubsystem.h"
#include "Extraction/RunOutcomeSubsystem.h"
#include "GameFramework/Actor.h"
#include "Loot/LootBagComponent.h"
#include "Engine/World.h"

UAutoWeaponComponent::UAutoWeaponComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
}

void UAutoWeaponComponent::BeginPlay()
{
	Super::BeginPlay();
	OwnerBag = GetOwner()->FindComponentByClass<ULootBagComponent>();
	OwnerHealth = GetOwner()->FindComponentByClass<UHealthComponent>();
}

void UAutoWeaponComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	CooldownRemaining = FMath::Max(CooldownRemaining - DeltaTime, 0.0f);
	if (!bEnabled || CooldownRemaining > 0.0f || (OwnerHealth && OwnerHealth->IsDead()))
	{
		return;
	}

	const URunOutcomeSubsystem* Run = GetWorld()->GetSubsystem<URunOutcomeSubsystem>();
	if (!Run || !Run->IsRunActive())
	{
		return;
	}

	const FVector Origin = GetOwner()->GetActorLocation();
	AEnemy* Target = GetWorld()->GetSubsystem<UEnemyManagerSubsystem>()->FindNearestEnemy(Origin, Tuning.Range);
	if (!Target)
	{
		return;
	}

	// Draw before damaging: the hit may destroy the target.
	DrawDebugLine(GetWorld(), Origin, Target->GetActorLocation(), FColor::Yellow, false, Tuning.HitLineSeconds, 0, Tuning.HitLineThickness);
	const float DamageMultiplier = OwnerBag ? OwnerBag->GetActiveModifier().DamageMultiplier : 1.0f;
	Target->ApplyDamage(Tuning.Damage * DamageMultiplier);
	CooldownRemaining = Tuning.Interval;
}
