#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Loot/LootTypes.h"
#include "LootContainer.generated.h"

class ALootContainer;
class ULootConfig;
class UStaticMesh;
class UStaticMeshComponent;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_FourParams(FOnContainerSearched, ALootContainer*, Container, int32, IronGained, bool, bGotRelic, ERelicType, RelicType);

/**
 * A supply crate or vault. One search per container; the yield comes from the
 * container's FLootContainerRow. The interact key (story 003) calls Interact().
 * If the bag is too full, nothing is consumed and the container stays
 * searchable; the rolled loot is remembered so retrying cannot re-roll.
 * Greybox: the mesh defaults to the engine cube.
 */
UCLASS()
class GAME_01_API ALootContainer : public AActor
{
	GENERATED_BODY()

public:
	ALootContainer();

	virtual void BeginPlay() override;

	/** Which row of the container data this uses. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Loot")
	ELootContainerType ContainerType = ELootContainerType::SupplyCrate;

	/** Optional override; otherwise the instigator's bag config is used. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Loot")
	TObjectPtr<ULootConfig> ConfigOverride;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Loot")
	TObjectPtr<UStaticMeshComponent> Mesh;

	UPROPERTY(BlueprintAssignable, Category = "Loot")
	FOnContainerSearched OnSearched;

	/**
	 * Searches the container on behalf of Instigator (needs a ULootBagComponent
	 * on it, or on its pawn if it is a controller). Returns true if the search
	 * completed. False if already searched, no bag, or the bag refused the loot.
	 */
	UFUNCTION(BlueprintCallable, Category = "Loot")
	bool Interact(AActor* InInstigator);

private:
	/** The bag of the actor that interacts: on the actor itself, or on the pawn of a controller. */
	static ULootBagComponent* FindBag(AActor* InInstigator);

	/** Rolls what this container holds, once. Returns false when no row is set up for its type. */
	bool RollContents(const ULootBagComponent& Bag, ULootConfig& Config);

public:

	UFUNCTION(BlueprintPure, Category = "Loot")
	bool HasBeenSearched() const { return bSearched; }

private:
	/** Flattens the mesh so a searched container reads as opened. */
	void ShowOpened();

	UPROPERTY(VisibleInstanceOnly, Transient, Category = "Loot", meta = (AllowPrivateAccess = "true"))
	bool bSearched = false;

	/** Meshes of the "RPG Tiny Fantasy Forest" pack: a tree stump for crates, a boulder for vaults. Null without the pack. */
	UPROPERTY()
	TObjectPtr<UStaticMesh> CrateModel;

	UPROPERTY()
	TObjectPtr<UStaticMesh> VaultModel;

	/** True once a pack model replaced the cube (its origin is at its base, so opening it does not sink it). */
	bool bUsesModel = false;

	bool bRolled = false;
	int32 RolledIron = 0;
	bool bRolledRelic = false;
	ERelicType RolledRelicType = ERelicType::Swiftstep;
};
