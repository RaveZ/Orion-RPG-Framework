// Copyright 2025 Ivan Chandra. All Rights Reserved.


#include "QuestBuilderNode.h"
#include "QuestBuilderEdge.h"
#include "Quest.h"
#include "Event/OrionEvent.h"
#include "Decorator/OrionDecorator.h"
#include "QuestBuilderGraph.h"

#define LOCTEXT_NAMESPACE "QuestBuilderNode"

UQuestBuilderNode::UQuestBuilderNode()
{
#if WITH_EDITORONLY_DATA
	CompatibleGraphType = UQuestBuilderGraph::StaticClass();

	FLinearColor Default(0.05f, 0.05f, 0.05f);
	BackgroundColor = Default;
#endif
}

UQuestBuilderNode::~UQuestBuilderNode()
{
}

UQuestBuilderEdge* UQuestBuilderNode::GetEdge(UQuestBuilderNode* ChildNode)
{
	return Edges.Contains(ChildNode) ? Edges.FindChecked(ChildNode) : nullptr;
}


FText UQuestBuilderNode::GetDescription_Implementation() const
{
	return Description;
}

void UQuestBuilderNode::BeginEvents(EEventRunType EventRunType)
{
	//End of a Node, Launch Event with the EventRunType param and Both Type
	for (auto& Event : Events)
	{
		if (Event)
		{
			if (Event->EventRunType == EEventRunType::E_Both || Event->EventRunType == EventRunType)
			{
				Event->BeginEvent(GetOwningController(), GetOwningPawn());
			}
		}
	}
}


FString UQuestBuilderNode::GetShortTypeName(const UObject* Ob)
{
	if ((Ob == nullptr) || (Ob->GetClass() == nullptr))
	{
		return TEXT("None");
	}

	if (Ob->GetClass()->HasAnyClassFlags(CLASS_CompiledFromBlueprint))
	{
		return Ob->GetClass()->GetName().LeftChop(2);
	}

	FString TypeDesc = Ob->GetClass()->GetName();
	const int32 ShortNameIdx = TypeDesc.Find(TEXT("_"), ESearchCase::CaseSensitive);
	if (ShortNameIdx != INDEX_NONE)
	{
		TypeDesc.MidInline(ShortNameIdx + 1, MAX_int32, EAllowShrinking::No);
	}

	return TypeDesc;
}

void UQuestBuilderNode::EvaluateUniqueID()
{
	if (UQuest* OwningQuest = GetOwningQuest())
	{
		TArray<UQuestBuilderNode*> QuestNodes = OwningQuest->AllNodes;
		TArray<FName> NodeIDs;

		for (auto& Node : QuestNodes)
		{
			if (Node != this)
			{
				NodeIDs.Add(Node->ID);
			}
		}

		int32 FinalSuffix = 1;
		FName NewID = ID;
		FString BaseName = NewID.ToString();
		int32 UnderscoreIndex;

		if (BaseName.FindLastChar(TEXT('_'), UnderscoreIndex))
		{
			FString Suffix = BaseName.Mid(UnderscoreIndex + 1);
			if (Suffix.IsNumeric())
			{
				BaseName = BaseName.Left(UnderscoreIndex);
			}
		}

		if (!NodeIDs.Contains(NewID))
		{
			return;
		}

		// Check if the new ID already exists in the array
		while (NodeIDs.Contains(NewID))
		{
			// If it does, add a numeric suffix and try again
			NewID = FName(*FString::Printf(TEXT("%s_%d"), *BaseName, FinalSuffix));
			FinalSuffix++;
		}

		ID = NewID;
	}
}




#if WITH_EDITOR
bool UQuestBuilderNode::IsNameEditable() const
{
	return true;
}

FLinearColor UQuestBuilderNode::GetBackgroundColor() const
{
	return BackgroundColor;
}

FText UQuestBuilderNode::GetNodeTitle() const
{
	return FText::FromString(UQuestBuilderNode::GetShortTypeName(this));
}

FText UQuestBuilderNode::GetNodeDescription() const
{
	return FText::FromString(TEXT("This is NodeBase"));
}

FText UQuestBuilderNode::GetEventsText() const
{
	FString EventListText;

	for (const UOrionEvent* Event : Events)
	{
		if (Event)
		{
			if(!EventListText.IsEmpty()) EventListText += TEXT("\n");
			EventListText += Event->GetNodeDisplayText();
		}

	}

	return FText::FromString(EventListText);
}

FText UQuestBuilderNode::GetConditionsText() const
{
	FString ConditionListText;

	for (const UOrionDecorator* Condition : Decorators)
	{
		if (Condition)
		{
			if(!ConditionListText.IsEmpty()) ConditionListText += TEXT("\n");

			ConditionListText += Condition->GetNodeDisplayText();
		}

	}

	return FText::FromString(ConditionListText);
}

void UQuestBuilderNode::SetNodeTitle(const FText& NewTitle)
{
}

bool UQuestBuilderNode::CanCreateConnection(UQuestBuilderNode* Other, FText& ErrorMessage)
{
	return true;
}

bool UQuestBuilderNode::CanCreateConnectionTo(UQuestBuilderNode* Other, int32 NumberOfChildrenNodes, FText& ErrorMessage)
{
	if (ChildrenLimitType == EDialogNodeLimits::Limited && NumberOfChildrenNodes >= ChildrenLimit)
	{
		ErrorMessage = FText::FromString("Children limit exceeded");
		return false;
	}

	return CanCreateConnection(Other, ErrorMessage);
	
}

bool UQuestBuilderNode::CanCreateConnectionFrom(UQuestBuilderNode* Other, int32 NumberOfParentNodes, FText& ErrorMessage)
{
	if (ParentLimitType == EDialogNodeLimits::Limited && NumberOfParentNodes >= ParentLimit)
	{
		ErrorMessage = FText::FromString("Parent limit exceeded");
		return false;
	}

	return true;
}

void UQuestBuilderNode::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);

	if (PropertyChangedEvent.MemberProperty)
	{
		//If we changed the ID, make sure it doesn't conflict with any other IDs in the quest
		if (PropertyChangedEvent.MemberProperty->GetFName() == GET_MEMBER_NAME_CHECKED(UQuestBuilderNode, ID))
		{
			EvaluateUniqueID();
		}
	}
}


#endif

bool UQuestBuilderNode::IsLeafNode() const
{
	return ChildrenNodes.Num() == 0;
}

void UQuestBuilderNode::Initialize()
{
	//empty, override in child class
}

void UQuestBuilderNode::Deinitialize()
{
	//empty, override in child class
}

void UQuestBuilderNode::InitializeFromAsset(UQuestBuilderGraph& OwningQuestGraph, UQuest& OwningQuest)
{
	QuestGraph = &OwningQuestGraph;
	Quest = &OwningQuest;
	Quest->AllNodes.Add(this);
	Quest->NodeMap.Emplace(ID, this);
}


UQuestBuilderGraph* UQuestBuilderNode::GetOwningQuestGraph() const
{
	if (!QuestGraph)
		return nullptr;

	return QuestGraph;
}

UQuest* UQuestBuilderNode::GetOwningQuest() const
{
	return Quest;
}


AController* UQuestBuilderNode::GetOwningController() const
{
	if (!QuestGraph)
		return nullptr;
	

	return QuestGraph->OwningController;
}

APawn* UQuestBuilderNode::GetOwningPawn() const
{
	if (!QuestGraph)
		return nullptr;

	return QuestGraph->OwningPawn;
}

UQuestComponent* UQuestBuilderNode::GetQuestComponent() const
{
	if (!QuestGraph)
		return nullptr;

	return QuestGraph->QuestComponent;
}

#undef LOCTEXT_NAMESPACE


