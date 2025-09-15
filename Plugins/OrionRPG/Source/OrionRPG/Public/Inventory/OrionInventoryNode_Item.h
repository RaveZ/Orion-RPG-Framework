// Copyright 2025 Ivan Chandra. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "OrionInventoryNode.h"
#include "Inventory/InventoryData.h"
#include "NativeGameplayTags.h"
#include "OrionInventoryNode_Item.generated.h"

UCLASS(Blueprintable, BlueprintType)

class ORIONRPG_API UOrionInventoryNode_Item : public UOrionInventoryNode
{
	GENERATED_BODY()

public:
	UOrionInventoryNode_Item();


	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Detail", meta = (ShowOnlyInnerProperties))
	FInventoryItemDefinition ItemDefinition;
	

public:


public:
#if WITH_EDITOR

	//Node Setting
	virtual FText GetNodeTitle() const override;
	virtual void SetNodeTitle(const FText& NewTitle);
	virtual FText GetNodeDescription() const override;
#endif
};
