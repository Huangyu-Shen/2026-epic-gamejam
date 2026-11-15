#include "Loot/LootConfig.h"
#include "Engine/DataTable.h"
#include "Game/DirectorTuningSettings.h"

ULootConfig::ULootConfig()
{
	// Editable inline defaults (used only when no tables are assigned).
	// One row per reel type. Each unlocks one upgrade option (story 009); a reel with no option never drops.
	FRelicRow FastCut;
	FastCut.RelicType = ERelicType::Swiftstep;
	FastCut.DisplayName = FText::FromString(TEXT("Fast Cut"));
	FastCut.UnlocksOption = TEXT("Haste");

	FRelicRow WideShot;
	WideShot.RelicType = ERelicType::Warbrand;
	WideShot.DisplayName = FText::FromString(TEXT("Wide Shot"));
	WideShot.UnlocksOption = TEXT("Magnet");

	FRelicRow ChainScene;
	ChainScene.RelicType = ERelicType::Aegis;
	ChainScene.DisplayName = FText::FromString(TEXT("Chain Scene"));
	ChainScene.UnlocksOption = TEXT("Chain");

	DefaultRelics = { FastCut, WideShot, ChainScene };

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

ULootConfig* ULootConfig::CreateFromSettings(UObject* Outer)
{
	ULootConfig* Config = NewObject<ULootConfig>(Outer);
	if (const UDirectorTuningSettings* Settings = GetDefault<UDirectorTuningSettings>())
	{
		Config->TuningTable = Settings->LootTuningTable.LoadSynchronous();
		Config->ContainerTable = Settings->LootContainerTable.LoadSynchronous();
		Config->RelicTable = Settings->ReelTable.LoadSynchronous();
	}
	return Config;
}

bool ULootConfig::IsTuningFromTable() const
{
	return TuningTable && TuningTable->FindRow<FLootTuning>(TuningRowName, TEXT("ULootConfig::IsTuningFromTable"), false) != nullptr;
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

bool ULootConfig::PickRandomRelic(const TArray<FName>& UnlockedOptions, const TArray<ERelicType>& ReelsInBag, ERelicType& OutType) const
{
	TArray<FRelicRow> Candidates;
	for (const FRelicRow& Row : GetRelicRows())
	{
		if (Row.IsDroppable(UnlockedOptions, ReelsInBag))
		{
			Candidates.Add(Row);
		}
	}
	if (Candidates.Num() == 0)
	{
		return false;
	}

	float Total = 0.0f;
	for (const FRelicRow& Row : Candidates)
	{
		Total += FMath::Max(0.0f, Row.DropWeight);
	}
	if (Total <= 0.0f)
	{
		OutType = Candidates[FMath::RandRange(0, Candidates.Num() - 1)].RelicType;
		return true;
	}

	float Roll = FMath::FRandRange(0.0f, Total);
	for (const FRelicRow& Row : Candidates)
	{
		Roll -= FMath::Max(0.0f, Row.DropWeight);
		if (Roll <= 0.0f)
		{
			OutType = Row.RelicType;
			return true;
		}
	}
	OutType = Candidates.Last().RelicType;
	return true;
}
