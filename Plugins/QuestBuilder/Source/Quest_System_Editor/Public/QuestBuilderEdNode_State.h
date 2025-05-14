// Copyright 2025 Ivan Chandra. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "QuestBuilderEdNode.h"
#include "EdGraph/EdGraphNode.h"
#include "QuestBuilderEdNode_State.generated.h"

/**
 * 
 */
UCLASS()
class QUEST_SYSTEM_EDITOR_API UQuestBuilderEdNode_State : public UQuestBuilderEdNode
{
	GENERATED_BODY()
public:
	UQuestBuilderEdNode_State();
	virtual void AllocateDefaultPins() override;
	virtual FText GetNodeTitle(ENodeTitleType::Type TitleType) const override;
	virtual FLinearColor GetBackgroundColor() const override;
};
