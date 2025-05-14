// Copyright 2025 Ivan Chandra. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "QuestBuilderSetting.generated.h"



UCLASS(config = Engine, defaultconfig)
class QUEST_SYSTEM_EDITOR_API UQuestBuilderSetting : public UObject
{
	GENERATED_BODY()

	UQuestBuilderSetting();
	virtual ~UQuestBuilderSetting();

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

	UPROPERTY(BlueprintReadOnly, config, Category = "Graph Rule")
	bool bEdgeEnabled;

	UPROPERTY(BlueprintReadOnly, config, Category = "Graph Rule")
	bool bCanRenameNode;

	UPROPERTY(BlueprintReadOnly, config, Category = "Graph Rule")
	bool bCanBeCyclical;
};

