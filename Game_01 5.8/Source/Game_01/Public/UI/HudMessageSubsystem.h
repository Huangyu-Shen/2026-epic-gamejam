// Director's Cut - Story 007. Short on-screen messages ("Searched: +3 Iron", "Pickup refused: bag full").
// They are shown by the HUD and the camp screen instead of the engine's debug text, which drew over the HP bar.
#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "HudMessageSubsystem.generated.h"

/** One message and the moment it stops showing (platform seconds). */
struct FHudMessage
{
	FString Text;
	FLinearColor Color = FLinearColor::White;
	double ExpireTime = 0.0;
};

/**
 * Keeps the newest few messages for the whole session, so the run HUD and the camp screen share one source.
 * Post() is safe to call with no world or game instance (it does nothing, e.g. in the editor preview world).
 */
UCLASS()
class GAME_01_API UHudMessageSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	/** Most messages kept; older ones are dropped first. */
	static constexpr int32 MaxMessages = 4;

	/** Shows Text for Seconds. */
	static void Post(const UObject* WorldContext, const FString& Text, const FLinearColor& Color = FLinearColor::White, float Seconds = 2.5f);

	/** Messages that have not expired yet, oldest first. */
	TArray<FHudMessage> GetActive() const;

private:
	TArray<FHudMessage> Messages;
};
