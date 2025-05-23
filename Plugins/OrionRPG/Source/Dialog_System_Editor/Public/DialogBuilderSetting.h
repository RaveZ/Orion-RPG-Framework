// Copyright 2025 Ivan Chandra. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "DialogBuilderSetting.generated.h"



UCLASS(config = Engine, defaultconfig)
class DIALOG_SYSTEM_EDITOR_API UDialogBuilderSetting : public UObject
{
	GENERATED_BODY()

	UDialogBuilderSetting();
	virtual ~UDialogBuilderSetting();

public:

	UPROPERTY(EditAnywhere, config, Category = "Graph Style")
	FLinearColor FailedNodeColor;

	UPROPERTY(EditAnywhere, config, Category = "Graph Style")
	FLinearColor SuccessNodeColor;

	UPROPERTY(EditAnywhere, config, Category = "Graph Style")
	FLinearColor StateNodeColor;

	UPROPERTY(EditAnywhere, config, Category = "Graph Style")
	FLinearColor RootNodeColor;

	UPROPERTY(EditAnywhere, config, Category = "Graph Style")
	FLinearColor ObjectiveNodeColor;

	UPROPERTY(EditAnywhere, config, Category = "Graph Style")
	FLinearColor CheckpointNodeColor;

	UPROPERTY(EditAnywhere, config, Category = "Graph Style")
	FLinearColor SubNodeColor;

	UPROPERTY(BlueprintReadOnly, config, Category = "Graph Rule")
	bool bEdgeEnabled;

	UPROPERTY(BlueprintReadOnly, config, Category = "Graph Rule")
	bool bCanRenameNode;

	UPROPERTY(BlueprintReadOnly, config, Category = "Graph Rule")
	bool bCanBeCyclical;
};

