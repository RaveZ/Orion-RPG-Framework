// Copyright 2025 Ivan Chandra. All Rights Reserved.

#pragma once

#include "Engine/World.h"
#include "Inventory/InventoryData.h"
#include "OrionEquipmentInstance.generated.h"

class AActor;
class APawn;
class UOrionInventoryItemInstance;
struct FFrame;
struct FOrionEquipmentDefinition;
struct FOrionEquipmentActorData;

DECLARE_MULTICAST_DELEGATE(FOrionEquipSignature);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOrionEquipActorSignature, AActor*, EquipmentActor);
/**
 * UOrionEquipmentInstance
 *
 * A piece of equipment spawned and applied to a pawn
 */
UCLASS(BlueprintType, Blueprintable)
class ORIONRPG_API UOrionEquipmentInstance : public UObject
{
	GENERATED_BODY()

public:
	UOrionEquipmentInstance(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	//~UObject interface
	virtual bool IsSupportedForNetworking() const override { return true; }
	virtual UWorld* GetWorld() const override final;
	//~End of UObject interface

	UFUNCTION(BlueprintPure, Category = Equipment)
	UObject* GetInstigator() const { return Instigator; }

	void SetInstigator(UObject* InInstigator) { Instigator = InInstigator; }

	void SetOwningItemInstance(UOrionInventoryItemInstance* InItemInstance) { OwningItemInstance = InItemInstance; }

	UFUNCTION(BlueprintPure, Category = Equipment)
	APawn* GetPawn() const;

	UFUNCTION(BlueprintPure, Category = Equipment)
	UOrionInventoryItemInstance* GetOwningItemInstance() const;

	UFUNCTION(BlueprintPure, Category = Equipment, meta = (DeterminesOutputType = PawnType))
	APawn* GetTypedPawn(TSubclassOf<APawn> PawnType) const;

	UFUNCTION(BlueprintPure, Category = Equipment)
	TArray<FOrionEquipmentActorData> GetSpawnedActorDatas() const { return SpawnedActorDatas; }

	UFUNCTION(BlueprintCallable, Category = Equipment)
	virtual void SpawnEquipmentActors(const TArray<FOrionEquipmentActorData>& ActorsToSpawn, APawn* InPawn);
	virtual void DestroyEquipmentActors();

	virtual void OnEquipped();
	virtual void OnUnequipped();

	void SetEquipmentDef(FOrionEquipmentDefinition& InDef);

	UFUNCTION(BlueprintPure, Category = Equipment)
	FOrionEquipmentDefinition GetEquipmentDef() { return EquipmentDef; }

public:
	FOrionEquipSignature OnEquippedDelegate;
	FOrionEquipSignature OnUnequippedDelegate;

	UPROPERTY(BlueprintAssignable, BlueprintCallable, Category = Equipment)
	FOrionEquipActorSignature OnEquipmentActorSpawned;

protected:
#if UE_WITH_IRIS
	/** Register all replication fragments */
	virtual void RegisterReplicationFragments(UE::Net::FFragmentRegistrationContext& Context, UE::Net::EFragmentRegistrationFlags RegistrationFlags) override;
#endif // UE_WITH_IRIS

	UFUNCTION(BlueprintImplementableEvent, Category = Equipment, meta = (DisplayName = "OnEquipped"))
	void K2_OnEquipped();

	UFUNCTION(BlueprintImplementableEvent, Category = Equipment, meta = (DisplayName = "OnUnequipped"))
	void K2_OnUnequipped();

private:
	UFUNCTION()
	void OnRep_Instigator();

	UFUNCTION()
	void OnRep_SpawnedActorDatas();

private:
	UPROPERTY(Replicated)
	TObjectPtr<UOrionInventoryItemInstance> OwningItemInstance;

	UPROPERTY(ReplicatedUsing = OnRep_Instigator)
	TObjectPtr<UObject> Instigator;

	UPROPERTY(ReplicatedUsing = OnRep_SpawnedActorDatas)
	TArray<FOrionEquipmentActorData> SpawnedActorDatas;

	UPROPERTY(Replicated)
	FOrionEquipmentDefinition EquipmentDef;
};
