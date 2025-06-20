// Copyright 2025 Ivan Chandra. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "OrionInventoryNode.h"
#include "OrionInventoryNode_ItemCategory.generated.h"

UCLASS(Blueprintable, BlueprintType)
class ORIONRPG_API UOrionInventoryNode_ItemCategory : public UOrionInventoryNode
{
	GENERATED_BODY()

public:
	UOrionInventoryNode_ItemCategory();

public:


public:
#if WITH_EDITOR

	//Node Setting
	virtual FText GetNodeTitle() const override;
	virtual void SetNodeTitle(const FText& NewTitle);
	virtual FText GetNodeDescription() const override;
#endif
};
