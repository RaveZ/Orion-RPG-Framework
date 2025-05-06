// Copyright 2024 Ivan Chandra. All Rights Reserved.


#include "Condition/QuestCondition.h"
#include "UObject/UObjectIterator.h"
#include "QuestComponent.h"
#include "Quest.h"
#include "QuestBuilderGraph.h"
#include "QuestBuilderNode.h"
#include "QuestBuilderNode_Root.h"

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

UQuestBuilderGraph* UQuestCondition::GetOwningQuestGraph() const
{
	if (!QuestGraph)
		return nullptr;

	return QuestGraph;
}

UQuest* UQuestCondition::GetOwningQuestSystem() const
{
	return QuestSystem;
}

TArray<FName> UQuestCondition::GetAllQuestID()
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

TArray<FName> UQuestCondition::GetAllNodeID()
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