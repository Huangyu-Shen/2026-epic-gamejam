#include "StandIn/StandInGameMode.h"
#include "StandIn/StandInCharacter.h"

AStandInGameMode::AStandInGameMode()
{
	DefaultPawnClass = AStandInCharacter::StaticClass();
}
