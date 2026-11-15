#include "Game/DirectorGameMode.h"
#include "Audio/GameAudioSubsystem.h"
#include "Engine/World.h"
#include "Player/HeroCharacter.h"

ADirectorGameMode::ADirectorGameMode()
{
	DefaultPawnClass = AHeroCharacter::StaticClass();
}

void ADirectorGameMode::BeginPlay()
{
	Super::BeginPlay();

	if (UGameAudioSubsystem* Audio = GetWorld()->GetSubsystem<UGameAudioSubsystem>())
	{
		Audio->StartMusic(EMusicTrack::Run);
	}
}
