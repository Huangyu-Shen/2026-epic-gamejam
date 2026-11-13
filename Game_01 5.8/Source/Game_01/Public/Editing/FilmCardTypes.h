// Director's Cut - Story 005 (design/game-brief.md, MVP feature 5: pre-run film cards).
#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "FilmCardTypes.generated.h"

/** Log category for the film-card (pre-run editing) system. */
DECLARE_LOG_CATEGORY_EXTERN(LogEditing, Log, All);

/**
 * What a film card forces into a run. Difficulty comes only from the cards the player picks,
 * never from random rolls.
 */
UENUM(BlueprintType)
enum class EFilmCardEffect : uint8
{
	/** Spawn a horde of Grunts at run start. */
	ForceHorde,
	/** Spawn one elite/Boss at run start. */
	ForceBoss,
};

/**
 * One film card. Usable as a DataTable row: the row key is the card's id, the member initializers
 * are the fallback used when no row is found. Adding a third card later is a data-row change only.
 */
USTRUCT(BlueprintType)
struct GAME_01_API FFilmCardRow : public FTableRowBase
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Card")
	EFilmCardEffect Effect = EFilmCardEffect::ForceHorde;

	/** Grunts spawned when Effect is ForceHorde. Ignored by ForceBoss. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Card", meta = (ClampMin = "1"))
	int32 HordeCount = 40;

	/** Earnings multiplier this card adds while active (cards multiply together). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Card", meta = (ClampMin = "1.0"))
	float Multiplier = 1.5f;
};
