// Director's Cut - all gameplay numbers are authored here, in Project Settings > Game > Director Tuning.
#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "Engine/DeveloperSettings.h"
#include "DirectorTuningSettings.generated.h"

/**
 * Data tables holding every gameplay number: enemy stats, wave rate and cap, swarm limits and hero health.
 * Each table is optional; an unset table or a missing row silently keeps the C++ defaults.
 */
UCLASS(Config = Game, DefaultConfig, meta = (DisplayName = "Director Tuning"))
class GAME_01_API UDirectorTuningSettings : public UDeveloperSettings
{
	GENERATED_BODY()

public:
	UDirectorTuningSettings();

	/** Table of FEnemyTuning. Row "Grunt" overrides the Grunt stats, row "Boss" the Boss stats. */
	UPROPERTY(EditAnywhere, Config, Category = "Tuning", meta = (RequiredAssetDataTags = "RowStructure=/Script/Game_01.EnemyTuning"))
	TSoftObjectPtr<UDataTable> EnemyTuningTable;

	/** Table of FSpawnTuning. Row "Spawn" overrides wave rate, alive cap and ring radii. */
	UPROPERTY(EditAnywhere, Config, Category = "Tuning", meta = (RequiredAssetDataTags = "RowStructure=/Script/Game_01.SpawnTuning"))
	TSoftObjectPtr<UDataTable> SpawnTuningTable;

	/** Table of FSwarmTuning. Row "Swarm" overrides the chase stop fraction and separation limits. */
	UPROPERTY(EditAnywhere, Config, Category = "Tuning", meta = (RequiredAssetDataTags = "RowStructure=/Script/Game_01.SwarmTuning"))
	TSoftObjectPtr<UDataTable> SwarmTuningTable;

	/** Table of FHealthTuning. Row "Hero" overrides hero max health and hit invulnerability. */
	UPROPERTY(EditAnywhere, Config, Category = "Tuning", meta = (RequiredAssetDataTags = "RowStructure=/Script/Game_01.HealthTuning"))
	TSoftObjectPtr<UDataTable> HealthTuningTable;

	/** Table of FXpCurveRow. Row "Progression" overrides the XP threshold curve and per-kill XP. */
	UPROPERTY(EditAnywhere, Config, Category = "Tuning", meta = (RequiredAssetDataTags = "RowStructure=/Script/Game_01.XpCurveRow"))
	TSoftObjectPtr<UDataTable> ProgressionTable;

	/** Table of FWeaponRow. Rows "Blade", "Bolt", "Nova" override the three weapons. */
	UPROPERTY(EditAnywhere, Config, Category = "Tuning", meta = (RequiredAssetDataTags = "RowStructure=/Script/Game_01.WeaponRow"))
	TSoftObjectPtr<UDataTable> WeaponTable;

	/** Table of FPassiveRow. Rows "Swiftness", "Might", "Vitality", "Greed" override the four passives. */
	UPROPERTY(EditAnywhere, Config, Category = "Tuning", meta = (RequiredAssetDataTags = "RowStructure=/Script/Game_01.PassiveRow"))
	TSoftObjectPtr<UDataTable> PassiveTable;

	/** Table of FFilmCardRow. Rows "HordeNight" and "CutKnight" define the pre-run film cards. */
	UPROPERTY(EditAnywhere, Config, Category = "Tuning", meta = (RequiredAssetDataTags = "RowStructure=/Script/Game_01.FilmCardRow"))
	TSoftObjectPtr<UDataTable> FilmCardTable;

	/** Table of FBuildingRow. Rows "Smithy" and "Clinic" define the camp buildings. */
	UPROPERTY(EditAnywhere, Config, Category = "Tuning", meta = (RequiredAssetDataTags = "RowStructure=/Script/Game_01.BuildingRow"))
	TSoftObjectPtr<UDataTable> BuildingTable;
};
