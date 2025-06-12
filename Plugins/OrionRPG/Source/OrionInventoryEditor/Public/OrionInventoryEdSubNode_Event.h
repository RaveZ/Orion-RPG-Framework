// Copyright 2024 Ivan Chandra. All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "OrionInventoryEdSubNode.h"
#include "EdGraph/EdGraphNode.h"
#include "OrionInventoryEdSubNode_Event.generated.h"

/**
 * 
 */
UCLASS()
class ORIONINVENTORYEDITOR_API UOrionInventoryEdSubNode_Event : public UOrionInventoryEdSubNode
{
	GENERATED_BODY()

public:
	UOrionInventoryEdSubNode_Event();
	virtual FText GetNodeTitle(ENodeTitleType::Type TitleType) const override;
	/** gets icon resource name for title bar */
	virtual FName GetNameIcon() const override;
	virtual FText GetDescription() const override;
};
