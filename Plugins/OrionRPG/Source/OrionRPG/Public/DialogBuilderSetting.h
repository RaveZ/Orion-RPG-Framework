// Copyright 2025 Ivan Chandra. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "DialogBuilderSetting.generated.h"



UCLASS(config = Engine, defaultconfig)
class ORIONRPG_API UDialogBuilderSetting : public UObject
{
	GENERATED_BODY()

	UDialogBuilderSetting();
	virtual ~UDialogBuilderSetting();

public:

	UPROPERTY(EditAnywhere, config, Category = "Graph Style")
	FLinearColor DialogLineNodeColor;

	UPROPERTY(EditAnywhere, config, Category = "Graph Style")
	FLinearColor StateNodeColor;

	UPROPERTY(EditAnywhere, config, Category = "Graph Style")
	FLinearColor RootNodeColor;
	
	UPROPERTY(EditAnywhere, config, Category = "Graph Style")
	FLinearColor EndDialogNodeColor;

	UPROPERTY(EditAnywhere, config, Category = "Graph Style")
	FLinearColor RerouteNodeColor;

	UPROPERTY(EditAnywhere, config, Category = "Graph Style")
	FLinearColor PlayerOptionNodeColor;

	UPROPERTY(EditAnywhere, config, Category = "Graph Style")
	FLinearColor SubNodeColor;

	UPROPERTY(BlueprintReadOnly, config, Category = "Graph Rule")
	bool bEdgeEnabled;

	UPROPERTY(BlueprintReadOnly, config, Category = "Graph Rule")
	bool bCanRenameNode;

	UPROPERTY(BlueprintReadOnly, config, Category = "Graph Rule")
	bool bCanBeCyclical;
	



};

