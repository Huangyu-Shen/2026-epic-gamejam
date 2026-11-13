#include "Flow/CampSaveSubsystem.h"
#include "Flow/CampSaveGame.h"
#include "Camp/CampMetaSubsystem.h"
#include "Extraction/CampStockSubsystem.h"
#include "Kismet/GameplayStatics.h"

DEFINE_LOG_CATEGORY_STATIC(LogCampSave, Log, All);

const FString& UCampSaveSubsystem::GetSlotName()
{
	static const FString Slot(TEXT("CampSave"));
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
			Stock->RestoreState(Save->Iron, Save->WarehouseRelics);
			Meta->RestoreLevels(Save->BuildingLevels);
			bApplyingLoad = false;
			UE_LOG(LogCampSave, Log, TEXT("Loaded camp save: Iron %d, %d relics, %d building entries"),
				Save->Iron, Save->WarehouseRelics.Num(), Save->BuildingLevels.Num());
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
	Save->Iron = Stock->GetIron();
	Save->WarehouseRelics = Stock->GetWarehouseRelics();
	Save->BuildingLevels = Meta->GetLevels();

	const bool bOk = UGameplayStatics::SaveGameToSlot(Save, GetSlotName(), 0);
	UE_LOG(LogCampSave, Log, TEXT("Camp save %s: Iron %d, %d relics"), bOk ? TEXT("written") : TEXT("FAILED"),
		Save->Iron, Save->WarehouseRelics.Num());
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
