// Copyright 2024 Ivan Chandra. All Rights Reserved.

#include "Event/QuestEvent.h"
#include "UObject/UObjectIterator.h"
#include "Quest_System_Runtime.h"
#include "QuestComponent.h"
#include "QuestSystem.h"
#include "QuestSystemGraph.h"
#include "QuestSystemGraphNode.h"
#include "QuestSystemGraphNode_Objective.h"
#include "QuestSystemGraphNode_Root.h"

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

UQuestSystemGraph* UQuestEvent::GetOwningQuestGraph() const
{
	if (!QuestGraph)
		return nullptr;

	return QuestGraph;
}

UQuestSystem* UQuestEvent::GetOwningQuestSystem() const 
{
	return QuestSystem;
}

TArray<FName> UQuestEvent::GetAllQuestID()
{
	TArray<FName> QuestIDs;
	for (TObjectIterator<UQuestSystemGraph> It; It; ++It)
	{
		for (UQuestSystem* Quest : It->QuestSystems)
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
		for (TObjectIterator<UQuestSystemGraph> It; It; ++It)
		{
			for (UQuestSystem* Quest : It->QuestSystems)
			{
				if (Quest)
				{
					if (Quest->QuestID != QuestID) continue;
					for (UQuestSystemGraphNode* Node : Quest->AllNodes)
					{
						if (Node)
						{
							if (UQuestSystemGraphNode_Root* RootNode = Cast<UQuestSystemGraphNode_Root>(Node)) continue;
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
			for (UQuestSystemGraphNode* Node : GetOwningQuestSystem()->AllNodes)
			{
				if (Node)
				{
					if (UQuestSystemGraphNode_Root* RootNode = Cast<UQuestSystemGraphNode_Root>(Node)) continue;
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
