// Copyright 2025 Ivan Chandra. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "DialogBuilderNode.h"
#include "DialogBuilderNode_Objective.generated.h"


UCLASS(Blueprintable)
class DIALOG_SYSTEM_RUNTIME_API UDialogBuilderNode_Objective : public UDialogBuilderNode
{
	GENERATED_BODY()

public:

	virtual void Initialize() override;

#if WITH_EDITOR
	virtual FText GetNodeTitle() const override;
	virtual void SetNodeTitle(const FText& NewTitle);
	virtual FText GetNodeDescription() const override;
#endif
};
