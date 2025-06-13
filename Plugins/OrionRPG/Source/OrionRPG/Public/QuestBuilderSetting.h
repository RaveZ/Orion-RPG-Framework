// Copyright 2025 Ivan Chandra. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "QuestBuilderSetting.generated.h"



UCLASS(config = Engine, defaultconfig)
class ORIONRPG_API UQuestBuilderSetting : public UObject
{
	GENERATED_BODY()

	UQuestBuilderSetting();
	virtual ~UQuestBuilderSetting();

public:
	//Runtime Config
	UPROPERTY(EditAnywhere, config, Category = "Quest Config")
	FText DefaultQuestCategory;
	
	//Graph Setting
	UPROPERTY(EditAnywhere, config, Category = "Graph Style")
	FLinearColor FailedNodeColor;

	UPROPERTY(EditAnywhere, config, Category = "Graph Style")
	FLinearColor SuccessNodeColor;

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

