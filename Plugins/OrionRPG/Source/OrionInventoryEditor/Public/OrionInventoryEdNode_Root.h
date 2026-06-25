// Copyright 2024 Ivan Chandra. All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "OrionInventoryEdNode.h"
#include "EdGraph/EdGraphNode.h"
#include "OrionInventoryEdNode_Root.generated.h"

/**
 * 
 */
UCLASS()
class ORIONINVENTORYEDITOR_API UOrionInventoryEdNode_Root : public UOrionInventoryEdNode
{
	GENERATED_BODY()

public:
	UOrionInventoryEdNode_Root();
	virtual void AllocateDefaultPins() override;
	virtual FText GetTooltipText() const override;
	virtual bool CanDuplicateNode() const override { return false; }
	virtual bool CanUserDeleteNode() const override { return false; }
	virtual FText GetNodeTitle(ENodeTitleType::Type TitleType) const override;
	virtual FLinearColor GetBackgroundColor() const override;
};
