#include "Combat/AutoWeaponComponent.h"
#include "Camp/CampMetaSubsystem.h"
#include "Combat/HealthComponent.h"
#include "Audio/GameAudioSubsystem.h"
#include "Combat/GunBullet.h"
#include "Combat/WeaponEffect.h"
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

	const float DamageMultiplier = Progression->GetDamageMultiplier();

	for (uint8 W = 0; W < static_cast<uint8>(EWeaponType::Count); ++W)
	{
		const EWeaponType Type = static_cast<EWeaponType>(W);
		const int32 Level = Progression->GetWeaponLevel(Type);
		if (Level <= 0)
		{
			continue;
		}

		// Stars (brief v2) multiply damage and widen the weapon's reach, so a star-up is visible on screen.
		// The Reach passive widens every weapon on top of that.
		FWeaponRow Row = Progression->GetWeaponRow(Type);
		const float Reach = Progression->GetWeaponStarRangeMultiplier(Type) * Progression->GetRangeMultiplier();
		Row.Range *= Reach;
		Row.LineHalfWidth *= Reach;
		Row.ChainRange *= Reach;
		const float WeaponDamage = DamageMultiplier * Progression->GetWeaponStarDamageMultiplier(Type);
		CurrentReach = Reach;

		if (Type == EWeaponType::Orbit)
		{
			TickOrbit(DeltaTime, Level, Row, WeaponDamage);
			continue;
		}

		float& Cooldown = Cooldowns.FindOrAdd(Type);
		Cooldown = FMath::Max(Cooldown - DeltaTime, 0.0f);
		if (Cooldown > 0.0f)
		{
			continue;
		}

		const int32 Hits = FireWeapon(Type, Level, Row, WeaponDamage);
		OnWeaponFired.Broadcast(Type, Hits);
		if (Hits > 0)
		{
			UGameAudioSubsystem::PlayIn(this, EGameSound::Attack);
		}
		// Haste shortens every weapon's interval.
		Cooldown = Row.Interval / FMath::Max(Progression->GetAttackSpeedMultiplier(), 0.1f);
	}
}

int32 UAutoWeaponComponent::FireWeapon(EWeaponType Type, int32 Level, const FWeaponRow& Row, float DamageMultiplier)
{
	UEnemyManagerSubsystem* Manager = GetWorld()->GetSubsystem<UEnemyManagerSubsystem>();
	if (!Manager)
	{
		return 0;
	}

	// Smithy camp bonus (story 006): flat damage added to every weapon.
	float CampDamage = 0.0f;
	if (const UCampMetaSubsystem* Camp = GetWorld()->GetGameInstance()->GetSubsystem<UCampMetaSubsystem>())
	{
		CampDamage = Camp->GetWeaponDamageBonus();
	}

	const float Damage = RollCrit((Row.Damage + Row.DamagePerLevel * static_cast<float>(Level - 1) + CampDamage) * DamageMultiplier);
	const FVector Origin = GetOwner()->GetActorLocation();
	int32 Hits = 0;

	switch (Type)
	{
	case EWeaponType::Blade:
	{
		AEnemy* Target = Manager->FindNearestEnemy(Origin, Row.Range);
		if (!Target)
		{
			break;
		}
		AWeaponEffect::SpawnBeam(GetWorld(), Origin, Target->GetActorLocation(), FLinearColor(1.0f, 0.85f, 0.1f), 14.0f * CurrentReach, 0.12f);
		Target->ApplyDamage(Damage);
		Hits = 1;
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
		// Collect first, damage after: a kill removes the enemy from the manager's list mid-loop.
		TArray<AEnemy*, TInlineAllocator<32>> Targets;
		for (AEnemy* Enemy : Manager->GetEnemies())
		{
			const FVector2D To = FVector2D(Enemy->GetActorLocation()) - FVector2D(Origin);
			const float Along = FVector2D::DotProduct(To, Dir);
			if (Along >= 0.0f && Along <= Row.Range && FMath::Abs(FVector2D::DotProduct(To, Perp)) <= Row.LineHalfWidth)
			{
				Targets.Add(Enemy);
			}
		}
		for (AEnemy* Enemy : Targets)
		{
			Enemy->ApplyDamage(Damage);
			++Hits;
		}
		AWeaponEffect::SpawnBeam(GetWorld(), Origin, Origin + FVector(Dir, 0.0f) * Row.Range, FLinearColor(0.1f, 0.9f, 1.0f), 20.0f * CurrentReach, 0.12f);
		break;
	}
	case EWeaponType::Nova:
	{
		const float RangeSq = Row.Range * Row.Range;
		TArray<AEnemy*, TInlineAllocator<32>> Targets;
		for (AEnemy* Enemy : Manager->GetEnemies())
		{
			if (FVector::DistSquared2D(Enemy->GetActorLocation(), Origin) <= RangeSq)
			{
				Targets.Add(Enemy);
			}
		}
		for (AEnemy* Enemy : Targets)
		{
			Enemy->ApplyDamage(Damage);
			++Hits;
		}
		AWeaponEffect::SpawnRing(GetWorld(), Origin, Row.Range, FLinearColor(1.0f, 0.2f, 0.9f), 14.0f * CurrentReach, 0.15f);
		break;
	}
	case EWeaponType::Chain:
	{
		// Pick the whole path first (nearest enemy in reach, then the nearest not-yet-hit enemy within one
		// jump of the last), and only then deal damage: a kill removes an enemy from the manager's list.
		AEnemy* Current = Manager->FindNearestEnemy(Origin, Row.Range);
		if (!Current)
		{
			break;
		}
		TArray<AEnemy*> Path;
		Path.Add(Current);
		const float JumpSq = Row.ChainRange * Row.ChainRange;
		for (int32 Jump = 0; Jump < Row.ChainJumps; ++Jump)
		{
			AEnemy* Next = nullptr;
			float BestSq = JumpSq;
			for (AEnemy* Candidate : Manager->GetEnemies())
			{
				if (!IsValid(Candidate) || Path.Contains(Candidate))
				{
					continue;
				}
				const float DistSq = FVector::DistSquared2D(Candidate->GetActorLocation(), Current->GetActorLocation());
				if (DistSq <= BestSq)
				{
					BestSq = DistSq;
					Next = Candidate;
				}
			}
			if (!Next)
			{
				break;
			}
			Path.Add(Next);
			Current = Next;
		}

		FVector From = Origin;
		for (AEnemy* Enemy : Path)
		{
			const FVector To = Enemy->GetActorLocation();
			AWeaponEffect::SpawnBeam(GetWorld(), From, To, FLinearColor(0.2f, 1.0f, 0.3f), 10.0f * CurrentReach, 0.3f);
			From = To;
			Enemy->ApplyDamage(Damage);
			++Hits;
		}
		break;
	}
	case EWeaponType::Gun:
	{
		AEnemy* Target = Manager->FindNearestEnemy(Origin, Row.Range);
		if (!Target)
		{
			break;
		}
		// One bullet at level 1, one more every second level, fanned out a little.
		const int32 Bullets = FMath::Min(Row.ProjectileCount + (Level - 1) / 2, 5);
		const FVector ToTarget = (Target->GetActorLocation() - Origin).GetSafeNormal2D();
		constexpr float SpreadDegrees = 9.0f;
		for (int32 Bullet = 0; Bullet < Bullets; ++Bullet)
		{
			const float Offset = (static_cast<float>(Bullet) - static_cast<float>(Bullets - 1) * 0.5f) * SpreadDegrees;
			AGunBullet::Fire(GetWorld(), Origin, ToTarget.RotateAngleAxis(Offset, FVector::UpVector), Row.ProjectileSpeed, Row.Range, Damage, 70.0f * CurrentReach, FLinearColor(1.0f, 0.65f, 0.1f));
		}
		Hits = 1; // a shot was fired: the sound and the probe hook count it
		break;
	}
	default:
		break;
	}
	return Hits;
}

float UAutoWeaponComponent::RollCrit(float Damage) const
{
	const URunProgressionSubsystem* Progression = GetWorld()->GetSubsystem<URunProgressionSubsystem>();
	if (Progression && FMath::FRand() < Progression->GetCritChance())
	{
		return Damage * Progression->GetCritMultiplier();
	}
	return Damage;
}

void UAutoWeaponComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	for (AOrbitOrb* Orb : Orbs)
	{
		if (IsValid(Orb))
		{
			Orb->Destroy();
		}
	}
	Orbs.Reset();
	Super::EndPlay(EndPlayReason);
}

void UAutoWeaponComponent::TickOrbit(float DeltaTime, int32 Level, const FWeaponRow& Row, float DamageMultiplier)
{
	UEnemyManagerSubsystem* Manager = GetWorld()->GetSubsystem<UEnemyManagerSubsystem>();
	if (!Manager)
	{
		return;
	}

	// Two balls at level 1, one more every second level.
	const int32 Count = FMath::Min(Row.ProjectileCount + (Level - 1) / 2, 6);
	const FVector Origin = GetOwner()->GetActorLocation();
	if (Orbs.Num() != Count)
	{
		for (AOrbitOrb* Orb : Orbs)
		{
			if (IsValid(Orb))
			{
				Orb->Destroy();
			}
		}
		Orbs.Reset();
		for (int32 i = 0; i < Count; ++i)
		{
			Orbs.Add(AOrbitOrb::Make(GetWorld(), Origin, FLinearColor(0.4f, 0.95f, 1.0f), 0.7f * CurrentReach));
		}
	}

	OrbitAngle = FMath::Fmod(OrbitAngle + Row.ProjectileSpeed * DeltaTime, 360.0f);
	TArray<FVector, TInlineAllocator<8>> Centers;
	for (int32 i = 0; i < Orbs.Num(); ++i)
	{
		const float Angle = FMath::DegreesToRadians(OrbitAngle + 360.0f * static_cast<float>(i) / static_cast<float>(Count));
		const FVector Center = Origin + FVector(FMath::Cos(Angle), FMath::Sin(Angle), 0.0f) * Row.Range;
		Centers.Add(Center);
		if (IsValid(Orbs[i]))
		{
			Orbs[i]->SetActorLocation(Center);
		}
	}

	// Collect first, damage after: a kill removes the enemy from the manager's list mid-loop.
	const double Now = GetWorld()->GetTimeSeconds();
	const float OrbRadius = 65.0f * CurrentReach;
	TArray<AEnemy*, TInlineAllocator<16>> Targets;
	for (AEnemy* Enemy : Manager->GetEnemies())
	{
		const double* ReadyAt = OrbitHitReady.Find(Enemy);
		if (ReadyAt && *ReadyAt > Now)
		{
			continue;
		}
		const float Reachable = OrbRadius + Enemy->GetBodyRadius();
		for (const FVector& Center : Centers)
		{
			if (FVector::DistSquared2D(Enemy->GetActorLocation(), Center) <= Reachable * Reachable)
			{
				Targets.Add(Enemy);
				break;
			}
		}
	}
	const float CampDamage = [this]()
	{
		const UCampMetaSubsystem* Camp = GetWorld()->GetGameInstance()->GetSubsystem<UCampMetaSubsystem>();
		return Camp ? Camp->GetWeaponDamageBonus() : 0.0f;
	}();
	for (AEnemy* Enemy : Targets)
	{
		OrbitHitReady.Add(Enemy, Now + Row.Interval);
		Enemy->ApplyDamage(RollCrit((Row.Damage + Row.DamagePerLevel * static_cast<float>(Level - 1) + CampDamage) * DamageMultiplier));
	}
	if (!Targets.IsEmpty())
	{
		OnWeaponFired.Broadcast(EWeaponType::Orbit, Targets.Num());
	}
	// Forget enemies that are gone, now and then.
	if (OrbitHitReady.Num() > 300)
	{
		for (auto It = OrbitHitReady.CreateIterator(); It; ++It)
		{
			if (!It.Key().IsValid() || It.Value() < Now)
			{
				It.RemoveCurrent();
			}
		}
	}
}
