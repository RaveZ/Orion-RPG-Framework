// Copyright 2025 Ivan Chandra. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "DialogBuilderNode.h"
#include "DialogBuilderNode_RerouteNode.generated.h"


UCLASS(BlueprintType)
class ORIONRPG_API UDialogBuilderNode_RerouteNode : public UDialogBuilderNode
{
	GENERATED_BODY()
	
public:
	virtual void BeginNode() override;
	virtual void SelectOption(class UDialogBuilderNode_PlayerOption* Option);
	
public:
#if WITH_EDITOR
    virtual FText GetNodeTitle() const override;
    virtual void SetNodeTitle(const FText& NewTitle);
    virtual FText GetNodeDescription() const override;
#endif
};
