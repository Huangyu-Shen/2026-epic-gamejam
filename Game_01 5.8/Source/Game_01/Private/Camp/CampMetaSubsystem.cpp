#include "Camp/CampMetaSubsystem.h"
#include "Audio/GameAudioSubsystem.h"
#include "Extraction/CampStockSubsystem.h"
#include "Game/DirectorTuningSettings.h"
#include "Engine/DataTable.h"

DEFINE_LOG_CATEGORY(LogCamp);

namespace
{
	const TCHAR* BuildingRowName(EBuildingType Type)
	{
		switch (Type)
		{
		case EBuildingType::Smithy: return TEXT("Smithy");
		case EBuildingType::Clinic:  return TEXT("Clinic");
		default: return TEXT("");
		}
	}

	FText BuildingDisplayName(EBuildingType Type)
	{
		return StaticEnum<EBuildingType>()->GetDisplayNameTextByValue(static_cast<int64>(Type));
	}

	FBuildingRow DefaultBuilding(EBuildingType Type)
	{
		FBuildingRow Row;
		Row.BuildingType = Type;
		if (Type == EBuildingType::Clinic)
		{
			Row.DamagePerLevel = 0.0f;
			Row.MaxHealthPerLevel = 25.0f;
		}
		return Row;
	}
}

void UCampMetaSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	Collection.InitializeDependency<UCampStockSubsystem>();

	const UDirectorTuningSettings* Settings = GetDefault<UDirectorTuningSettings>();
	const TCHAR* Context = TEXT("UCampMetaSubsystem::Initialize");

	for (uint8 B = 0; B < static_cast<uint8>(EBuildingType::Count); ++B)
	{
		const EBuildingType Type = static_cast<EBuildingType>(B);
		FBuildingRow Row = DefaultBuilding(Type);
		if (const UDataTable* Table = Settings->BuildingTable.LoadSynchronous())
		{
			if (const FBuildingRow* Found = Table->FindRow<FBuildingRow>(BuildingRowName(Type), Context, false))
			{
				Row = *Found;
				Row.BuildingType = Type; // the row key is the source of truth, not the row's own type field
			}
		}
		Buildings.Add(Type, Row);
	}
}

int32 UCampMetaSubsystem::GetBuildingLevel(EBuildingType Type) const
{
	return Levels.FindRef(Type);
}

int32 UCampMetaSubsystem::GetUpgradeCost(EBuildingType Type) const
{
	const FBuildingRow& Row = Buildings.FindChecked(Type);
	return Row.CostForLevel(Levels.FindRef(Type));
}

bool UCampMetaSubsystem::IsMaxLevel(EBuildingType Type) const
{
	return Levels.FindRef(Type) >= Buildings.FindChecked(Type).MaxLevel;
}

bool UCampMetaSubsystem::TryUpgrade(EBuildingType Type, FText& Reason)
{
	if (!Buildings.Contains(Type))
	{
		Reason = FText::FromString(TEXT("Unknown building"));
		return false;
	}

	if (IsMaxLevel(Type))
	{
		Reason = FText::FromString(FString::Printf(TEXT("%s is already at max level"), *BuildingDisplayName(Type).ToString()));
		return false;
	}

	const int32 Cost = GetUpgradeCost(Type);
	UCampStockSubsystem* Stock = GetGameInstance()->GetSubsystem<UCampStockSubsystem>();
	const bool bStone = GetCostResource(Type) == ELootResource::Stone;
	if (!Stock || !(bStone ? Stock->SpendStone(Cost) : Stock->SpendWood(Cost)))
	{
		Reason = FText::FromString(FString::Printf(TEXT("Not enough %s (need %d)"), bStone ? TEXT("stone") : TEXT("wood"), Cost));
		return false;
	}

	int32& Level = Levels.FindOrAdd(Type);
	++Level;
	UE_LOG(LogCamp, Log, TEXT("%s upgraded to level %d for %d %s"), *BuildingDisplayName(Type).ToString(), Level, Cost, bStone ? TEXT("stone") : TEXT("wood"));
	UGameAudioSubsystem::PlayIn(GetGameInstance(), EGameSound::Upgrade);
	OnMetaChanged.Broadcast();
	return true;
}

float UCampMetaSubsystem::GetWeaponDamageBonus() const
{
	return Buildings.FindChecked(EBuildingType::Smithy).DamageBonusForLevel(Levels.FindRef(EBuildingType::Smithy));
}

float UCampMetaSubsystem::GetMaxHealthBonus() const
{
	return Buildings.FindChecked(EBuildingType::Clinic).MaxHealthBonusForLevel(Levels.FindRef(EBuildingType::Clinic));
}

void UCampMetaSubsystem::RestoreLevels(const TMap<EBuildingType, int32>& InLevels)
{
	Levels.Reset();
	for (const TPair<EBuildingType, int32>& Pair : InLevels)
	{
		if (const FBuildingRow* Row = Buildings.Find(Pair.Key))
		{
			Levels.Add(Pair.Key, FMath::Clamp(Pair.Value, 0, Row->MaxLevel));
		}
	}
	OnMetaChanged.Broadcast();
}

const FBuildingRow& UCampMetaSubsystem::GetBuildingRow(EBuildingType Type) const
{
	return Buildings.FindChecked(Type);
}
