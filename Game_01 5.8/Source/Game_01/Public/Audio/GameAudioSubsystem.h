// Director's Cut - Story 011 (design/game-brief.md, MVP feature 6). The one place gameplay code asks for sound.
#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "GameAudioSubsystem.generated.h"

class UAudioComponent;
class USoundBase;

/** Every sound gameplay code can request. The asset for each is set in Project Settings > Director Tuning > Audio. */
UENUM(BlueprintType)
enum class EGameSound : uint8
{
	Attack,
	EnemyHit,
	EnemyDeath,
	Pickup,
	LevelUp,
	Cut,
	StarUp,
	Extraction,
	PlayerDeath,
	/** A camp building was upgraded. */
	Upgrade,
	/** The hero stepped into the extraction point and the bar started filling. */
	ExtractStart,
	Count UMETA(Hidden)
};

/** The two music tracks (PRD section 6, row 11). */
UENUM(BlueprintType)
enum class EMusicTrack : uint8
{
	Camp,
	Run
};

/** The three volume levels the Esc menu changes. */
enum class EAudioBus : uint8
{
	Master,
	Music,
	Effects,
};

/**
 * Single audio entry point. Gameplay calls Play(EGameSound) and StartMusic(EMusicTrack); nothing else in the game
 * touches sound assets. A missing asset is a silent no-op that is counted and logged once, so the call sites can be
 * verified from the log before any sound file exists. Crowd sounds (attack, hit, death) share a per-second cap.
 */
UCLASS()
class GAME_01_API UGameAudioSubsystem : public UWorldSubsystem
{
	GENERATED_BODY()

public:
	virtual bool ShouldCreateSubsystem(UObject* Outer) const override;
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	/** Plays one sound effect, unless it is a crowd sound and the per-second cap is already used up. */
	void Play(EGameSound Sound);

	/** Convenience for call sites that only have a world context. Safe with a null context or world. */
	static void PlayIn(const UObject* WorldContext, EGameSound Sound);

	/** Starts a looping music track. Does nothing if that track is already playing; stops the other track first. */
	void StartMusic(EMusicTrack Track);

	/** Volume 0 to 1 of a bus: the player's setting for this session, else the value in Director Tuning. */
	static float GetBusVolume(EAudioBus Bus);

	/** Sets a bus (clamped to 0..1) for this session and applies it to the music that is playing now. */
	static void SetBusVolume(const UObject* WorldContext, EAudioBus Bus, float Volume);

	/** Number of Play requests for a sound this world, counting capped and asset-less ones. */
	int32 GetRequestCount(EGameSound Sound) const { return Requests[static_cast<int32>(Sound)]; }

	/** Number of requests dropped by the crowd cap. */
	int32 GetCappedCount() const { return Capped; }

private:
	static bool IsCrowdSound(EGameSound Sound);

	UPROPERTY(Transient)
	TArray<TObjectPtr<USoundBase>> Loaded;

	UPROPERTY(Transient)
	TObjectPtr<UAudioComponent> MusicComponent;

	int32 Requests[static_cast<int32>(EGameSound::Count)] = {};
	int32 Played[static_cast<int32>(EGameSound::Count)] = {};
	int32 Capped = 0;
	bool bLoggedFirst[static_cast<int32>(EGameSound::Count)] = {};
	TArray<double> CrowdTimes;
	bool bHasMusicTrack = false;
	EMusicTrack CurrentTrack = EMusicTrack::Camp;
};
