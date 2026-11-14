#include "Loot/LootHeatSubsystem.h"

void ULootHeatSubsystem::SetTuning(const FLootTuning& InTuning)
{
	Tuning = InTuning;
	Heat = FMath::Clamp(Heat, 0.0f, Tuning.MaxHeat);
}

float ULootHeatSubsystem::GetHeatNormalized() const
{
	return Tuning.MaxHeat > 0.0f ? FMath::Clamp(Heat / Tuning.MaxHeat, 0.0f, 1.0f) : 0.0f;
}

float ULootHeatSubsystem::GetSpawnPressureMultiplier() const
{
	return FMath::Lerp(Tuning.SpawnPressureAtZeroHeat, Tuning.SpawnPressureAtMaxHeat, GetHeatNormalized());
}

void ULootHeatSubsystem::AddHeat(float Amount)
{
	const float NewHeat = FMath::Clamp(Heat + Amount, 0.0f, Tuning.MaxHeat);
	const float Delta = NewHeat - Heat;
	if (FMath::IsNearlyZero(Delta))
	{
		return;
	}
	Heat = NewHeat;
	UE_LOG(LogLoot, Log, TEXT("Heat %.1f (%+.1f)"), Heat, Delta);
	OnHeatChanged.Broadcast(Heat, Delta);
}

void ULootHeatSubsystem::AddRarePickupHeat()
{
	AddHeat(Tuning.HeatPerRarePickup);
}

void ULootHeatSubsystem::ResetHeat()
{
	AddHeat(-Heat);
}
