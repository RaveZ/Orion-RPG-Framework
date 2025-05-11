// Copyright 2024 Ivan Chandra. All Rights Reserved.


#include "QuestBuilderEdNode.h"
#include "QuestBuilderSetting.h"
#include "QuestBuilderNode.h"
#include "QuestBuilderNode_Root.h"
#include "QuestBuilderNode_Objective.h"
#include "QuestBuilderNode_State.h"
#include "QuestBuilderEditorUtils.h"

UQuestBuilderEdNode::UQuestBuilderEdNode()
{
	bIsReadOnly = false;
	bCanRenameNode = true;
}

UQuestBuilderEdNode::~UQuestBuilderEdNode()
{
}


UQuestBuilderEdGraph* UQuestBuilderEdNode::GetQuestBuilderEdGraph()
{
	return Cast<UQuestBuilderEdGraph>(GetGraph());
}



void UQuestBuilderEdNode::AllocateDefaultPins()
{
	CreatePin(EGPD_Input, "MultipleNodes", FName(), TEXT("In"));
	CreatePin(EGPD_Output, "MultipleNodes", FName(), TEXT("Out"));
}

FText UQuestBuilderEdNode::GetNodeTitle(ENodeTitleType::Type TitleType) const
{
	if (NodeInstance == nullptr)
	{
		return Super::GetNodeTitle(TitleType);
	}
	else
	{
		const UQuestBuilderNode* QuestNode = Cast<UQuestBuilderNode>(NodeInstance);
		return QuestNode->GetNodeTitle();
	}
}

bool UQuestBuilderEdNode::CanDuplicateNode() const
{
	return bIsReadOnly ? false : Super::CanDuplicateNode();
}

void UQuestBuilderEdNode::PostPlacedNewNode()
{
	// NodeInstance can be already spawned by paste operation, don't override it

	UClass* NodeClass = ClassData.GetClass(true);
	if (NodeClass)
	{
		UEdGraph* MyGraph = GetGraph();
		UQuestBuilderEdGraph* QuestEdGraph = Cast<UQuestBuilderEdGraph>(GetGraph());
		UObject* GraphOwner = MyGraph ? MyGraph->GetOuter() : nullptr;
		if (GraphOwner)
		{
			if (NodeInstance == nullptr)
			{
				NodeInstance = NewObject<UObject>(GraphOwner, NodeClass);
				NodeInstance->SetFlags(RF_Transactional);
			}
			InitializeInstance();

			if (NodeInstance)
			{
				if (UQuestBuilderNode_Objective* ObjectiveQuestNode = Cast<UQuestBuilderNode_Objective>(NodeInstance))
				{
					ObjectiveQuestNode->ID = FQuestBuilderEditorUtils::FindUniqueNodeName("ObjectiveNode");
				}
				if (UQuestBuilderNode_Root* RootQuestNode = Cast<UQuestBuilderNode_Root>(NodeInstance))
				{
					RootQuestNode->ID = FQuestBuilderEditorUtils::FindUniqueNodeName("RootNode");
				}
				if (UQuestBuilderNode_State* StateQuestNode = Cast<UQuestBuilderNode_State>(NodeInstance))
				{
					StateQuestNode->ID = FQuestBuilderEditorUtils::FindUniqueNodeName("StateNode");
				}
			}
			UQuestBuilderNode* QuestNode = NodeInstance ?  Cast<UQuestBuilderNode>(NodeInstance) : nullptr;
			if (QuestNode)
			{
				QuestEdGraph->AddToNodeIDMap(QuestNode->ID, QuestNode);
			}
		}
	}
}

void UQuestBuilderEdNode::InitializeInstance()
{
	UQuestBuilderNode* QuestNode = Cast<UQuestBuilderNode>(NodeInstance);
	UQuestBuilderGraph* QuestAsset = QuestNode ? Cast<UQuestBuilderGraph>(QuestNode->GetOuter()) : nullptr;
	if (QuestNode && QuestAsset)
	{
		QuestNode->InitializeFromAsset(*QuestAsset);
	}
}

bool UQuestBuilderEdNode::CanUserDeleteNode() const
{
	return bIsReadOnly ? false : Super::CanUserDeleteNode();
}

void UQuestBuilderEdNode::PrepareForCopying()
{
	UQuestBuilderNode* QuestNode = Cast<UQuestBuilderNode>(NodeInstance);
	if (QuestNode)
	{
		QuestNode->Rename(nullptr, this, REN_DontCreateRedirectors | REN_DoNotDirty);

	}
}

void UQuestBuilderEdNode::AutowireNewNode(UEdGraphPin* FromPin)
{
	Super::AutowireNewNode(FromPin);

	if (FromPin != nullptr) {
		if (GetSchema()->TryCreateConnection(FromPin, GetInputPin()))
		{
			FromPin->GetOwningNode()->NodeConnectionListChanged();
		}
	}
}

FText UQuestBuilderEdNode::GetDescription() const
{
	UQuestBuilderNode* QuestNode = Cast<UQuestBuilderNode>(NodeInstance);
	if (QuestNode)
	{
		return QuestNode->GetNodeDescription();
	}
	return FText::GetEmpty();
}

FText UQuestBuilderEdNode::GetEventsText() const
{
	UQuestBuilderNode* QuestNode = Cast<UQuestBuilderNode>(NodeInstance);
	if (QuestNode)
	{
		return QuestNode->GetEventsText();
	}
	return FText::GetEmpty();
}

FText UQuestBuilderEdNode::GetConditionsText() const
{
	UQuestBuilderNode* QuestNode = Cast<UQuestBuilderNode>(NodeInstance);
	if (QuestNode)
	{
		return QuestNode->GetConditionsText();
	}
	return FText::GetEmpty();
}

FLinearColor UQuestBuilderEdNode::GetBackgroundColor() const
{
	return GetDefault<UQuestBuilderSetting>()->RootNodeColor;
}

UEdGraphPin* UQuestBuilderEdNode::GetInputPin() const
{
	//return Pins[0];
	for (int32 PinIndex = 0; PinIndex < Pins.Num(); PinIndex++)
	{
		if (Pins[PinIndex]->Direction == EGPD_Input)
		{
			return Pins[PinIndex];
		}
	}

	return nullptr;
}

UEdGraphPin* UQuestBuilderEdNode::GetOutputPin() const
{
	//return Pins[1];
	for (int32 PinIndex = 0; PinIndex < Pins.Num(); PinIndex++)
	{
		if (Pins[PinIndex]->Direction == EGPD_Output)
		{
			return Pins[PinIndex];
		}
	}

	return nullptr;
}

void UQuestBuilderEdNode::GetNodeContextMenuActions(UToolMenu* Menu, UGraphNodeContextMenuContext* Context) const
{
}



bool UQuestBuilderEdNode::UsesBlueprint() const
{
	UQuestBuilderNode* QuestNode = Cast<UQuestBuilderNode>(NodeInstance);
	if (QuestNode)
	{
		return QuestNode && QuestNode->GetClass()->HasAnyClassFlags(CLASS_CompiledFromBlueprint);
	}
	return false;
}

#if WITH_EDITOR
void UQuestBuilderEdNode::PostEditUndo()
{
	UEdGraphNode::PostEditUndo();
}

void UQuestBuilderEdNode::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);

	
}
#endif