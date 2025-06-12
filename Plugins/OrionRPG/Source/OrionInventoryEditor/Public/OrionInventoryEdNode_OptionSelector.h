// Copyright 2025 Ivan Chandra. All Rights Reserved.UOrionInventoryEdNode_OptionSelector

#pragma once

#include "CoreMinimal.h"
#include "OrionInventoryEdNode.h"
#include "EdGraph/EdGraphNode.h"
#include "OrionInventoryEdNode_OptionSelector.generated.h"

/**
 * 
 */
UCLASS()
class ORIONINVENTORYEDITOR_API UOrionInventoryEdNode_OptionSelector : public UOrionInventoryEdNode
{
	GENERATED_BODY()
public:
	UOrionInventoryEdNode_OptionSelector();
	virtual FText GetNodeTitle(ENodeTitleType::Type TitleType) const override;
	virtual FText GetTooltipText() const override;
	virtual FLinearColor GetBackgroundColor() const override;
	/** Gets a list of actions that can be done to this particular node */
	virtual void GetNodeContextMenuActions(class UToolMenu* Menu, class UGraphNodeContextMenuContext* Context) const override;

};
