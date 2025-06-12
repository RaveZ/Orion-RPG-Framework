// Copyright 2025 Ivan Chandra. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "OrionInventoryEdNode.h"
#include "OrionInventoryEdNode_Speaker.generated.h"

/**
 * 
 */
UCLASS()
class ORIONINVENTORYEDITOR_API UOrionInventoryEdNode_Speaker : public UOrionInventoryEdNode
{
	GENERATED_BODY()
public:
	virtual FText GetNodeTitle(ENodeTitleType::Type TitleType) const override;
	virtual FLinearColor GetBackgroundColor() const override;
	virtual FText GetTooltipText() const override;
	/** Gets a list of actions that can be done to this particular node */
	virtual void GetNodeContextMenuActions(class UToolMenu* Menu, class UGraphNodeContextMenuContext* Context) const override;

};
