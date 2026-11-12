#include "Loot/LootConfig.h"
#include "Engine/DataTable.h"

ULootConfig::ULootConfig()
{
	// Editable inline defaults (used only when no tables are assigned).
	// Relic buff values: one row per relic type.
	FRelicRow Swift;
	Swift.RelicType = ERelicType::Swiftstep;
	Swift.DisplayName = FText::FromString(TEXT("Swiftstep"));
	Swift.Buff.MoveSpeedMultiplier = 1.3f;
	Swift.Buff.DurationSeconds = 20.0f;

	FRelicRow War;
	War.RelicType = ERelicType::Warbrand;
	War.DisplayName = FText::FromString(TEXT("Warbrand"));
	War.Buff.DamageMultiplier = 1.5f;
	War.Buff.DurationSeconds = 20.0f;

	FRelicRow Aegis;
	Aegis.RelicType = ERelicType::Aegis;
	Aegis.DisplayName = FText::FromString(TEXT("Aegis"));
	Aegis.Buff.DamageTakenMultiplier = 0.6f;
	Aegis.Buff.DurationSeconds = 20.0f;

	DefaultRelics = { Swift, War, Aegis };

	FLootContainerRow Crate;
	Crate.ContainerType = ELootContainerType::SupplyCrate;
	Crate.DisplayName = FText::FromString(TEXT("Supply Crate"));
	Crate.IronMin = 3;
	Crate.IronMax = 6;
	Crate.RelicChance = 0.2f;

	FLootContainerRow Vault;
	Vault.ContainerType = ELootContainerType::Vault;
	Vault.DisplayName = FText::FromString(TEXT("Vault"));
	Vault.IronMin = 10;
	Vault.IronMax = 20;
	Vault.RelicChance = 1.0f;

	DefaultContainers = { Crate, Vault };
}

FLootTuning ULootConfig::GetTuning() const
{
	if (TuningTable)
	{
		if (const FLootTuning* Row = TuningTable->FindRow<FLootTuning>(TuningRowName, TEXT("ULootConfig::GetTuning"), false))
		{
			return *Row;
		}
	}
	return DefaultTuning;
}

TArray<FRelicRow> ULootConfig::GetRelicRows() const
{
	TArray<FRelicRow> Result;
	if (RelicTable)
	{
		TArray<FRelicRow*> Rows;
		RelicTable->GetAllRows<FRelicRow>(TEXT("ULootConfig::GetRelicRows"), Rows);
		for (const FRelicRow* Row : Rows)
		{
			if (Row)
			{
				Result.Add(*Row);
			}
		}
	}
	return Result.Num() > 0 ? Result : DefaultRelics;
}

bool ULootConfig::FindRelicRow(ERelicType Type, FRelicRow& OutRow) const
{
	for (const FRelicRow& Row : GetRelicRows())
	{
		if (Row.RelicType == Type)
		{
			OutRow = Row;
			return true;
		}
	}
	return false;
}

bool ULootConfig::FindContainerRow(ELootContainerType Type, FLootContainerRow& OutRow) const
{
	TArray<FLootContainerRow> Rows;
	if (ContainerTable)
	{
		TArray<FLootContainerRow*> TableRows;
		ContainerTable->GetAllRows<FLootContainerRow>(TEXT("ULootConfig::FindContainerRow"), TableRows);
		for (const FLootContainerRow* Row : TableRows)
		{
			if (Row)
			{
				Rows.Add(*Row);
			}
		}
	}
	if (Rows.Num() == 0)
	{
		Rows = DefaultContainers;
	}
	for (const FLootContainerRow& Row : Rows)
	{
		if (Row.ContainerType == Type)
		{
			OutRow = Row;
			return true;
		}
	}
	return false;
}

bool ULootConfig::PickRandomRelic(ERelicType& OutType) const
{
	const TArray<FRelicRow> Rows = GetRelicRows();
	if (Rows.Num() == 0)
	{
		return false;
	}

	float Total = 0.0f;
	for (const FRelicRow& Row : Rows)
	{
		Total += FMath::Max(0.0f, Row.DropWeight);
	}
	if (Total <= 0.0f)
	{
		OutType = Rows[FMath::RandRange(0, Rows.Num() - 1)].RelicType;
		return true;
	}

	float Roll = FMath::FRandRange(0.0f, Total);
	for (const FRelicRow& Row : Rows)
	{
		Roll -= FMath::Max(0.0f, Row.DropWeight);
		if (Roll <= 0.0f)
		{
			OutType = Row.RelicType;
			return true;
		}
	}
	OutType = Rows.Last().RelicType;
	return true;
}
