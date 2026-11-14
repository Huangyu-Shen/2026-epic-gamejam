#include "Flow/CampSaveSubsystem.h"
#include "Flow/CampSaveGame.h"
#include "Camp/CampMetaSubsystem.h"
#include "Extraction/CampStockSubsystem.h"
#include "Loot/LootConfig.h"
#include "Kismet/GameplayStatics.h"
#include "Misc/CommandLine.h"
#include "Misc/Parse.h"

DEFINE_LOG_CATEGORY_STATIC(LogCampSave, Log, All);

const FString& UCampSaveSubsystem::GetSlotName()
{
	// -SaveSlot=Name picks another slot, so a probe or a clean-save test never touches the real save.
	static const FString Slot = []()
	{
		FString Override;
		return FParse::Value(FCommandLine::Get(), TEXT("SaveSlot="), Override) && !Override.IsEmpty() ? Override : FString(TEXT("CampSave"));
	}();
	return Slot;
}

void UCampSaveSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	Collection.InitializeDependency<UCampStockSubsystem>();
	Collection.InitializeDependency<UCampMetaSubsystem>();

	UCampStockSubsystem* Stock = GetGameInstance()->GetSubsystem<UCampStockSubsystem>();
	UCampMetaSubsystem* Meta = GetGameInstance()->GetSubsystem<UCampMetaSubsystem>();
	if (!Stock || !Meta)
	{
		return;
	}

	// Load first, bind after: restoring must not trigger a save.
	if (UGameplayStatics::DoesSaveGameExist(GetSlotName(), 0))
	{
		if (const UCampSaveGame* Save = Cast<UCampSaveGame>(UGameplayStatics::LoadGameFromSlot(GetSlotName(), 0)))
		{
			bApplyingLoad = true;
			TArray<FName> Unlocked = Save->UnlockedOptions;
			if (Save->SaveVersion < 2)
			{
				// A v1 save kept relics in a warehouse: each one becomes the option its reel row names.
				const ULootConfig* Config = ULootConfig::CreateFromSettings(GetTransientPackage());
				for (const FName Option : FReelUnlocks::OptionsForReels(Save->WarehouseRelics, Config->GetRelicRows()))
				{
					FReelUnlocks::AddOption(Unlocked, Option);
				}
				FString RelicNames;
				for (const ERelicType Relic : Save->WarehouseRelics)
				{
					RelicNames += StaticEnum<ERelicType>()->GetNameStringByValue(static_cast<int64>(Relic)) + TEXT(" ");
				}
				UE_LOG(LogCampSave, Log, TEXT("Migrated a v%d save: %d relics [%s] -> %d unlocked options"),
					Save->SaveVersion, Save->WarehouseRelics.Num(), *RelicNames, Unlocked.Num());
			}
			Stock->RestoreState(Save->Iron, Unlocked);
			Meta->RestoreLevels(Save->BuildingLevels);
			bApplyingLoad = false;
			UE_LOG(LogCampSave, Log, TEXT("Loaded camp save: Iron %d, %d unlocked options, %d building entries"),
				Save->Iron, Unlocked.Num(), Save->BuildingLevels.Num());
		}
		else
		{
			UE_LOG(LogCampSave, Warning, TEXT("Camp save slot exists but could not be loaded; starting fresh"));
		}
	}
	else
	{
		UE_LOG(LogCampSave, Log, TEXT("No camp save found; starting a new camp"));
	}

	Stock->OnStockChanged.AddDynamic(this, &UCampSaveSubsystem::HandleCampChanged);
	Meta->OnMetaChanged.AddDynamic(this, &UCampSaveSubsystem::HandleCampChanged);
}

void UCampSaveSubsystem::Deinitialize()
{
	if (UGameInstance* GI = GetGameInstance())
	{
		if (UCampStockSubsystem* Stock = GI->GetSubsystem<UCampStockSubsystem>())
		{
			Stock->OnStockChanged.RemoveDynamic(this, &UCampSaveSubsystem::HandleCampChanged);
		}
		if (UCampMetaSubsystem* Meta = GI->GetSubsystem<UCampMetaSubsystem>())
		{
			Meta->OnMetaChanged.RemoveDynamic(this, &UCampSaveSubsystem::HandleCampChanged);
		}
	}
	Super::Deinitialize();
}

void UCampSaveSubsystem::HandleCampChanged()
{
	if (!bApplyingLoad)
	{
		SaveNow();
	}
}

bool UCampSaveSubsystem::SaveNow()
{
	const UCampStockSubsystem* Stock = GetGameInstance()->GetSubsystem<UCampStockSubsystem>();
	const UCampMetaSubsystem* Meta = GetGameInstance()->GetSubsystem<UCampMetaSubsystem>();
	if (!Stock || !Meta)
	{
		return false;
	}

	UCampSaveGame* Save = Cast<UCampSaveGame>(UGameplayStatics::CreateSaveGameObject(UCampSaveGame::StaticClass()));
	Save->SaveVersion = UCampSaveGame::CurrentSaveVersion;
	Save->Iron = Stock->GetIron();
	Save->UnlockedOptions = Stock->GetUnlockedOptions();
	Save->BuildingLevels = Meta->GetLevels();

	const bool bOk = UGameplayStatics::SaveGameToSlot(Save, GetSlotName(), 0);
	UE_LOG(LogCampSave, Log, TEXT("Camp save %s: Iron %d, %d unlocked options"), bOk ? TEXT("written") : TEXT("FAILED"),
		Save->Iron, Save->UnlockedOptions.Num());
	return bOk;
}

void UCampSaveSubsystem::DeleteSaveAndReset()
{
	bApplyingLoad = true;
	UGameplayStatics::DeleteGameInSlot(GetSlotName(), 0);
	if (UCampStockSubsystem* Stock = GetGameInstance()->GetSubsystem<UCampStockSubsystem>())
	{
		Stock->RestoreState(0, {});
	}
	if (UCampMetaSubsystem* Meta = GetGameInstance()->GetSubsystem<UCampMetaSubsystem>())
	{
		Meta->ResetCamp();
	}
	bApplyingLoad = false;
}
