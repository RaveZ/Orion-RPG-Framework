// Copyright 2025 Ivan Chandra. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "DialogBuilderNode.h"
#include "DialogBuilderNode_PlayerOption.generated.h"


UCLASS(BlueprintType)
class ORIONRPG_API UDialogBuilderNode_PlayerOption : public UDialogBuilderNode
{
	GENERATED_BODY()
public:

#if WITH_EDITOR
    virtual FText GetNodeTitle() const override;
    virtual void SetNodeTitle(const FText& NewTitle);
    virtual FText GetNodeDescription() const override;

#endif
};
