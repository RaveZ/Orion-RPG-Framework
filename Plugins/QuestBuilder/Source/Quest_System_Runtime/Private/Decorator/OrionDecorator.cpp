// Copyright 2024 Ivan Chandra. All Rights Reserved.


#include "Decorator/OrionDecorator.h"
#include "UObject/UObjectIterator.h"
#include "QuestComponent.h"
#include "Quest.h"
#include "QuestBuilderGraph.h"
#include "QuestBuilderNode.h"
#include "QuestBuilderNode_Root.h"

#define LOCTEXT_NAMESPACE "QuestCondition"

bool UOrionDecorator::PerformConditionCheck_Implementation(AController* OwnerController, APawn* ControlledPawn) const
{
	return true;
}

FString UOrionDecorator::GetNodeDisplayText_Implementation() const
{
	return GetName();
}

AController* UOrionDecorator::GetOwningController() const
{
	if (!QuestGraph)
		return nullptr;

	return QuestGraph->OwningController;
}

APawn* UOrionDecorator::GetOwningPawn() const
{
	if (!QuestGraph)
		return nullptr;

	return QuestGraph->OwningPawn;
}

UQuestComponent* UOrionDecorator::GetQuestComponent() const
{
	if (!QuestGraph)
		return nullptr;

	return QuestGraph->QuestComponent;
}

UQuestBuilderGraph* UOrionDecorator::GetOwningQuestGraph() const
{
	if (!QuestGraph)
		return nullptr;

	return QuestGraph;
}

UQuest* UOrionDecorator::GetOwningQuest() const
{
	return Quest;
}

TArray<FName> UOrionDecorator::GetAllQuestID()
{
	TArray<FName> QuestIDs;
	for (TObjectIterator<UQuestBuilderGraph> It; It; ++It)
	{
		for (UQuest* QuestObj : It->QuestList)
		{
			QuestIDs.Add(QuestObj->ID);
		}
	}

	return QuestIDs;
}

TArray<FName> UOrionDecorator::GetAllNodeID()
{
	TArray<FName> NodeIDs;
	if (bUseQuestID)
	{
		for (TObjectIterator<UQuestBuilderGraph> It; It; ++It)
		{
			for (UQuest* QuestObj : It->QuestList)
			{
				if (QuestObj)
				{
					if (QuestObj->ID != QuestID) continue;
					for (UQuestBuilderNode* Node : QuestObj->AllNodes)
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

	}
	else
	{
		if (GetOwningQuest())
		{
			for (UQuestBuilderNode* Node : GetOwningQuest()->AllNodes)
			{
				if (Node)
				{
					if (UQuestBuilderNode_Root* RootNode = Cast<UQuestBuilderNode_Root>(Node)) continue;
					NodeIDs.Emplace(Node->ID);
				}
			}
		}
	}
	return NodeIDs;
}

UWorld* UOrionDecorator::GetWorld() const
{
	if (HasAllFlags(RF_ClassDefaultObject))
	{
		// If we are a CDO, we must return nullptr instead of calling Outer->GetWorld() to fool UObject::ImplementsGetWorld.
		return nullptr;
	}

	if (GetQuestComponent())
	{
		return GetQuestComponent()->GetWorld();
	}

	return nullptr;
}

#if WITH_EDITOR
void UOrionDecorator::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);
	if (PropertyChangedEvent.Property)
	{
		FName PropertyName = PropertyChangedEvent.Property->GetFName();
		if (PropertyName == GET_MEMBER_NAME_CHECKED(UOrionDecorator, QuestID))
		{
			NodeID = NAME_None;
		}
	}
}
#endif
#undef LOCTEXT_NAMESPACE