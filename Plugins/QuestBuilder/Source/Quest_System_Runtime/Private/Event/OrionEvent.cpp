// Copyright 2025 Ivan Chandra. All Rights Reserved.

#include "Event/OrionEvent.h"
#include "UObject/UObjectIterator.h"
#include "Quest_System_Runtime.h"
#include "QuestComponent.h"
#include "Quest.h"
#include "QuestBuilderGraph.h"
#include "QuestBuilderNode.h"
#include "QuestBuilderNode_Objective.h"
#include "QuestBuilderNode_Root.h"

UOrionEvent::UOrionEvent()
{
}

void UOrionEvent::BeginEvent_Implementation(AController* OwnerController, APawn* ControlledPawn)
{
}

FString UOrionEvent::GetNodeDisplayText_Implementation() const
{
	return GetName();
}

AController* UOrionEvent::GetOwningController() const
{
	if (!QuestGraph)
		return nullptr;

	return QuestGraph->OwningController;
}

APawn* UOrionEvent::GetOwningPawn() const
{
	if (!QuestGraph)
		return nullptr;

	return QuestGraph->OwningPawn;
}

UQuestComponent* UOrionEvent::GetQuestComponent() const
{
	if (!QuestGraph)
		return nullptr;

	return QuestGraph->QuestComponent;
}

UQuestBuilderGraph* UOrionEvent::GetOwningQuestGraph() const
{
	if (!QuestGraph)
		return nullptr;

	return QuestGraph;
}

UQuest* UOrionEvent::GetOwningQuest() const 
{
	return Quest;
}

TArray<FName> UOrionEvent::GetAllQuestID()
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

TArray<FName> UOrionEvent::GetAllNodeID()
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

FString UOrionEvent::GetNodeName()
{
	return NodeName.Len() ? NodeName : GetShortTypeName(this);
}

FString UOrionEvent::GetShortTypeName(UObject* Ob)
{
	if ((Ob == nullptr) || (Ob->GetClass() == nullptr))
	{
		return TEXT("None");
	}

	FString TypeDesc = Ob->GetClass()->GetName();

	if (Ob->GetClass()->HasAnyClassFlags(CLASS_CompiledFromBlueprint))
	{

		TypeDesc = Ob->GetClass()->GetName().LeftChop(2);
	}

	// Insert space before each capital letter (except the first character) and remove underscores
	FString Result;
	for (int32 i = 0; i < TypeDesc.Len(); ++i)
	{
		if (TypeDesc[i] != TEXT('_'))
		{
			TCHAR Char = TypeDesc[i];
			if (i > 0 && FChar::IsUpper(Char) && !FChar::IsWhitespace(TypeDesc[i - 1]))
			{
				Result += TEXT(" ");
			}
			Result += Char;
		}
	}

	return Result;
}

UWorld* UOrionEvent::GetWorld() const
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
void UOrionEvent::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);
	if (PropertyChangedEvent.Property)
	{
		FName PropertyName = PropertyChangedEvent.Property->GetFName();
		if (PropertyName == GET_MEMBER_NAME_CHECKED(UOrionEvent, QuestID))
		{
			NodeID = NAME_None;
		}
	}
	
}
#endif
