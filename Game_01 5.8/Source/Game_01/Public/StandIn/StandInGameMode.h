#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "StandInGameMode.generated.h"

/** THROWAWAY game mode for story 001: spawns AStandInCharacter as the player. Replaced by story 003/007. */
UCLASS()
class GAME_01_API AStandInGameMode : public AGameModeBase
{
	GENERATED_BODY()

public:
	AStandInGameMode();
};
