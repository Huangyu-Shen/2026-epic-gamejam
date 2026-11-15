#include "Game/DirectorGameMode.h"
#include "Audio/GameAudioSubsystem.h"
#include "Extraction/ExtractionZone.h"
#include "EngineUtils.h"
#include "Camp/CampMetaSubsystem.h"
#include "Combat/AutoWeaponComponent.h"
#include "Combat/HealthComponent.h"
#include "Extraction/CampStockSubsystem.h"
#include "Extraction/RunOutcomeSubsystem.h"
#include "Loot/LootBagComponent.h"
#include "Loot/LootConfig.h"
#include "Loot/LootHeatSubsystem.h"
#include "Enemies/EnemyManagerSubsystem.h"
#include "Enemies/EnemySpawnSubsystem.h"
#include "Enemies/EnemyTypes.h"
#include "Player/HeroCharacter.h"
#include "Progression/RunProgressionSubsystem.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"
#include "Misc/App.h"
#include "Misc/CommandLine.h"
#include "Misc/Parse.h"
#include "TimerManager.h"
#include "UnrealClient.h"

ADirectorGameMode::ADirectorGameMode()
{
	DefaultPawnClass = AHeroCharacter::StaticClass();

	// Ticks only while the probe samples.
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.bStartWithTickEnabled = false;
}

void ADirectorGameMode::BeginPlay()
{
	Super::BeginPlay();

	if (UGameAudioSubsystem* Audio = GetWorld()->GetSubsystem<UGameAudioSubsystem>())
	{
		Audio->StartMusic(EMusicTrack::Run);
	}

	if (FParse::Param(FCommandLine::Get(), TEXT("WeaponShowcase")))
	{
		// Story 011 check: own the new weapons, bring a crowd, photograph them in action.
		FTimerHandle Handle;
		GetWorldTimerManager().SetTimer(Handle, FTimerDelegate::CreateWeakLambda(this, [this]()
		{
			AHeroCharacter* Hero = Cast<AHeroCharacter>(UGameplayStatics::GetPlayerPawn(this, 0));
			URunProgressionSubsystem* Progression = GetWorld()->GetSubsystem<URunProgressionSubsystem>();
			if (!Hero || !Progression)
			{
				FPlatformMisc::RequestExit(false);
				return;
			}
			Hero->Health->bGodMode = true;
			Progression->DebugGrantWeapon(EWeaponType::Gun, 3);
			Progression->DebugGrantWeapon(EWeaponType::Orbit, 3);
			GetWorld()->GetSubsystem<UEnemySpawnSubsystem>()->SpawnHorde(30, true);
			UE_LOG(LogEnemies, Display, TEXT("SHOWCASE loadout=[%s]"), *Progression->GetLoadoutSummary());
			FTimerHandle S1, S2, Quit;
			GetWorldTimerManager().SetTimer(S1, FTimerDelegate::CreateWeakLambda(this, [this]()
			{
				FScreenshotRequest::RequestScreenshot(FPaths::ProjectSavedDir() / TEXT("Showcase") / TEXT("1-gun-and-orbit.png"), true, false);
			}), 2.5f, false);
			GetWorldTimerManager().SetTimer(S2, FTimerDelegate::CreateWeakLambda(this, [this]()
			{
				FScreenshotRequest::RequestScreenshot(FPaths::ProjectSavedDir() / TEXT("Showcase") / TEXT("2-gun-and-orbit.png"), true, false);
			}), 3.3f, false);
			GetWorldTimerManager().SetTimer(Quit, FTimerDelegate::CreateLambda([]() { FPlatformMisc::RequestExit(false); }), 5.0f, false);
		}), 3.0f, false);
	}

	if (FParse::Param(FCommandLine::Get(), TEXT("EdgeProbe")))
	{
		// Void fix check: push the hero past the floor edge, spawn a horde there, and see where everyone ends up.
		FTimerHandle Handle;
		GetWorldTimerManager().SetTimer(Handle, FTimerDelegate::CreateWeakLambda(this, [this]()
		{
			AHeroCharacter* Hero = Cast<AHeroCharacter>(UGameplayStatics::GetPlayerPawn(this, 0));
			UEnemySpawnSubsystem* Spawner = GetWorld()->GetSubsystem<UEnemySpawnSubsystem>();
			if (!Hero || !Spawner)
			{
				FPlatformMisc::RequestExit(false);
				return;
			}
			Hero->Health->bGodMode = true;
			Hero->SetActorLocation(FVector(6000.0f, 3900.0f, Hero->GetActorLocation().Z), false, nullptr, ETeleportType::TeleportPhysics);
			Spawner->SpawnHorde(40, true);
			Spawner->SpawnBoss();
			FTimerHandle Report, Shot, Quit;
			GetWorldTimerManager().SetTimer(Report, FTimerDelegate::CreateWeakLambda(this, [this]()
			{
				const AHeroCharacter* H = Cast<AHeroCharacter>(UGameplayStatics::GetPlayerPawn(this, 0));
				float MaxAbs = 0.0f;
				for (TActorIterator<AEnemy> It(GetWorld()); It; ++It)
				{
					MaxAbs = FMath::Max(MaxAbs, FMath::Max(FMath::Abs(It->GetActorLocation().X), FMath::Abs(It->GetActorLocation().Y)));
				}
				UE_LOG(LogEnemies, Display, TEXT("EDGEPROBE hero=%s farthest enemy=%.0f"), *H->GetActorLocation().ToString(), MaxAbs);
			}), 2.0f, false);
			GetWorldTimerManager().SetTimer(Shot, FTimerDelegate::CreateWeakLambda(this, [this]()
			{
				FScreenshotRequest::RequestScreenshot(FPaths::ProjectSavedDir() / TEXT("EdgeProbe") / TEXT("hero-at-edge.png"), true, false);
			}), 2.2f, false);
			GetWorldTimerManager().SetTimer(Quit, FTimerDelegate::CreateLambda([]() { FPlatformMisc::RequestExit(false); }), 3.5f, false);
		}), 3.0f, false);
	}

	if (FParse::Param(FCommandLine::Get(), TEXT("BossProbe")))
	{
		// Story 012 check: a boss with no cuts, then cuts one by one; photograph its attacks.
		FTimerHandle Handle;
		GetWorldTimerManager().SetTimer(Handle, FTimerDelegate::CreateWeakLambda(this, [this]()
		{
			AHeroCharacter* Hero = Cast<AHeroCharacter>(UGameplayStatics::GetPlayerPawn(this, 0));
			URunProgressionSubsystem* Progression = GetWorld()->GetSubsystem<URunProgressionSubsystem>();
			UEnemySpawnSubsystem* Spawner = GetWorld()->GetSubsystem<UEnemySpawnSubsystem>();
			if (!Hero || !Progression || !Spawner)
			{
				FPlatformMisc::RequestExit(false);
				return;
			}
			Hero->Health->bGodMode = true;
			AEnemy* Boss = Spawner->SpawnBoss();
			if (Boss)
			{
				Boss->SetActorLocation(Hero->GetActorLocation() + FVector(900.0f, 0.0f, 0.0f));
			}
			UE_LOG(LogEnemies, Display, TEXT("BOSSPROBE boss=%d"), Boss != nullptr);

			auto Cut = [Progression](bool bWeapon, EWeaponType W, EPassiveType P)
			{
				FOfferOption Option;
				Option.bIsWeapon = bWeapon;
				Option.Weapon = W;
				Option.Passive = P;
				Progression->DebugCut(Option);
			};
			FTimerHandle C1, C2, C3, S1, S2, S3, Quit;
			GetWorldTimerManager().SetTimer(C1, FTimerDelegate::CreateWeakLambda(this, [Cut]() { Cut(true, EWeaponType::Gun, EPassiveType::Might); }), 1.0f, false);
			GetWorldTimerManager().SetTimer(C2, FTimerDelegate::CreateWeakLambda(this, [Cut]() { Cut(true, EWeaponType::Nova, EPassiveType::Might); }), 2.0f, false);
			GetWorldTimerManager().SetTimer(C3, FTimerDelegate::CreateWeakLambda(this, [Cut]() { Cut(false, EWeaponType::Blade, EPassiveType::Greed); Cut(true, EWeaponType::Orbit, EPassiveType::Might); }), 3.0f, false);
			GetWorldTimerManager().SetTimer(S1, FTimerDelegate::CreateWeakLambda(this, [this]()
			{
				FScreenshotRequest::RequestScreenshot(FPaths::ProjectSavedDir() / TEXT("BossProbe") / TEXT("1-gun-balls.png"), true, false);
			}), 3.9f, false);
			GetWorldTimerManager().SetTimer(S2, FTimerDelegate::CreateWeakLambda(this, [this]()
			{
				FScreenshotRequest::RequestScreenshot(FPaths::ProjectSavedDir() / TEXT("BossProbe") / TEXT("2-slam-warning.png"), true, false);
			}), 7.4f, false);
			GetWorldTimerManager().SetTimer(S3, FTimerDelegate::CreateWeakLambda(this, [this]()
			{
				FScreenshotRequest::RequestScreenshot(FPaths::ProjectSavedDir() / TEXT("BossProbe") / TEXT("3-ring-of-balls.png"), true, false);
			}), 9.5f, false);
			GetWorldTimerManager().SetTimer(Quit, FTimerDelegate::CreateLambda([]() { FPlatformMisc::RequestExit(false); }), 11.0f, false);
		}), 3.0f, false);
	}

	if (FParse::Param(FCommandLine::Get(), TEXT("ZoneShot")))
	{
		// Story 011 check: photograph the extraction point from nearby, then while the bar is filling.
		FTimerHandle Handle;
		GetWorldTimerManager().SetTimer(Handle, FTimerDelegate::CreateWeakLambda(this, [this]()
		{
			AHeroCharacter* Hero = Cast<AHeroCharacter>(UGameplayStatics::GetPlayerPawn(this, 0));
			AExtractionZone* Zone = nullptr;
			int32 ZoneCount = 0;
			for (TActorIterator<AExtractionZone> It(GetWorld()); It; ++It)
			{
				Zone = *It;
				++ZoneCount;
			}
			UE_LOG(LogEnemies, Display, TEXT("ZONESHOT zones in level: %d"), ZoneCount);
			if (!Hero || !Zone)
			{
				UE_LOG(LogEnemies, Error, TEXT("ZONESHOT: no hero or no extraction zone"));
				FPlatformMisc::RequestExit(false);
				return;
			}
			Hero->Health->bGodMode = true;
			Hero->Weapon->bEnabled = false;
			const FVector Zone3 = Zone->GetActorLocation();
			Hero->SetActorLocation(Zone3 + FVector(-900.0f, 0.0f, 100.0f));
			FTimerHandle Step1, Step2, Step3;
			GetWorldTimerManager().SetTimer(Step1, FTimerDelegate::CreateWeakLambda(this, [this]()
			{
				FScreenshotRequest::RequestScreenshot(FPaths::ProjectSavedDir() / TEXT("ZoneShot") / TEXT("1-zone-from-nearby.png"), true, false);
			}), 1.0f, false);
			GetWorldTimerManager().SetTimer(Step2, FTimerDelegate::CreateWeakLambda(this, [this, Hero, Zone3]()
			{
				Hero->SetActorLocation(Zone3 + FVector(0.0f, 0.0f, 100.0f));
			}), 2.0f, false);
			GetWorldTimerManager().SetTimer(Step3, FTimerDelegate::CreateWeakLambda(this, [this, Zone]()
			{
				UE_LOG(LogEnemies, Display, TEXT("ZONESHOT progress=%.2f"), Zone->GetProgress());
				FScreenshotRequest::RequestScreenshot(FPaths::ProjectSavedDir() / TEXT("ZoneShot") / TEXT("2-zone-filling.png"), true, false);
			}), 3.5f, false);
			FTimerHandle Quit;
			GetWorldTimerManager().SetTimer(Quit, FTimerDelegate::CreateLambda([]() { FPlatformMisc::RequestExit(false); }), 5.0f, false);
		}), 3.0f, false);
	}

	if (FParse::Param(FCommandLine::Get(), TEXT("PerfProbe")))
	{
		GetWorldTimerManager().SetTimer(ProbeStartHandle, this, &ADirectorGameMode::StartPerfProbe, PerfProbe.StartDelaySeconds);
	}

	if (FParse::Param(FCommandLine::Get(), TEXT("OfferProbe")))
	{
		GetWorldTimerManager().SetTimer(OfferProbeHandle, this, &ADirectorGameMode::StepOfferProbe, OfferProbe.StepSeconds, true, OfferProbe.StartDelaySeconds);
	}

	if (FParse::Param(FCommandLine::Get(), TEXT("PaceProbe")))
	{
		bPaceProbe = true;
		SetActorTickEnabled(true);
		GetWorldTimerManager().SetTimer(PaceHandle, this, &ADirectorGameMode::StepPaceProbe, 1.0f, true, 2.0f);
	}

	FString ReelProbeMode;
	if (FParse::Param(FCommandLine::Get(), TEXT("ReelProbe")) || FParse::Value(FCommandLine::Get(), TEXT("ReelProbe="), ReelProbeMode))
	{
		bReelProbeDies = ReelProbeMode.Equals(TEXT("die"), ESearchCase::IgnoreCase);
		GetWorldTimerManager().SetTimer(ReelProbeHandle, this, &ADirectorGameMode::StepReelProbe, OfferProbe.StepSeconds, true, OfferProbe.StartDelaySeconds);
	}
}

void ADirectorGameMode::LogTuning(const AHeroCharacter& Hero) const
{
	const UWorld* World = GetWorld();
	const UEnemySpawnSubsystem* Spawner = World->GetSubsystem<UEnemySpawnSubsystem>();
	const URunProgressionSubsystem* Progression = World->GetSubsystem<URunProgressionSubsystem>();
	const UCampMetaSubsystem* Camp = World->GetGameInstance()->GetSubsystem<UCampMetaSubsystem>();
	UE_LOG(LogEnemies, Display, TEXT("TUNING hero maxHealth=%.0f campDamage=+%.0f campHealth=+%.0f xpToLevel2=%.0f"),
		Hero.Health->GetMaxHealth(), Camp ? Camp->GetWeaponDamageBonus() : 0.0f, Camp ? Camp->GetMaxHealthBonus() : 0.0f, Progression->GetXpToNext());
	UE_LOG(LogEnemies, Display, TEXT("TUNING grunt health=%.0f speed=%.0f contact=%.0f/%.1fs range=%.0f  boss health=%.0f speed=%.0f contact=%.0f"),
		Spawner->GruntTuning.MaxHealth, Spawner->GruntTuning.MoveSpeed, Spawner->GruntTuning.ContactDamage, Spawner->GruntTuning.ContactCooldown,
		Spawner->GruntTuning.ContactRange, Spawner->BossTuning.MaxHealth, Spawner->BossTuning.MoveSpeed, Spawner->BossTuning.ContactDamage);
	UE_LOG(LogEnemies, Display, TEXT("TUNING spawn base=%.2f/s ramp=+%.2f/s per minute maxAlive=%d ring=%.0f-%.0f"),
		Spawner->Tuning.BaseRate, Spawner->Tuning.RampPerMinute, Spawner->Tuning.MaxAlive, Spawner->Tuning.RingMinRadius, Spawner->Tuning.RingMaxRadius);
	for (uint8 W = 0; W < static_cast<uint8>(EWeaponType::Count); ++W)
	{
		const FWeaponRow& Row = Progression->GetWeaponRow(static_cast<EWeaponType>(W));
		UE_LOG(LogEnemies, Display, TEXT("TUNING weapon %d damage=%.0f+%.0f/level interval=%.2fs range=%.0f locked=%d"), W,
			Row.Damage, Row.DamagePerLevel, Row.Interval, Row.Range, Row.bStartsLocked ? 1 : 0);
	}
}

void ADirectorGameMode::StepPaceProbe()
{
	AHeroCharacter* Hero = Cast<AHeroCharacter>(UGameplayStatics::GetPlayerPawn(this, 0));
	URunProgressionSubsystem* Progression = GetWorld()->GetSubsystem<URunProgressionSubsystem>();
	URunOutcomeSubsystem* Outcome = GetWorld()->GetSubsystem<URunOutcomeSubsystem>();
	UEnemySpawnSubsystem* Spawner = GetWorld()->GetSubsystem<UEnemySpawnSubsystem>();
	if (!Hero || !Progression || !Outcome || !Spawner)
	{
		UE_LOG(LogEnemies, Error, TEXT("PACEPROBE aborted: missing hero or subsystem."));
		FPlatformMisc::RequestExit(false);
		return;
	}

	if (PaceSeconds == 0.0f)
	{
		float Speed = 4.0f;
		FParse::Value(FCommandLine::Get(), TEXT("PaceSpeed="), Speed);
		UGameplayStatics::SetGlobalTimeDilation(this, FMath::Clamp(Speed, 1.0f, 8.0f));

		// Probe-only overrides, to search for numbers before they go into the data tables.
		float Value = 0.0f;
		if (FParse::Value(FCommandLine::Get(), TEXT("PaceSpawnBase="), Value)) { Spawner->Tuning.BaseRate = Value; }
		if (FParse::Value(FCommandLine::Get(), TEXT("PaceSpawnRamp="), Value)) { Spawner->Tuning.RampPerMinute = Value; }
		if (FParse::Value(FCommandLine::Get(), TEXT("PaceGruntHealth="), Value)) { Spawner->GruntTuning.MaxHealth = Value; }
		if (FParse::Value(FCommandLine::Get(), TEXT("PaceGruntContact="), Value)) { Spawner->GruntTuning.ContactDamage = Value; }
		if (FParse::Value(FCommandLine::Get(), TEXT("PaceHeroHealth="), Value)) { Hero->Health->SetMaxHealth(Value); }
		LogTuning(*Hero);
	}
	PaceSeconds += 1.0f;

	// The bot always takes the first option and never cuts.
	if (Progression->IsOfferPending())
	{
		Progression->ChooseOption(0);
	}

	const int32 Seconds = FMath::RoundToInt(PaceSeconds);
	const bool bEnded = !Outcome->IsRunActive();
	if (Seconds % 30 == 0 || bEnded || Seconds >= 600)
	{
		UE_LOG(LogEnemies, Display, TEXT("PACEPROBE t=%ds level=%d hp=%.0f/%.0f alive=%d spawnRate=%.2f/s loadout=[%s]"), Seconds, Progression->GetLevel(),
			Hero->Health->GetHealth(), Hero->Health->GetMaxHealth(), GetWorld()->GetSubsystem<UEnemyManagerSubsystem>()->GetAliveCount(),
			Spawner->GetCurrentSpawnRate(), *Progression->GetLoadoutSummary());
	}
	if (bEnded || Seconds >= 600)
	{
		UE_LOG(LogEnemies, Display, TEXT("PACEPROBE END outcome=%d (1 extracted, 2 died, 0 still running) at t=%ds level=%d"), static_cast<int32>(Outcome->GetOutcome()),
			Seconds, Progression->GetLevel());
		GetWorldTimerManager().ClearTimer(PaceHandle);
		FPlatformMisc::RequestExit(false);
	}
}

void ADirectorGameMode::StepReelProbe()
{
	AHeroCharacter* Hero = Cast<AHeroCharacter>(UGameplayStatics::GetPlayerPawn(this, 0));
	URunOutcomeSubsystem* Outcome = GetWorld()->GetSubsystem<URunOutcomeSubsystem>();
	UCampStockSubsystem* Stock = GetWorld()->GetGameInstance()->GetSubsystem<UCampStockSubsystem>();
	ULootHeatSubsystem* Heat = GetWorld()->GetSubsystem<ULootHeatSubsystem>();
	if (!Hero || !Outcome || !Stock || !Heat)
	{
		UE_LOG(LogEnemies, Error, TEXT("REELPROBE aborted: missing hero, outcome, stock or heat subsystem."));
		FPlatformMisc::RequestExit(false);
		return;
	}

	auto Shot = [](const TCHAR* Name)
	{
		FScreenshotRequest::RequestScreenshot(FPaths::ProjectSavedDir() / TEXT("ReelProbe") / Name, true, false);
	};
	auto LogState = [&](const TCHAR* Label)
	{
		FString Unlocked;
		for (const FName Option : Stock->GetUnlockedOptions())
		{
			Unlocked += Option.ToString() + TEXT(" ");
		}
		ERelicType Dummy;
		const bool bAnotherReelDrops = Hero->Bag->GetConfig()->PickRandomRelic(Stock->GetUnlockedOptions(), Hero->Bag->GetRelics(), Dummy);
		UE_LOG(LogEnemies, Display, TEXT("REELPROBE %s reelsInBag=%d heat=%.1f unlocked=[%s] anotherReelCanDrop=%d outcome=%d"), Label,
			Hero->Bag->GetRelicCount(), Heat->GetHeat(), *Unlocked, bAnotherReelDrops ? 1 : 0, static_cast<int32>(Outcome->GetOutcome()));
	};

	switch (ReelProbeStep++)
	{
	case 0:
		// A reel and some Iron in the bag; picking the reel up raises heat. Nothing else spawns, the hero cannot die by accident.
		Hero->Health->bGodMode = true;
		GetWorld()->GetSubsystem<UEnemySpawnSubsystem>()->SetSchedulerEnabled(false);
		Hero->Bag->TryAddIron(25);
		{
			// -ReelCount=N puts the first N reel types in the bag (default 1).
			int32 ReelCount = 1;
			FParse::Value(FCommandLine::Get(), TEXT("ReelCount="), ReelCount);
			const ERelicType Reels[] = { ERelicType::Swiftstep, ERelicType::Warbrand, ERelicType::Aegis };
			for (int32 i = 0; i < FMath::Clamp(ReelCount, 1, 3); ++i)
			{
				Hero->Bag->TryAddRelic(Reels[i]);
			}
		}
		LogState(TEXT("reel-in-bag"));
		break;
	case 1:
		Shot(TEXT("1-reel-in-bag.png"));
		break;
	case 2:
		if (bReelProbeDies)
		{
			Outcome->ReportPlayerDied(Hero->Bag);
		}
		else
		{
			Outcome->ReportExtracted(Hero->Bag);
		}
		LogState(bReelProbeDies ? TEXT("after-death") : TEXT("after-extraction"));
		break;
	case 3:
		Shot(bReelProbeDies ? TEXT("3-result-died.png") : TEXT("2-result-extracted.png"));
		break;
	default:
		GetWorldTimerManager().ClearTimer(ReelProbeHandle);
		FPlatformMisc::RequestExit(false);
		break;
	}
}

void ADirectorGameMode::StepOfferProbe()
{
	AHeroCharacter* Hero = Cast<AHeroCharacter>(UGameplayStatics::GetPlayerPawn(this, 0));
	URunProgressionSubsystem* Progression = GetWorld()->GetSubsystem<URunProgressionSubsystem>();
	if (!Hero || !Progression)
	{
		UE_LOG(LogEnemies, Error, TEXT("OFFERPROBE aborted: no hero pawn or progression subsystem."));
		FPlatformMisc::RequestExit(false);
		return;
	}

	auto Shot = [](const TCHAR* Name)
	{
		FScreenshotRequest::RequestScreenshot(FPaths::ProjectSavedDir() / TEXT("OfferProbe") / Name, true, false);
	};
	// Take the Chain option when it is offered (else the first), so the Chain weapon is owned by the end of the probe.
	// Before taking, cut one other option when a cut is allowed, so the cut path is exercised too.
	auto ResolveOffer = [Progression]()
	{
		auto KeepIndex = [Progression]()
		{
			const TArray<FString> Lines = Progression->GetOfferDescriptions();
			for (int32 i = 0; i < Lines.Num(); ++i)
			{
				if (Lines[i].Contains(TEXT("Chain")))
				{
					return i;
				}
			}
			return 0;
		};
		if (Progression->CanCutFromOffer())
		{
			const int32 Keep = KeepIndex();
			Progression->CutFromOffer(Keep == 0 ? 1 : 0);
		}
		Progression->ChooseOption(KeepIndex());
	};
	auto LogState = [Progression](const TCHAR* Label)
	{
		UE_LOG(LogEnemies, Display, TEXT("OFFERPROBE %s level=%d pool=%d cutsLeft=%d cutSoFar=%d attackSpeed=%.2f magnetRadius=%.0f loadout=[%s]"), Label,
			Progression->GetLevel(), Progression->GetPoolSize(), Progression->GetPermanentCutsLeft(), Progression->GetCuttingRoomFloor().Num(),
			Progression->GetAttackSpeedMultiplier(), Progression->GetMagnetRadius(), *Progression->GetLoadoutSummary());
		for (const FString& Line : Progression->GetOfferDescriptions())
		{
			UE_LOG(LogEnemies, Display, TEXT("OFFERPROBE   option: %s"), *Line);
		}
	};

	switch (OfferProbeStep++)
	{
	case 0:
		// A calm screen: the hero cannot die and nothing new spawns. Then force the first level-up.
		Hero->Health->bGodMode = true;
		GetWorld()->GetSubsystem<UEnemySpawnSubsystem>()->SetSchedulerEnabled(false);
		Progression->GrantXp(Progression->GetXpToNext());
		LogState(TEXT("first-offer"));
		break;
	case 1:
		Shot(TEXT("1-first-offer.png"));
		break;
	case 2:
		// Play several level-ups, cutting one option each time and taking another.
		for (int32 i = 0; i < OfferProbe.LevelUps; ++i)
		{
			if (Progression->IsOfferPending())
			{
				ResolveOffer();
			}
			Progression->GrantXp(Progression->GetXpToNext());
		}
		if (Progression->IsOfferPending())
		{
			ResolveOffer();
		}
		LogState(TEXT("after-level-ups"));
		break;
	case 3:
		Shot(TEXT("2-loadout-after-cuts.png"));
		break;
	case 4:
		Progression->GrantXp(Progression->GetXpToNext());
		LogState(TEXT("late-offer"));
		break;
	case 5:
		Shot(TEXT("3-late-offer.png"));
		break;
	case 6:
		// Resolve the offer, then give the Chain weapon (if owned) a crowd: screenshot its first multi-hit.
		if (Progression->IsOfferPending())
		{
			ResolveOffer();
		}
		GetWorld()->GetSubsystem<UEnemySpawnSubsystem>()->SpawnHorde(15, true);
		Hero->Weapon->OnWeaponFired.AddLambda([this, Seen = MakeShared<TSet<EWeaponType>>()](EWeaponType Type, int32 Hits)
		{
			// Story 011: the first attack of every owned weapon that hits something is photographed, to show its visual.
			if (Type != EWeaponType::Chain && Hits >= 1 && !Seen->Contains(Type))
			{
				Seen->Add(Type);
				const FString Name = FString::Printf(TEXT("fx-%s.png"), *StaticEnum<EWeaponType>()->GetNameStringByValue(static_cast<int64>(Type)));
				FScreenshotRequest::RequestScreenshot(FPaths::ProjectSavedDir() / TEXT("OfferProbe") / Name, true, false);
				UE_LOG(LogEnemies, Display, TEXT("OFFERPROBE weapon visual shot: %s"), *Name);
			}
			if (Type == EWeaponType::Chain && Hits >= 2 && !bChainShotTaken)
			{
				bChainShotTaken = true;
				// Requested in the same frame the weapon drew its lines, so they are in the image.
				FScreenshotRequest::RequestScreenshot(FPaths::ProjectSavedDir() / TEXT("OfferProbe") / TEXT("4-chain-fire.png"), true, false);
				UE_LOG(LogEnemies, Display, TEXT("OFFERPROBE chain fired: hit %d enemies in one attack"), Hits);
			}
		});
		break;
	case 7:
		// Wait (up to eight steps) for a Chain attack that hits at least two enemies.
		if (Progression->GetWeaponLevel(EWeaponType::Chain) > 0 && !bChainShotTaken && ChainWaitSteps++ < 8)
		{
			--OfferProbeStep;
		}
		break;
	default:
		UE_LOG(LogEnemies, Display, TEXT("OFFERPROBE chain: owned=%d multiHitShot=%d"),
			Progression->GetWeaponLevel(EWeaponType::Chain) > 0 ? 1 : 0, bChainShotTaken ? 1 : 0);
		GetWorldTimerManager().ClearTimer(OfferProbeHandle);
		FPlatformMisc::RequestExit(false);
		break;
	}
}

void ADirectorGameMode::StartPerfProbe()
{
	AHeroCharacter* Hero = Cast<AHeroCharacter>(UGameplayStatics::GetPlayerPawn(this, 0));
	if (!Hero)
	{
		UE_LOG(LogEnemies, Error, TEXT("PERFPROBE aborted: no hero pawn."));
		FPlatformMisc::RequestExit(false);
		return;
	}

	// God mode keeps the hero alive; weapon and scheduler off keep exactly EnemyCount enemies alive.
	Hero->Health->bGodMode = true;
	Hero->Weapon->bEnabled = false;
	UEnemySpawnSubsystem* Spawner = GetWorld()->GetSubsystem<UEnemySpawnSubsystem>();
	Spawner->SetSchedulerEnabled(false);
	Spawner->SpawnHorde(PerfProbe.EnemyCount, true);

	SetActorTickEnabled(true);
}

void ADirectorGameMode::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	if (bPaceProbe)
	{
		// The pace bot walks in a circle (a player who keeps moving), the weapons fire on their own.
		if (AHeroCharacter* Hero = Cast<AHeroCharacter>(UGameplayStatics::GetPlayerPawn(this, 0)))
		{
			PaceAngle += DeltaSeconds * 0.8f;
			Hero->AddMovementInput(FVector(FMath::Cos(PaceAngle), FMath::Sin(PaceAngle), 0.0f), 1.0f);
		}
		return;
	}

	const float FrameSeconds = FApp::GetDeltaTime();
	SampledSeconds += FrameSeconds;
	WorstFrameSeconds = FMath::Max(WorstFrameSeconds, FrameSeconds);
	++SampleCount;

	if (SampledSeconds >= PerfProbe.SampleSeconds)
	{
		const int32 Alive = GetWorld()->GetSubsystem<UEnemyManagerSubsystem>()->GetAliveCount();
		const float AvgFps = SampleCount / SampledSeconds;
		const float MinFps = 1.0f / FMath::Max(WorstFrameSeconds, UE_SMALL_NUMBER);
		UE_LOG(LogEnemies, Display, TEXT("PERFPROBE enemies=%d avgFPS=%.1f minFPS=%.1f samples=%d"), Alive, AvgFps, MinFps, SampleCount);
		SetActorTickEnabled(false);
		FPlatformMisc::RequestExit(false);
	}
}
