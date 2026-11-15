#include "Flow/CampSaveSubsystem.h"
#include "Flow/CampSaveGame.h"
#include "Camp/CampMetaSubsystem.h"
#include "Extraction/CampStockSubsystem.h"
#include "Loot/LootConfig.h"
#include "Engine/GameInstance.h"
#include "Kismet/GameplayStatics.h"
#include "TimerManager.h"
#include "HAL/FileManager.h"
#include "HAL/PlatformFileManager.h"
#include "Misc/CommandLine.h"
#include "Misc/Paths.h"
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

void UCampSaveSubsystem::BackupSlot(const TCHAR* Reason)
{
	const FString Source = FPaths::ProjectSavedDir() / TEXT("SaveGames") / GetSlotName() + TEXT(".sav");
	const FString Target = Source + TEXT(".") + Reason + TEXT(".bak");
	IPlatformFile& Files = FPlatformFileManager::Get().GetPlatformFile();
	if (Files.FileExists(*Source) && !Files.FileExists(*Target))
	{
		const bool bCopied = Files.CopyFile(*Target, *Source);
		UE_LOG(LogCampSave, Log, TEXT("Backup of the save (%s): %s"), Reason, bCopied ? TEXT("written") : TEXT("FAILED"));
	}
}

bool UCampSaveSubsystem::SlotFileLooksValid()
{
	const FString Source = FPaths::ProjectSavedDir() / TEXT("SaveGames") / GetSlotName() + TEXT(".sav");
	TUniquePtr<FArchive> Reader(IFileManager::Get().CreateFileReader(*Source));
	if (!Reader || Reader->TotalSize() < 4)
	{
		return false;
	}
	uint8 Header[4] = {};
	Reader->Serialize(Header, 4);
	return FSaveVersionCheck::LooksLikeSaveFile(Header, 4);
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
	if (UGameplayStatics::DoesSaveGameExist(GetSlotName(), 0) && !SlotFileLooksValid())
	{
		// Not a save file (empty, half-written, damaged). The engine's loader asserts on such a file, so it is never given one.
		BackupSlot(TEXT("unreadable"));
		UE_LOG(LogCampSave, Warning, TEXT("Camp save file is not a valid save; backed up and starting a new camp"));
	}
	else if (UGameplayStatics::DoesSaveGameExist(GetSlotName(), 0))
	{
		const UCampSaveGame* Save = Cast<UCampSaveGame>(UGameplayStatics::LoadGameFromSlot(GetSlotName(), 0));
		const FSaveVersionCheck::EResult Check = Save ? FSaveVersionCheck::Classify(Save->SaveVersion, UCampSaveGame::CurrentSaveVersion) : FSaveVersionCheck::EResult::Current;
		if (Save && Check == FSaveVersionCheck::EResult::TooNew)
		{
			// A newer game wrote this file. Keep it as it is (the next change would overwrite the slot) and start a new camp.
			BackupSlot(TEXT("newer-version"));
			UE_LOG(LogCampSave, Warning, TEXT("Camp save is version %d, newer than this game (%d); backed up and starting a new camp"),
				Save->SaveVersion, UCampSaveGame::CurrentSaveVersion);
		}
		else if (Save)
		{
			if (Check == FSaveVersionCheck::EResult::NeedsMigration)
			{
				BackupSlot(TEXT("before-migration"));
			}
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
			Stock->RestoreState(Save->Iron, Unlocked, Save->NewOptions, Save->Stone);
			Meta->RestoreLevels(Save->BuildingLevels);
			bApplyingLoad = false;
			UE_LOG(LogCampSave, Log, TEXT("Loaded camp save: Iron %d, %d unlocked options, %d building entries"),
				Save->Iron, Unlocked.Num(), Save->BuildingLevels.Num());
		}
		else
		{
			BackupSlot(TEXT("unreadable"));
			UE_LOG(LogCampSave, Warning, TEXT("Camp save slot exists but could not be loaded; backed up and starting fresh"));
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
	// A change made in the last frame must not be lost when the game shuts down.
	if (bSavePending)
	{
		SaveNow();
	}
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
	if (bApplyingLoad || bSavePending)
	{
		return;
	}
	bSavePending = true;
	GetGameInstance()->GetTimerManager().SetTimerForNextTick(FTimerDelegate::CreateWeakLambda(this, [this]()
	{
		if (bSavePending)
		{
			SaveNow();
		}
	}));
}

bool UCampSaveSubsystem::SaveNow()
{
	bSavePending = false;
	const UCampStockSubsystem* Stock = GetGameInstance()->GetSubsystem<UCampStockSubsystem>();
	const UCampMetaSubsystem* Meta = GetGameInstance()->GetSubsystem<UCampMetaSubsystem>();
	if (!Stock || !Meta)
	{
		return false;
	}

	UCampSaveGame* Save = Cast<UCampSaveGame>(UGameplayStatics::CreateSaveGameObject(UCampSaveGame::StaticClass()));
	Save->SaveVersion = UCampSaveGame::CurrentSaveVersion;
	Save->Iron = Stock->GetIron();
	Save->Stone = Stock->GetStone();
	Save->UnlockedOptions = Stock->GetUnlockedOptions();
	Save->NewOptions = Stock->GetNewOptions();
	Save->BuildingLevels = Meta->GetLevels();

	const bool bOk = UGameplayStatics::SaveGameToSlot(Save, GetSlotName(), 0);
	UE_LOG(LogCampSave, Log, TEXT("Camp save %s: Iron %d, %d unlocked options"), bOk ? TEXT("written") : TEXT("FAILED"),
		Save->Iron, Save->UnlockedOptions.Num());
	return bOk;
}

