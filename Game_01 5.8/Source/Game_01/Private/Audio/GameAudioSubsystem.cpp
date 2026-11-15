#include "Audio/GameAudioSubsystem.h"
#include "Components/AudioComponent.h"
#include "Engine/World.h"
#include "Game/DirectorTuningSettings.h"
#include "Kismet/GameplayStatics.h"
#include "Sound/SoundBase.h"

DEFINE_LOG_CATEGORY_STATIC(LogGameAudio, Log, All);

bool UGameAudioSubsystem::ShouldCreateSubsystem(UObject* Outer) const
{
	// Game worlds only: editor preview worlds and commandlets have no use for sound.
	const UWorld* World = Cast<UWorld>(Outer);
	return World && World->IsGameWorld();
}

void UGameAudioSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	// Load every effect now, not on first use: a hitch in the middle of a fight is worse than a longer load.
	const UDirectorTuningSettings* Settings = GetDefault<UDirectorTuningSettings>();
	Loaded.SetNum(static_cast<int32>(EGameSound::Count));
	int32 Assigned = 0;
	for (const TPair<EGameSound, TSoftObjectPtr<USoundBase>>& Pair : Settings->Sounds)
	{
		if (Pair.Key != EGameSound::Count && !Pair.Value.IsNull())
		{
			Loaded[static_cast<int32>(Pair.Key)] = Pair.Value.LoadSynchronous();
			Assigned += Loaded[static_cast<int32>(Pair.Key)] ? 1 : 0;
		}
	}
	UE_LOG(LogGameAudio, Log, TEXT("AUDIOCFG sounds assigned=%d of %d, master=%.2f music=%.2f sfx=%.2f crowd cap=%d/s"),
		Assigned, static_cast<int32>(EGameSound::Count), Settings->MasterVolume, Settings->MusicVolume, Settings->SfxVolume, Settings->MaxCrowdSoundsPerSecond);
}

void UGameAudioSubsystem::Deinitialize()
{
	if (Capped > 0 || MusicComponent)
	{
		UE_LOG(LogGameAudio, Log, TEXT("AUDIO totals: capped by the crowd limit=%d"), Capped);
	}
	if (MusicComponent)
	{
		MusicComponent->Stop();
		MusicComponent = nullptr;
	}
	Super::Deinitialize();
}

bool UGameAudioSubsystem::IsCrowdSound(EGameSound Sound)
{
	return Sound == EGameSound::Attack || Sound == EGameSound::EnemyHit || Sound == EGameSound::EnemyDeath;
}

void UGameAudioSubsystem::PlayIn(const UObject* WorldContext, EGameSound Sound)
{
	const UWorld* World = WorldContext ? WorldContext->GetWorld() : nullptr;
	if (UGameAudioSubsystem* Audio = World ? World->GetSubsystem<UGameAudioSubsystem>() : nullptr)
	{
		Audio->Play(Sound);
	}
}

void UGameAudioSubsystem::Play(EGameSound Sound)
{
	const int32 Index = static_cast<int32>(Sound);
	if (Index < 0 || Index >= static_cast<int32>(EGameSound::Count))
	{
		return;
	}
	++Requests[Index];

	const UDirectorTuningSettings* Settings = GetDefault<UDirectorTuningSettings>();
	const double Now = GetWorld()->GetRealTimeSeconds();

	if (IsCrowdSound(Sound))
	{
		CrowdTimes.RemoveAll([Now](double T) { return Now - T > 1.0; });
		if (CrowdTimes.Num() >= Settings->MaxCrowdSoundsPerSecond)
		{
			++Capped;
			return;
		}
		CrowdTimes.Add(Now);
	}

	USoundBase* Asset = Loaded.IsValidIndex(Index) ? Loaded[Index].Get() : nullptr;
	if (!bLoggedFirst[Index])
	{
		bLoggedFirst[Index] = true;
		UE_LOG(LogGameAudio, Log, TEXT("AUDIO first request: %s (%s)"), *StaticEnum<EGameSound>()->GetNameStringByIndex(Index), Asset ? TEXT("asset assigned") : TEXT("no asset assigned, silent"));
	}
	if (Asset)
	{
		++Played[Index];
		UGameplayStatics::PlaySound2D(this, Asset, Settings->MasterVolume * Settings->SfxVolume);
	}
}

void UGameAudioSubsystem::StartMusic(EMusicTrack Track)
{
	if (bHasMusicTrack && CurrentTrack == Track && MusicComponent && MusicComponent->IsPlaying())
	{
		return; // never restart a track on top of itself
	}
	if (MusicComponent)
	{
		MusicComponent->FadeOut(0.5f, 0.0f);
		MusicComponent = nullptr;
	}

	const UDirectorTuningSettings* Settings = GetDefault<UDirectorTuningSettings>();
	const TSoftObjectPtr<USoundBase>& Ref = Track == EMusicTrack::Camp ? Settings->CampMusic : Settings->RunMusic;
	bHasMusicTrack = true;
	CurrentTrack = Track;
	USoundBase* Asset = Ref.IsNull() ? nullptr : Ref.LoadSynchronous();
	UE_LOG(LogGameAudio, Log, TEXT("MUSIC start %s (%s)"), Track == EMusicTrack::Camp ? TEXT("Camp") : TEXT("Run"), Asset ? TEXT("asset assigned") : TEXT("no asset assigned, silent"));
	if (Asset)
	{
		MusicComponent = UGameplayStatics::SpawnSound2D(this, Asset, Settings->MasterVolume * Settings->MusicVolume, 1.0f, 0.0f, nullptr, false, false);
	}
}
