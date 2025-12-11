// Copyright 2025 Ivan Chandra. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Inventory/InventoryData.h"
#include "NativeGameplayTags.h"
#include "Engine/Texture2D.h"
#include "OrionInventoryItemFragment.generated.h"

class UOrionInventoryItemInstance;
//////////////////////////////////////////////////////////////////////

// Represents a fragment of an item definition
UCLASS(Abstract, Blueprintable, BlueprintType, DefaultToInstanced, EditInlineNew)
class ORIONRPG_API UOrionInventoryItemFragment : public UObject
{
	GENERATED_BODY()

public:
	UOrionInventoryItemFragment();
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Fragment)
	bool bHasMenuAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Fragment, meta = (EditCondition = "bHasMenuAction == true", HideEditConditionToggle, EditConditionHides))
	FText ActionDisplayName;

	UPROPERTY(BlueprintReadOnly, Category = Fragment)
	TObjectPtr<UOrionInventoryItemInstance> OwningItemInstance;

public:

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, DisplayName = "Can Execute Action", Category = Fragment)
	bool K2_CanExecuteAction() const;

	virtual bool K2_CanExecuteAction_Implementation() const;

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, DisplayName = "Execute Action", Category = Fragment)
	void K2_ExecuteAction();

	virtual void K2_ExecuteAction_Implementation();

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, DisplayName = "Get Action Display Name", Category = Fragment)
	FText K2_GetActionDisplayName() const;

	virtual FText K2_GetActionDisplayName_Implementation() const;

	virtual void OnInstanceCreated(UOrionInventoryItemInstance* Instance) { OwningItemInstance = Instance; }

	// Allows the Object to get a valid UWorld from it's outer.
	virtual UWorld* GetWorld() const override;
};


//////////////////////////////////////////////////////////////////////

// Consumable item fragment
UCLASS(meta = (DisplayName = "Consumable Fragment"))
class UOrionInventoryFragment_ConsumableItem : public UOrionInventoryItemFragment
{
	GENERATED_BODY()

public:
	UOrionInventoryFragment_ConsumableItem();

};


//////////////////////////////////////////////////////////////////////

// Inspect item fragment
UCLASS(meta = (DisplayName = "Inspectable Fragment"))
class UOrionInventoryFragment_InspectItem : public UOrionInventoryItemFragment
{
	GENERATED_BODY()

public:
	UOrionInventoryFragment_InspectItem();

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Fragment)
	TSoftClassPtr<AActor> ActorToInspect;

	virtual void K2_ExecuteAction_Implementation() override;
};


//////////////////////////////////////////////////////////////////////

// Drop item fragment
UCLASS(meta = (DisplayName = "Drop Item Fragment"))
class UOrionInventoryFragment_DropItem : public UOrionInventoryItemFragment
{
	GENERATED_BODY()

public:
	UOrionInventoryFragment_DropItem();

	virtual void K2_ExecuteAction_Implementation() override;
};


