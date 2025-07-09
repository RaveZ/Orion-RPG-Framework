// Copyright 2025 Ivan Chandra. All Rights Reserved.


#include "Event/BeginQuestGraph.h"
#include "Quest.h"
#include "QuestBuilderFunctionLibrary.h"
#include "QuestComponent.h"
#include "QuestBuilderGraph.h"
#include "QuestBuilderNode.h"

UBeginQuestGraph::UBeginQuestGraph()
{
}

void UBeginQuestGraph::BeginEvent(AController* OwnerController, APawn* ControlledPawn)
{
	if (GetWorld())
	{
		UQuestComponent* QuestComp = UQuestBuilderFunctionLibrary::GetQuestComponent(GetWorld());
		if (QuestComp)
		{
			QuestComp->BeginQuestGraph(QuestAsset);
		}
	}
	EndEvent();
	
	
}

FString UBeginQuestGraph::GetNodeDisplayText_Implementation() const
{
	if (QuestAsset)
	{
		return FString::Printf(TEXT("Begin Quest Graph :  %s"), *QuestAsset->GetName());
	}
	else
	{
		return FString("Begin Quest Graph");
	}
}
