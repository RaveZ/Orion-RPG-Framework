// Copyright 2025 Ivan Chandra. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "DialogBuilderEdNode.h"
#include "DialogBuilderEdNode_Objective.generated.h"

/**
 * 
 */
UCLASS()
class DIALOG_SYSTEM_EDITOR_API UDialogBuilderEdNode_Objective : public UDialogBuilderEdNode
{
	GENERATED_BODY()
public:
	virtual FText GetNodeTitle(ENodeTitleType::Type TitleType) const override;
	virtual FLinearColor GetBackgroundColor() const override;
};
