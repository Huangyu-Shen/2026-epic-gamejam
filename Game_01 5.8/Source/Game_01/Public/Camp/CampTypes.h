// Director's Cut - Story 006 (design/game-brief.md, MVP feature 6: camp meta growth).
#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "CampTypes.generated.h"

DECLARE_LOG_CATEGORY_EXTERN(LogCamp, Log, All);

/** The two upgradable camp buildings. Each raises a run stat. */
UENUM(BlueprintType)
enum class EBuildingType : uint8
{
	Smithy UMETA(DisplayName = "Smithy"),	// weapon strength: flat damage added to every weapon
	Clinic UMETA(DisplayName = "Clinic"),	// survivability: flat max health added
	Count UMETA(Hidden),
};

/** Data row for one building. Member initializers are the fallback when the table is missing. */
USTRUCT(BlueprintType)
struct GAME_01_API FBuildingRow : public FTableRowBase
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Building")
	EBuildingType BuildingType = EBuildingType::Smithy;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Building")
	FText DisplayName;

	/** Cost of the next upgrade (wood or stone, see GetCostResource): floor(BaseCost * CostGrowth^CurrentLevel). */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Building", meta = (ClampMin = "0"))
	int32 BaseCost = 10;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Building", meta = (ClampMin = "1.0"))
	float CostGrowth = 2.0f;

	/** Flat weapon damage added per Smithy level (all weapons). */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Building", meta = (ClampMin = "0.0"))
	float DamagePerLevel = 5.0f;

	/** Flat max health added per Clinic level. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Building", meta = (ClampMin = "0.0"))
	float MaxHealthPerLevel = 25.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Building", meta = (ClampMin = "1"))
	int32 MaxLevel = 5;

	/** What the next upgrade costs when the building is at CurrentLevel: floor(BaseCost * CostGrowth^CurrentLevel). */
	int32 CostForLevel(int32 CurrentLevel) const
	{
		return FMath::FloorToInt32(static_cast<float>(BaseCost) * FMath::Pow(CostGrowth, static_cast<float>(CurrentLevel)));
	}

	/** Total flat weapon damage a Smithy at this level adds. */
	float DamageBonusForLevel(int32 Level) const { return static_cast<float>(Level) * DamagePerLevel; }

	/** Total flat max health a Clinic at this level adds. */
	float MaxHealthBonusForLevel(int32 Level) const { return static_cast<float>(Level) * MaxHealthPerLevel; }
};
