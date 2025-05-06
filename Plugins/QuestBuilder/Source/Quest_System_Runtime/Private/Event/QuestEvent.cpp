// Copyright 2024 Ivan Chandra. All Rights Reserved.

#include "Event/QuestEvent.h"
#include "UObject/UObjectIterator.h"
#include "Quest_System_Runtime.h"
#include "QuestComponent.h"
#include "Quest.h"
#include "QuestBuilderGraph.h"
#include "QuestBuilderNode.h"
#include "QuestBuilderNode_Objective.h"
#include "QuestBuilderNode_Root.h"

UQuestEvent::UQuestEvent()
{
}

void UQuestEvent::BeginEvent_Implementation()
{
}

FString UQuestEvent::GetNodeDisplayText_Implementation() const
{
	return GetName();
}

AController* UQuestEvent::GetOwningController() const
{
	if (!QuestGraph)
		return nullptr;

	return QuestGraph->OwningController;
}

APawn* UQuestEvent::GetOwningPawn() const
{
	if (!QuestGraph)
		return nullptr;

	return QuestGraph->OwningPawn;
}

UQuestComponent* UQuestEvent::GetQuestComponent() const
{
	if (!QuestGraph)
		return nullptr;

	return QuestGraph->QuestComponent;
}

UQuestBuilderGraph* UQuestEvent::GetOwningQuestGraph() const
{
	if (!QuestGraph)
		return nullptr;

	return QuestGraph;
}

UQuest* UQuestEvent::GetOwningQuestSystem() const 
{
	return QuestSystem;
}

TArray<FName> UQuestEvent::GetAllQuestID()
{
	TArray<FName> QuestIDs;
	for (TObjectIterator<UQuestBuilderGraph> It; It; ++It)
	{
		for (UQuest* Quest : It->QuestSystems)
		{
			QuestIDs.Add(Quest->QuestID);
		}
	}

	return QuestIDs;
}

TArray<FName> UQuestEvent::GetAllNodeID()
{
	TArray<FName> NodeIDs;
	if (bUseQuestID)
	{
		for (TObjectIterator<UQuestBuilderGraph> It; It; ++It)
		{
			for (UQuest* Quest : It->QuestSystems)
			{
				if (Quest)
				{
					if (Quest->QuestID != QuestID) continue;
					for (UQuestBuilderNode* Node : Quest->AllNodes)
					{
						if (Node)
						{
							if (UQuestBuilderNode_Root* RootNode = Cast<UQuestBuilderNode_Root>(Node)) continue;
							NodeIDs.Emplace(Node->ID);
						}
					}
				}

			}

		}

		return NodeIDs;
	}
	else
	{
		if (GetOwningQuestSystem())
		{
			for (UQuestBuilderNode* Node : GetOwningQuestSystem()->AllNodes)
			{
				if (Node)
				{
					if (UQuestBuilderNode_Root* RootNode = Cast<UQuestBuilderNode_Root>(Node)) continue;
					NodeIDs.Emplace(Node->ID);
				}
			}
		}
		return NodeIDs;
	}
	
}

#if WITH_EDITOR
void UQuestEvent::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);
	if (PropertyChangedEvent.Property)
	{
		FName PropertyName = PropertyChangedEvent.Property->GetFName();
		if (PropertyName == GET_MEMBER_NAME_CHECKED(UQuestEvent, QuestID))
		{
			NodeID = NAME_None;
		}
	}
	
}
#endif
