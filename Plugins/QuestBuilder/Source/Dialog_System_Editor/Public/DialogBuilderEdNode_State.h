// Copyright 2025 Ivan Chandra. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "DialogBuilderEdNode.h"
#include "EdGraph/EdGraphNode.h"
#include "DialogBuilderEdNode_State.generated.h"

/**
 * 
 */
UCLASS()
class DIALOG_SYSTEM_EDITOR_API UDialogBuilderEdNode_State : public UDialogBuilderEdNode
{
	GENERATED_BODY()
public:
	UDialogBuilderEdNode_State();
	virtual void AllocateDefaultPins() override;
	virtual FText GetNodeTitle(ENodeTitleType::Type TitleType) const override;
	virtual FLinearColor GetBackgroundColor() const override;
};
