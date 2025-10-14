// Copyright 2025 Ivan Chandra. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "OrionInventoryNode.h"
#include "Inventory/InventoryData.h"
#include "NativeGameplayTags.h"
#include "OrionInventoryNode_ItemDefinition.generated.h"

class UOrionInventoryItemInstance;
//////////////////////////////////////////////////////////////////////

// Represents a fragment of an item definition
UCLASS(MinimalAPI, DefaultToInstanced, EditInlineNew, Abstract)
class UOrionInventoryItemFragment : public UObject
{
	GENERATED_BODY()

public:
	virtual void OnInstanceCreated(UOrionInventoryItemInstance* Instance) const {}
};

//////////////////////////////////////////////////////////////////////

/**
 * UOrionInventoryNode_ItemDefinition
 */
UCLASS(Blueprintable, Const)
class ORIONRPG_API UOrionInventoryNode_ItemDefinition : public UOrionInventoryNode
{
	GENERATED_BODY()

public:
	UOrionInventoryNode_ItemDefinition();


	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Detail", meta = (ShowOnlyInnerProperties))
	FInventoryItemDefinition ItemDefinition;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Display, Instanced)
	TArray<TObjectPtr<UOrionInventoryItemFragment>> Fragments;

public:
	const UOrionInventoryItemFragment* FindFragmentByClass(TSubclassOf<UOrionInventoryItemFragment> FragmentClass) const;


public:
#if WITH_EDITOR

	//Node Setting
	virtual FText GetNodeTitle() const override;
	virtual void SetNodeTitle(const FText& NewTitle);
	virtual FText GetNodeDescription() const override;
#endif
};
