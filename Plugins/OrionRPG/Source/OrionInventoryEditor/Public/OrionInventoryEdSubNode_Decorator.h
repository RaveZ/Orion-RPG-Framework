// Copyright 2024 Ivan Chandra. All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "OrionInventoryEdSubNode.h"
#include "EdGraph/EdGraphNode.h"
#include "OrionInventoryEdSubNode_Decorator.generated.h"

/**
 * 
 */
UCLASS()
class ORIONINVENTORYEDITOR_API UOrionInventoryEdSubNode_Decorator : public UOrionInventoryEdSubNode
{
	GENERATED_BODY()

public:
	UOrionInventoryEdSubNode_Decorator();
	virtual FText GetNodeTitle(ENodeTitleType::Type TitleType) const override;
	/** gets icon resource name for title bar */
	virtual FName GetNameIcon() const override;
	virtual FText GetDescription() const override;
};
