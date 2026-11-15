#include "Enemies/BossRejectsSubsystem.h"
#include "Combat/GunBullet.h"
#include "Combat/WeaponEffect.h"
#include "Enemies/Enemy.h"
#include "Extraction/RunOutcomeSubsystem.h"
#include "Game/DirectorTuningSettings.h"
#include "Player/HeroCharacter.h"
#include "Progression/RunProgressionSubsystem.h"
#include "UI/HudMessageSubsystem.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"

namespace
{
	const FLinearColor BossShotColor(1.0f, 0.35f, 0.05f);
	const FLinearColor BossWarnColor(1.0f, 0.15f, 0.05f);

	FBossAttackTuning MakeAttack(EBossAttackKind Kind, float Interval, float Damage, float Speed, float Radius = 450.0f, float WindUp = 1.0f, int32 Count = 1)
	{
		FBossAttackTuning A;
		A.Kind = Kind;
		A.Interval = Interval;
		A.Damage = Damage;
		A.Speed = Speed;
		A.Radius = Radius;
		A.WindUp = WindUp;
		A.Count = Count;
		return A;
	}

	FString CutName(const FOfferOption& Cut)
	{
		return Cut.bIsWeapon ? StaticEnum<EWeaponType>()->GetNameStringByValue(static_cast<int64>(Cut.Weapon))
		                     : StaticEnum<EPassiveType>()->GetNameStringByValue(static_cast<int64>(Cut.Passive));
	}
}

FBossRejectTuning::FBossRejectTuning()
{
	Attacks.Add(EWeaponType::Blade, MakeAttack(EBossAttackKind::Shot, 3.0f, 12.0f, 800.0f));
	Attacks.Add(EWeaponType::Bolt, MakeAttack(EBossAttackKind::Shot, 4.0f, 18.0f, 1500.0f));
	Attacks.Add(EWeaponType::Chain, MakeAttack(EBossAttackKind::Shot, 1.8f, 7.0f, 700.0f));
	Attacks.Add(EWeaponType::Gun, MakeAttack(EBossAttackKind::Shot, 1.3f, 9.0f, 1000.0f));
	Attacks.Add(EWeaponType::Nova, MakeAttack(EBossAttackKind::Slam, 5.0f, 25.0f, 0.0f, 450.0f, 1.0f));
	Attacks.Add(EWeaponType::Orbit, MakeAttack(EBossAttackKind::Ring, 6.0f, 8.0f, 450.0f, 0.0f, 1.0f, 8));
}

void UBossRejectsSubsystem::SetBoss(AEnemy* InBoss)
{
	ClearAttacks();
	Boss = InBoss;
	bHadBoss = InBoss != nullptr;
	Processed = 0;
	SwiftnessCuts = MightCuts = HasteCuts = 0;
	HealthGiven = 0.0f;
	Copies.Reset();
	NextUse.Reset();
	if (InBoss)
	{
		BaseMoveSpeed = InBoss->Tuning.MoveSpeed;
		BaseContactDamage = InBoss->Tuning.ContactDamage;
	}
}

void UBossRejectsSubsystem::Deinitialize()
{
	ClearAttacks();
	Super::Deinitialize();
}

void UBossRejectsSubsystem::ClearAttacks()
{
	for (const FShot& Shot : Shots)
	{
		if (AActor* Visual = Shot.Visual.Get())
		{
			Visual->Destroy();
		}
	}
	Shots.Reset();
	Slams.Reset();
}

void UBossRejectsSubsystem::ApplyCut(const FOfferOption& Cut)
{
	const FBossRejectTuning& Tuning = GetDefault<UDirectorTuningSettings>()->BossRejects;
	FString Gain;

	if (Cut.bIsWeapon && Tuning.Attacks.Contains(Cut.Weapon))
	{
		int32& Count = Copies.FindOrAdd(Cut.Weapon);
		++Count;
		NextUse.FindOrAdd(Cut.Weapon) = GetWorld()->GetTimeSeconds() + Tuning.Attacks[Cut.Weapon].Interval;
		Gain = Count == 1 ? TEXT("gets a new attack") : TEXT("gets a stronger attack");
	}
	else if (!Cut.bIsWeapon && Cut.Passive == EPassiveType::Swiftness)
	{
		++SwiftnessCuts;
		Gain = TEXT("moves faster");
	}
	else if (!Cut.bIsWeapon && Cut.Passive == EPassiveType::Might)
	{
		++MightCuts;
		Gain = TEXT("hits harder");
	}
	else if (!Cut.bIsWeapon && Cut.Passive == EPassiveType::Haste)
	{
		++HasteCuts;
		Gain = TEXT("attacks faster");
	}
	else
	{
		const float Amount = Cut.bIsWeapon || Cut.Passive != EPassiveType::Vitality ? Tuning.GenericHealthPerCut : Tuning.HealthPerVitalityCut;
		const float Room = FMath::Max(Tuning.MaxBonusHealth - HealthGiven, 0.0f);
		const float Given = FMath::Min(Amount, Room);
		HealthGiven += Given;
		if (AEnemy* B = Boss.Get())
		{
			B->AddHealth(Given);
		}
		Gain = TEXT("has more health");
	}

	RefreshBossStats();
	UE_LOG(LogEnemies, Display, TEXT("BOSS gains %s: %s"), *CutName(Cut), *Gain);
	UHudMessageSubsystem::Post(this, FString::Printf(TEXT("THE BOSS TAKES %s - it %s"), *CutName(Cut).ToUpper(), *Gain), BossShotColor, 3.0f);
}

void UBossRejectsSubsystem::RefreshBossStats()
{
	AEnemy* B = Boss.Get();
	if (!B)
	{
		return;
	}
	const FBossRejectTuning& Tuning = GetDefault<UDirectorTuningSettings>()->BossRejects;
	B->Tuning.MoveSpeed = BaseMoveSpeed * (1.0f + FMath::Min(SwiftnessCuts * Tuning.MoveSpeedPerCut, Tuning.MaxStatBonus));
	B->Tuning.ContactDamage = BaseContactDamage * (1.0f + FMath::Min(MightCuts * Tuning.ContactDamagePerCut, Tuning.MaxStatBonus));
}

void UBossRejectsSubsystem::SpawnShot(const FBossAttackTuning& Attack, float Damage, const FVector& Direction)
{
	AEnemy* B = Boss.Get();
	if (!B)
	{
		return;
	}
	FShot Shot;
	Shot.Visual = AOrbitOrb::Make(GetWorld(), B->GetActorLocation(), BossShotColor, 0.45f);
	Shot.Direction = Direction.GetSafeNormal2D();
	Shot.Speed = Attack.Speed;
	Shot.DistanceLeft = Attack.Range;
	Shot.Damage = Damage;
	Shots.Add(Shot);
}

void UBossRejectsSubsystem::FireAttack(EWeaponType Weapon, const FBossAttackTuning& Attack, float Damage, const AHeroCharacter& Hero)
{
	const AEnemy* B = Boss.Get();
	const FVector ToHero = (Hero.GetActorLocation() - B->GetActorLocation()).GetSafeNormal2D();
	switch (Attack.Kind)
	{
	case EBossAttackKind::Shot:
		SpawnShot(Attack, Damage, ToHero);
		break;
	case EBossAttackKind::Ring:
		for (int32 i = 0; i < Attack.Count; ++i)
		{
			SpawnShot(Attack, Damage, ToHero.RotateAngleAxis(360.0f * i / Attack.Count, FVector::UpVector));
		}
		break;
	case EBossAttackKind::Slam:
	{
		FSlam Slam;
		Slam.Center = Hero.GetActorLocation();
		Slam.Radius = Attack.Radius;
		Slam.Damage = Damage;
		Slam.TimeLeft = Attack.WindUp;
		Slams.Add(Slam);
		// The warning: a red ring on the floor for the whole wind-up. Step out of it before it ends.
		AWeaponEffect::SpawnRing(GetWorld(), Slam.Center, Slam.Radius, BossWarnColor, 20.0f, Attack.WindUp);
		break;
	}
	}
}

void UBossRejectsSubsystem::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	if (!bHadBoss)
	{
		return;
	}

	// The boss died (or was destroyed): it stays dead, its attacks disappear and later cuts do nothing.
	if (!Boss.IsValid())
	{
		ClearAttacks();
		bHadBoss = false;
		return;
	}

	const URunProgressionSubsystem* Progression = GetWorld()->GetSubsystem<URunProgressionSubsystem>();
	const URunOutcomeSubsystem* Run = GetWorld()->GetSubsystem<URunOutcomeSubsystem>();
	AHeroCharacter* Hero = Cast<AHeroCharacter>(UGameplayStatics::GetPlayerPawn(this, 0));
	if (!Progression || !Run || !Hero || !Run->IsRunActive())
	{
		return;
	}

	// New cuts first, so the player's choice has an effect the moment it is made. The offer pauses everything else.
	const TArray<FOfferOption>& Floor = Progression->GetCuttingRoomFloor();
	while (Processed < Floor.Num())
	{
		ApplyCut(Floor[Processed++]);
	}
	if (Progression->IsOfferPending() || Hero->IsDead())
	{
		return;
	}

	const FBossRejectTuning& Tuning = GetDefault<UDirectorTuningSettings>()->BossRejects;
	FireDueAttacks(Tuning, *Hero);
	MoveShots(Tuning, *Hero, DeltaTime);
	CountDownSlams(*Hero, DeltaTime);
}

void UBossRejectsSubsystem::FireDueAttacks(const FBossRejectTuning& Tuning, AHeroCharacter& Hero)
{
	const double Now = GetWorld()->GetTimeSeconds();
	const float HasteFactor = FMath::Max(FMath::Pow(Tuning.AttackIntervalPerHaste, static_cast<float>(HasteCuts)), 0.5f);

	for (const TPair<EWeaponType, int32>& Pair : Copies)
	{
		const FBossAttackTuning* Attack = Tuning.Attacks.Find(Pair.Key);
		double& Next = NextUse.FindOrAdd(Pair.Key);
		if (Attack && Now >= Next)
		{
			const float Damage = Attack->Damage * (1.0f + Tuning.RepeatDamageStep * (Pair.Value - 1));
			FireAttack(Pair.Key, *Attack, Damage, Hero);
			Next = Now + Attack->Interval * HasteFactor;
		}
	}
}

void UBossRejectsSubsystem::MoveShots(const FBossRejectTuning& Tuning, AHeroCharacter& Hero, float DeltaTime)
{
	const FVector HeroLocation = Hero.GetActorLocation();
	for (int32 i = Shots.Num() - 1; i >= 0; --i)
	{
		FShot& Shot = Shots[i];
		AActor* Visual = Shot.Visual.Get();
		bool bDone = !Visual;
		if (Visual)
		{
			const float Step = Shot.Speed * DeltaTime;
			Visual->SetActorLocation(Visual->GetActorLocation() + Shot.Direction * Step);
			Shot.DistanceLeft -= Step;
			if (FVector::Dist2D(Visual->GetActorLocation(), HeroLocation) < Tuning.ShotHitRadius)
			{
				Hero.ReceiveEnemyDamage(Shot.Damage);
				UE_LOG(LogEnemies, Display, TEXT("BOSS ball hit the hero for %.0f"), Shot.Damage);
				bDone = true;
			}
			bDone = bDone || Shot.DistanceLeft <= 0.0f;
		}
		if (bDone)
		{
			if (Visual)
			{
				Visual->Destroy();
			}
			Shots.RemoveAtSwap(i);
		}
	}
}

void UBossRejectsSubsystem::CountDownSlams(AHeroCharacter& Hero, float DeltaTime)
{
	const FVector HeroLocation = Hero.GetActorLocation();
	for (int32 i = Slams.Num() - 1; i >= 0; --i)
	{
		FSlam& Slam = Slams[i];
		Slam.TimeLeft -= DeltaTime;
		if (Slam.TimeLeft <= 0.0f)
		{
			if (FVector::Dist2D(Slam.Center, HeroLocation) < Slam.Radius)
			{
				Hero.ReceiveEnemyDamage(Slam.Damage);
				UE_LOG(LogEnemies, Display, TEXT("BOSS slam hit the hero for %.0f"), Slam.Damage);
			}
			AWeaponEffect::SpawnRing(GetWorld(), Slam.Center, Slam.Radius, FLinearColor::White, 40.0f, 0.25f);
			Slams.RemoveAtSwap(i);
		}
	}
}
