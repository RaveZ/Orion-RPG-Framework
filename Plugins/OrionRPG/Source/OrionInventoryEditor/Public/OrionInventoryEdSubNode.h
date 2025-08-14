// Copyright 2024 Ivan Chandra. All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "OrionInventoryEdNode.h"
#include "EdGraph/EdGraphNode.h"
#include "OrionInventoryEdSubNode.generated.h"

/**
 * 
 */
UCLASS()
class ORIONINVENTORYEDITOR_API UOrionInventoryEdSubNode : public UOrionInventoryEdNode
{
	GENERATED_BODY()

public:
	UOrionInventoryEdSubNode();
	virtual void AllocateDefaultPins() override;
	virtual bool CanDuplicateNode() const override { return true; }
	virtual bool CanUserDeleteNode() const override { return true; }
	virtual FText GetNodeTitle(ENodeTitleType::Type TitleType) const override;
	virtual FLinearColor GetBackgroundColor() const override;
};
