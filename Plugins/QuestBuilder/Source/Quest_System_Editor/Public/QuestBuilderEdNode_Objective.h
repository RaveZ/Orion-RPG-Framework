// Copyright 2025 Ivan Chandra. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "QuestBuilderEdNode.h"
#include "QuestBuilderEdNode_Objective.generated.h"

/**
 * 
 */
UCLASS()
class QUEST_SYSTEM_EDITOR_API UQuestBuilderEdNode_Objective : public UQuestBuilderEdNode
{
	GENERATED_BODY()
public:
	virtual FText GetNodeTitle(ENodeTitleType::Type TitleType) const override;
	virtual FLinearColor GetBackgroundColor() const override;
};
