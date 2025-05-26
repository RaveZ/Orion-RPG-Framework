// Copyright 2025 Ivan Chandra. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "DialogBuilderNode.h"
#include "DialogBuilderNode_Speaker.generated.h"

class UDialogTask;

UCLASS(Blueprintable)
class ORIONRPG_API UDialogBuilderNode_Speaker : public UDialogBuilderNode
{
	GENERATED_BODY()

public:

	virtual void Initialize() override;
	virtual void Deinitialize() override;




#if WITH_EDITOR
	virtual FText GetNodeTitle() const override;
	virtual void SetNodeTitle(const FText& NewTitle);
	virtual FText GetNodeDescription() const override;
#endif
};
