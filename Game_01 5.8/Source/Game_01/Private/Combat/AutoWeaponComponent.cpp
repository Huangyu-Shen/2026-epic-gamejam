#include "Combat/AutoWeaponComponent.h"
#include "Camp/CampMetaSubsystem.h"
#include "Combat/HealthComponent.h"
#include "DrawDebugHelpers.h"
#include "Enemies/Enemy.h"
#include "Enemies/EnemyManagerSubsystem.h"
#include "Extraction/RunOutcomeSubsystem.h"
#include "GameFramework/Actor.h"
#include "Loot/LootBagComponent.h"
#include "Progression/RunProgressionSubsystem.h"
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

	URunProgressionSubsystem* Progression = GetWorld()->GetSubsystem<URunProgressionSubsystem>();
	if (!bEnabled || !Progression || Progression->IsOfferPending() || (OwnerHealth && OwnerHealth->IsDead()))
	{
		return;
	}

	const URunOutcomeSubsystem* Run = GetWorld()->GetSubsystem<URunOutcomeSubsystem>();
	if (!Run || !Run->IsRunActive())
	{
		return;
	}

	const float DamageMultiplier = (OwnerBag ? OwnerBag->GetActiveModifier().DamageMultiplier : 1.0f) * Progression->GetDamageMultiplier();

	for (uint8 W = 0; W < static_cast<uint8>(EWeaponType::Count); ++W)
	{
		const EWeaponType Type = static_cast<EWeaponType>(W);
		const int32 Level = Progression->GetWeaponLevel(Type);
		if (Level <= 0)
		{
			continue;
		}

		float& Cooldown = Cooldowns.FindOrAdd(Type);
		Cooldown = FMath::Max(Cooldown - DeltaTime, 0.0f);
		if (Cooldown > 0.0f)
		{
			continue;
		}

		// Stars (brief v2) multiply damage and widen the weapon's reach, so a star-up is visible on screen.
		FWeaponRow Row = Progression->GetWeaponRow(Type);
		const float Reach = Progression->GetWeaponStarRangeMultiplier(Type);
		Row.Range *= Reach;
		Row.LineHalfWidth *= Reach;
		FireWeapon(Type, Level, Row, DamageMultiplier * Progression->GetWeaponStarDamageMultiplier(Type));
		Cooldown = Row.Interval;
	}
}

void UAutoWeaponComponent::FireWeapon(EWeaponType Type, int32 Level, const FWeaponRow& Row, float DamageMultiplier)
{
	UEnemyManagerSubsystem* Manager = GetWorld()->GetSubsystem<UEnemyManagerSubsystem>();
	if (!Manager)
	{
		return;
	}

	// Smithy camp bonus (story 006): flat damage added to every weapon.
	float CampDamage = 0.0f;
	if (const UCampMetaSubsystem* Camp = GetWorld()->GetGameInstance()->GetSubsystem<UCampMetaSubsystem>())
	{
		CampDamage = Camp->GetWeaponDamageBonus();
	}

	const float Damage = (Row.Damage + Row.DamagePerLevel * static_cast<float>(Level - 1) + CampDamage) * DamageMultiplier;
	const FVector Origin = GetOwner()->GetActorLocation();

	switch (Type)
	{
	case EWeaponType::Blade:
	{
		AEnemy* Target = Manager->FindNearestEnemy(Origin, Row.Range);
		if (!Target)
		{
			break;
		}
		DrawDebugLine(GetWorld(), Origin, Target->GetActorLocation(), FColor::Yellow, false, 0.12f, 0, 6.0f);
		Target->ApplyDamage(Damage);
		break;
	}
	case EWeaponType::Bolt:
	{
		FVector2D Dir = FVector2D(GetOwner()->GetActorForwardVector()).GetSafeNormal();
		if (AEnemy* Nearest = Manager->FindNearestEnemy(Origin, Row.Range))
		{
			Dir = (FVector2D(Nearest->GetActorLocation()) - FVector2D(Origin)).GetSafeNormal();
		}
		const FVector2D Perp(-Dir.Y, Dir.X);
		for (AEnemy* Enemy : Manager->GetEnemies())
		{
			const FVector2D To = FVector2D(Enemy->GetActorLocation()) - FVector2D(Origin);
			const float Along = FVector2D::DotProduct(To, Dir);
			if (Along < 0.0f || Along > Row.Range)
			{
				continue;
			}
			if (FMath::Abs(FVector2D::DotProduct(To, Perp)) <= Row.LineHalfWidth)
			{
				Enemy->ApplyDamage(Damage);
			}
		}
		DrawDebugLine(GetWorld(), Origin, Origin + FVector(Dir, 0.0f) * Row.Range, FColor::Cyan, false, 0.12f, 0, 4.0f);
		break;
	}
	case EWeaponType::Nova:
	{
		const float RangeSq = Row.Range * Row.Range;
		for (AEnemy* Enemy : Manager->GetEnemies())
		{
			if (FVector::DistSquared2D(Enemy->GetActorLocation(), Origin) <= RangeSq)
			{
				Enemy->ApplyDamage(Damage);
			}
		}
		DrawDebugCircle(GetWorld(), Origin, Row.Range, 32, FColor::Magenta, false, 0.12f, 0, 2.0f, FVector(1.0f, 0.0f, 0.0f), FVector(0.0f, 1.0f, 0.0f), false);
		break;
	}
	default:
		break;
	}
}
