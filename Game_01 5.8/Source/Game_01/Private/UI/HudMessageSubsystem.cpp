#include "UI/HudMessageSubsystem.h"
#include "Engine/GameInstance.h"
#include "Engine/World.h"
#include "HAL/PlatformTime.h"

void UHudMessageSubsystem::Post(const UObject* WorldContext, const FString& Text, const FLinearColor& Color, float Seconds)
{
	const UWorld* World = WorldContext ? WorldContext->GetWorld() : nullptr;
	UHudMessageSubsystem* Self = World ? UGameInstance::GetSubsystem<UHudMessageSubsystem>(World->GetGameInstance()) : nullptr;
	if (!Self)
	{
		return;
	}

	FHudMessage Message;
	Message.Text = Text;
	Message.Color = Color;
	Message.ExpireTime = FPlatformTime::Seconds() + Seconds;
	Self->Messages.Add(Message);
	while (Self->Messages.Num() > MaxMessages)
	{
		Self->Messages.RemoveAt(0);
	}
}

TArray<FHudMessage> UHudMessageSubsystem::GetActive() const
{
	const double Now = FPlatformTime::Seconds();
	TArray<FHudMessage> Active;
	for (const FHudMessage& Message : Messages)
	{
		if (Message.ExpireTime > Now)
		{
			Active.Add(Message);
		}
	}
	return Active;
}
