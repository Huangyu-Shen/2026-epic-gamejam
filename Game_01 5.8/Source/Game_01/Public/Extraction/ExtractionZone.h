// Director's Cut - Story 002 (design/game-brief.md, MVP feature 4). Greybox extraction point.
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Extraction/ExtractionTypes.h"
#include "ExtractionZone.generated.h"

class ULootBagComponent;
class UBoxComponent;
class UPrimitiveComponent;
class UStaticMeshComponent;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnExtractionProgress, float, Progress);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnExtractionInterrupted);

/**
 * Extraction point. While a pawn carrying a ULootBagComponent stands inside and the run
 * is active, the bar fills over ExtractionSeconds; a full bar calls
 * URunOutcomeSubsystem::ReportExtracted. Leaving empties (or decays) the bar; the run
 * ending by death interrupts it. Ticks only while someone is inside or the bar is draining.
 */
UCLASS()
class GAME_01_API AExtractionZone : public AActor
{
	GENERATED_BODY()

public:
	AExtractionZone();

	/** Bar rules and timing. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Extraction")
	FExtractionTuning Tuning;

	/** Fired with the bar value (0..1) every tick it changes. */
	UPROPERTY(BlueprintAssignable, Category = "Extraction")
	FOnExtractionProgress OnExtractionProgress;

	/** Fired when a partly filled bar is emptied by leaving or by the run ending. */
	UPROPERTY(BlueprintAssignable, Category = "Extraction")
	FOnExtractionInterrupted OnExtractionInterrupted;

	/** Bar value, 0..1. */
	UFUNCTION(BlueprintPure, Category = "Extraction")
	float GetProgress() const { return Progress; }

	virtual void Tick(float DeltaSeconds) override;

protected:
	virtual void BeginPlay() override;

private:
	UFUNCTION()
	void HandleBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	UFUNCTION()
	void HandleEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex);

	/** Picks up pawns that were already standing in the zone when play began. */
	void EnterPawnsAlreadyInside();

	/** Makes OtherActor the occupant if the zone is free and it is a pawn carrying a loot bag. */
	void TryEnter(AActor* OtherActor);

	/** Empties the bar and tells listeners if it was not already empty. */
	void Interrupt();

	UPROPERTY(VisibleAnywhere, Category = "Extraction")
	TObjectPtr<UStaticMeshComponent> DiscMesh;

	/** A tall bright column so the point can be found from across the map. */
	UPROPERTY(VisibleAnywhere, Category = "Extraction")
	TObjectPtr<UStaticMeshComponent> BeaconMesh;

	/** A second disc, drawn above the first, that grows from the middle as the bar fills. */
	UPROPERTY(VisibleAnywhere, Category = "Extraction")
	TObjectPtr<UStaticMeshComponent> FillMesh;

	void UpdateFillVisual() const;

	UPROPERTY(VisibleAnywhere, Category = "Extraction")
	TObjectPtr<UBoxComponent> ZoneVolume;

	/** The bag of the pawn currently extracting. */
	TWeakObjectPtr<ULootBagComponent> Occupant;

	float Progress = 0.0f;
};
