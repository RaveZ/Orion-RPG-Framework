// Copyright 2025 Ivan Chandra. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Inventory/InventoryData.h"
#include "NativeGameplayTags.h"
#include "OrionInventoryItemFragment.generated.h"

class UOrionInventoryItemInstance;
//////////////////////////////////////////////////////////////////////

// Represents a fragment of an item definition
UCLASS(MinimalAPI, DefaultToInstanced, EditInlineNew, Abstract)
class UOrionInventoryItemFragment : public UObject
{
	GENERATED_BODY()

public:
	UOrionInventoryItemFragment();
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Fragment)
	bool bIsMenuAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Fragment)
	FText ActionDisplayName;

	virtual void OnInstanceCreated(UOrionInventoryItemInstance* Instance) const {}
};


//////////////////////////////////////////////////////////////////////

// Equippable item fragment
UCLASS(meta = (DisplayName="Equippable Item Fragment"))
class UOrionInventoryFragment_EquippableItem : public UOrionInventoryItemFragment
{
	GENERATED_BODY()

public:
	UOrionInventoryFragment_EquippableItem();

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Fragment)
	TSoftClassPtr<AActor> ActorToSpawn;

	/**Which socket to attach this equipment*/
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Fragment)
	FName AttachSocket;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Fragment)
	FTransform AttachTransform;
};


//////////////////////////////////////////////////////////////////////

// Consumable item fragment
UCLASS(meta = (DisplayName = "Consumable Item Fragment"))
class UOrionInventoryFragment_ConsumableItem : public UOrionInventoryItemFragment
{
	GENERATED_BODY()

public:
	UOrionInventoryFragment_ConsumableItem();

};


