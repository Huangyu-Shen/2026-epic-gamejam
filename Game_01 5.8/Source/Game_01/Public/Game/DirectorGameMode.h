// Director's Cut - Story 003 (design/game-brief.md, MVP feature 1). Replaces AStandInGameMode.
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "DirectorGameMode.generated.h"

/**
 * Game mode for the Director's Cut run map: the hero is the default pawn, and the run music starts.
 * The test probes (-PerfProbe, -OfferProbe and the others) live in UDirectorProbeSubsystem (Dev/DirectorProbeSubsystem.h).
 */
UCLASS()
class GAME_01_API ADirectorGameMode : public AGameModeBase
{
	GENERATED_BODY()

public:
	ADirectorGameMode();

	virtual void BeginPlay() override;
};
