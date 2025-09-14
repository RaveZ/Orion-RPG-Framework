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

	/**Show the common UI bound action*/
	UPROPERTY(EditAnywhere, BlueprintReadOnly, config, Category = "Quest Options")
	bool ShowUIBoundAction;

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

	UPROPERTY(config)
	bool bEdgeEnabled;

	UPROPERTY(config)
	bool bCanRenameNode;

	UPROPERTY(config)
	bool bCanBeCyclical;

};

