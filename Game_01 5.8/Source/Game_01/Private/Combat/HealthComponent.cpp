#include "Combat/HealthComponent.h"
#include "Game/DirectorTuningSettings.h"
#include "Engine/DataTable.h"
#include "Engine/World.h"

UHealthComponent::UHealthComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UHealthComponent::BeginPlay()
{
	Super::BeginPlay();

	// Project Settings > Game > Director Tuning. A missing table or row silently keeps the inline defaults.
	const UDirectorTuningSettings* Settings = GetDefault<UDirectorTuningSettings>();
	if (const UDataTable* HealthTable = Settings->HealthTuningTable.LoadSynchronous())
	{
		if (const FHealthTuning* Row = HealthTable->FindRow<FHealthTuning>(TEXT("Hero"), TEXT("UHealthComponent::BeginPlay"), false))
		{
			Tuning = *Row;
		}
	}

	BaseMaxHealth = Tuning.MaxHealth;
	ResetHealth();
}

bool UHealthComponent::IsInvulnerable() const
{
	return GetWorld() && GetWorld()->GetTimeSeconds() < InvulnerableUntil;
}

float UHealthComponent::ApplyDamage(float Amount)
{
	if (Amount <= 0.0f || IsDead() || bGodMode || IsInvulnerable())
	{
		return 0.0f;
	}

	const float Applied = FMath::Min(Amount, Health);
	Health -= Applied;
	OnDamaged.Broadcast(Applied, Health);

	if (IsDead())
	{
		OnDied.Broadcast();
	}
	else
	{
		GrantInvulnerability(Tuning.HitInvulnerabilitySeconds);
	}
	return Applied;
}

void UHealthComponent::Kill()
{
	if (IsDead())
	{
		return;
	}
	const float Applied = Health;
	Health = 0.0f;
	OnDamaged.Broadcast(Applied, Health);
	OnDied.Broadcast();
}

void UHealthComponent::Heal(float Amount)
{
	if (Amount > 0.0f && !IsDead())
	{
		Health = FMath::Min(Health + Amount, Tuning.MaxHealth);
	}
}

void UHealthComponent::ResetHealth()
{
	Health = Tuning.MaxHealth;
	InvulnerableUntil = 0.0;
}

void UHealthComponent::IncreaseMaxHealth(float Amount)
{
	if (Amount <= 0.0f)
	{
		return;
	}
	Tuning.MaxHealth += Amount;
	Health = FMath::Min(Health + Amount, Tuning.MaxHealth);
}

void UHealthComponent::SetMaxHealth(float NewMax)
{
	Tuning.MaxHealth = FMath::Max(NewMax, 1.0f);
	Health = Tuning.MaxHealth;
	InvulnerableUntil = 0.0;
}

void UHealthComponent::GrantInvulnerability(float Seconds)
{
	if (const UWorld* World = GetWorld())
	{
		InvulnerableUntil = FMath::Max(InvulnerableUntil, World->GetTimeSeconds() + static_cast<double>(Seconds));
	}
}
