// Director's Cut - Story 006. Game mode for the camp scene; spawns the greybox camp.
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "CampGameMode.generated.h"

UCLASS()
class GAME_01_API ACampGameMode : public AGameModeBase
{
	GENERATED_BODY()

public:
	ACampGameMode();

protected:
	virtual void BeginPlay() override;

private:
	/** -CampShot: logs the camp state, saves a screenshot to Saved/ReelProbe/ and exits. */
	void TakeCampShot();

	FTimerHandle CampShotHandle;
};
