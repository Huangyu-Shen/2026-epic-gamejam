#include "Editing/FilmCardSubsystem.h"

#include "Enemies/EnemySpawnSubsystem.h"
#include "Extraction/RunOutcomeSubsystem.h"
#include "Game/DirectorTuningSettings.h"
#include "Engine/DataTable.h"

DEFINE_LOG_CATEGORY(LogEditing);

void UFilmCardSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	Collection.InitializeDependency<URunOutcomeSubsystem>();

	const UDirectorTuningSettings* Settings = GetDefault<UDirectorTuningSettings>();
	CardTable = Settings->FilmCardTable.LoadSynchronous();
	if (CardTable)
	{
		AvailableCards = CardTable->GetRowNames();
	}

	if (URunOutcomeSubsystem* Run = GetWorld()->GetSubsystem<URunOutcomeSubsystem>())
	{
		Run->OnRunEnded.AddDynamic(this, &UFilmCardSubsystem::HandleRunEnded);
	}
}

const FFilmCardRow* UFilmCardSubsystem::FindRow(FName CardId) const
{
	if (!CardTable)
	{
		return nullptr;
	}
	return CardTable->FindRow<FFilmCardRow>(CardId, TEXT("UFilmCardSubsystem"), false);
}

bool UFilmCardSubsystem::AddCard(FName CardId)
{
	if (ActiveCards.Num() >= MaxSlots || ActiveCards.Contains(CardId) || !AvailableCards.Contains(CardId))
	{
		return false;
	}
	ActiveCards.Add(CardId);
	UE_LOG(LogEditing, Log, TEXT("Slotted card [%s] (%d/%d)."), *CardId.ToString(), ActiveCards.Num(), MaxSlots);
	return true;
}

bool UFilmCardSubsystem::RemoveCard(FName CardId)
{
	const int32 Removed = ActiveCards.RemoveSingle(CardId);
	if (Removed > 0)
	{
		UE_LOG(LogEditing, Log, TEXT("Removed card [%s]."), *CardId.ToString());
	}
	return Removed > 0;
}

float UFilmCardSubsystem::GetEarningsMultiplier() const
{
	float Multiplier = 1.0f;
	for (const FName CardId : ActiveCards)
	{
		if (const FFilmCardRow* Row = FindRow(CardId))
		{
			Multiplier *= Row->Multiplier;
		}
	}
	return Multiplier;
}

void UFilmCardSubsystem::ApplyActiveCards()
{
	UEnemySpawnSubsystem* Spawner = GetWorld()->GetSubsystem<UEnemySpawnSubsystem>();
	for (const FName CardId : ActiveCards)
	{
		const FFilmCardRow* Row = FindRow(CardId);
		if (!Row)
		{
			continue;
		}

		switch (Row->Effect)
		{
		case EFilmCardEffect::ForceHorde:
			if (Spawner)
			{
				Spawner->SpawnHorde(Row->HordeCount, /*bIgnoreCap=*/true);
			}
			break;

		case EFilmCardEffect::ForceBoss:
			if (Spawner)
			{
				Spawner->SpawnBoss();
			}
			break;
		}
	}

	if (URunOutcomeSubsystem* Run = GetWorld()->GetSubsystem<URunOutcomeSubsystem>())
	{
		Run->SetEarningsMultiplier(GetEarningsMultiplier());
	}

	UE_LOG(LogEditing, Log, TEXT("Applied %d card(s), earnings multiplier x%.2f."), ActiveCards.Num(), GetEarningsMultiplier());
}

void UFilmCardSubsystem::ResetCards()
{
	ActiveCards.Reset();

	if (URunOutcomeSubsystem* Run = GetWorld()->GetSubsystem<URunOutcomeSubsystem>())
	{
		Run->SetEarningsMultiplier(1.0f);
	}
}

void UFilmCardSubsystem::HandleRunEnded(const FRunResult& Result)
{
	ResetCards();
}
