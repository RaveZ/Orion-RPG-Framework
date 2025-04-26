// Copyright 2024 Ivan Chandra. All Rights Reserved.


#include "Condition/QuestCondition.h"
#include "UObject/UObjectIterator.h"
#include "QuestComponent.h"
#include "QuestSystem.h"
#include "QuestSystemGraph.h"
#include "QuestSystemGraphNode.h"
#include "QuestSystemGraphNode_Root.h"

#define LOCTEXT_NAMESPACE "QuestCondition"

bool UQuestCondition::IsConditionMet_Implementation() const
{
	return true;
}

FString UQuestCondition::GetNodeDisplayText_Implementation() const
{
	return GetName();
}

AController* UQuestCondition::GetOwningController() const
{
	if (!QuestGraph)
		return nullptr;

	return QuestGraph->OwningController;
}

APawn* UQuestCondition::GetOwningPawn() const
{
	if (!QuestGraph)
		return nullptr;

	return QuestGraph->OwningPawn;
}

UQuestComponent* UQuestCondition::GetQuestComponent() const
{
	if (!QuestGraph)
		return nullptr;

	return QuestGraph->QuestComponent;
}

UQuestSystemGraph* UQuestCondition::GetOwningQuestGraph() const
{
	if (!QuestGraph)
		return nullptr;

	return QuestGraph;
}

UQuestSystem* UQuestCondition::GetOwningQuestSystem() const
{
	return QuestSystem;
}

TArray<FName> UQuestCondition::GetAllQuestID()
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

TArray<FName> UQuestCondition::GetAllNodeID()
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
void UQuestCondition::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);
	if (PropertyChangedEvent.Property)
	{
		FName PropertyName = PropertyChangedEvent.Property->GetFName();
		if (PropertyName == GET_MEMBER_NAME_CHECKED(UQuestCondition, QuestID))
		{
			NodeID = NAME_None;
		}
	}
}
#endif
#undef LOCTEXT_NAMESPACE