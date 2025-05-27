// Copyright 2025 Ivan Chandra. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "DialogBuilderNode.h"
#include "Dialog.h"
#include "DialogBuilderNode_OptionSelector.generated.h"


UCLASS(/*HideCategories = ("Detail")*/)
class ORIONRPG_API UDialogBuilderNode_OptionSelector : public UDialogBuilderNode
{
	GENERATED_BODY()
public:

    virtual void Initialize() override;

#if WITH_EDITOR
    virtual FText GetNodeTitle() const override;
    virtual void SetNodeTitle(const FText& NewTitle);
    virtual FText GetNodeDescription() const override;

    virtual FLinearColor GetBackgroundColor() const override;
#endif
};
