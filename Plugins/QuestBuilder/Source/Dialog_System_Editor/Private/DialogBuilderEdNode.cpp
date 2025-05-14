// Copyright 2024 Ivan Chandra. All Rights Reserved.


#include "DialogBuilderEdNode.h"
#include "DialogBuilderSetting.h"
#include "QuestBuilderNode.h"
#include "Quest.h"
#include "QuestBuilderNode_Root.h"
#include "QuestBuilderNode_Objective.h"
#include "QuestBuilderNode_State.h"
#include "DialogBuilderEditorUtils.h"

UDialogBuilderEdNode::UDialogBuilderEdNode()
{
	bIsReadOnly = false;
	bCanRenameNode = true;
}

UDialogBuilderEdNode::~UDialogBuilderEdNode()
{
}


UDialogBuilderEdGraph* UDialogBuilderEdNode::GetQuestBuilderEdGraph()
{
	return Cast<UDialogBuilderEdGraph>(GetGraph());
}



void UDialogBuilderEdNode::AllocateDefaultPins()
{
	CreatePin(EGPD_Input, "MultipleNodes", FName(), TEXT("In"));
	CreatePin(EGPD_Output, "MultipleNodes", FName(), TEXT("Out"));
}

FText UDialogBuilderEdNode::GetNodeTitle(ENodeTitleType::Type TitleType) const
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

bool UDialogBuilderEdNode::CanDuplicateNode() const
{
	return bIsReadOnly ? false : Super::CanDuplicateNode();
}

void UDialogBuilderEdNode::PostPlacedNewNode()
{
	// NodeInstance can be already spawned by paste operation, don't override it

	UClass* NodeClass = ClassData.GetClass(true);
	if (NodeClass)
	{
		UEdGraph* MyGraph = GetGraph();
		UDialogBuilderEdGraph* QuestEdGraph = Cast<UDialogBuilderEdGraph>(GetGraph());
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
					ObjectiveQuestNode->ID = FDialogBuilderEditorUtils::FindUniqueNodeName("ObjectiveNode");
				}
				if (UQuestBuilderNode_State* StateQuestNode = Cast<UQuestBuilderNode_State>(NodeInstance))
				{
					StateQuestNode->ID = FDialogBuilderEditorUtils::FindUniqueNodeName("StateNode");
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

void UDialogBuilderEdNode::InitializeInstance()
{
	UQuestBuilderNode* QuestNode = Cast<UQuestBuilderNode>(NodeInstance);
	UQuestBuilderGraph* QuestAsset = QuestNode ? Cast<UQuestBuilderGraph>(QuestNode->GetOuter()) : nullptr;
	UQuest* Dialog = GetQuestBuilderEdGraph() ? GetQuestBuilderEdGraph()->Dialog : nullptr;
	if (Dialog && QuestNode && QuestAsset)
	{
		QuestNode->InitializeFromAsset(*QuestAsset, *Dialog);

	}
}

bool UDialogBuilderEdNode::CanUserDeleteNode() const
{
	return bIsReadOnly ? false : Super::CanUserDeleteNode();
}

void UDialogBuilderEdNode::PrepareForCopying()
{
	UQuestBuilderNode* QuestNode = Cast<UQuestBuilderNode>(NodeInstance);
	if (QuestNode)
	{
		QuestNode->Rename(nullptr, this, REN_DontCreateRedirectors | REN_DoNotDirty);

	}
}

void UDialogBuilderEdNode::AutowireNewNode(UEdGraphPin* FromPin)
{
	Super::AutowireNewNode(FromPin);

	if (FromPin != nullptr) {
		if (GetSchema()->TryCreateConnection(FromPin, GetInputPin()))
		{
			FromPin->GetOwningNode()->NodeConnectionListChanged();
		}
	}
}

FText UDialogBuilderEdNode::GetDescription() const
{
	UQuestBuilderNode* QuestNode = Cast<UQuestBuilderNode>(NodeInstance);
	if (QuestNode)
	{
		return QuestNode->GetNodeDescription();
	}
	return FText::GetEmpty();
}

FText UDialogBuilderEdNode::GetEventsText() const
{
	UQuestBuilderNode* QuestNode = Cast<UQuestBuilderNode>(NodeInstance);
	if (QuestNode)
	{
		return QuestNode->GetEventsText();
	}
	return FText::GetEmpty();
}

FText UDialogBuilderEdNode::GetConditionsText() const
{
	UQuestBuilderNode* QuestNode = Cast<UQuestBuilderNode>(NodeInstance);
	if (QuestNode)
	{
		return QuestNode->GetConditionsText();
	}
	return FText::GetEmpty();
}

FLinearColor UDialogBuilderEdNode::GetBackgroundColor() const
{
	return GetDefault<UDialogBuilderSetting>()->RootNodeColor;
}

UEdGraphPin* UDialogBuilderEdNode::GetInputPin() const
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

UEdGraphPin* UDialogBuilderEdNode::GetOutputPin() const
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

void UDialogBuilderEdNode::GetNodeContextMenuActions(UToolMenu* Menu, UGraphNodeContextMenuContext* Context) const
{
}



bool UDialogBuilderEdNode::UsesBlueprint() const
{
	UQuestBuilderNode* QuestNode = Cast<UQuestBuilderNode>(NodeInstance);
	if (QuestNode)
	{
		return QuestNode && QuestNode->GetClass()->HasAnyClassFlags(CLASS_CompiledFromBlueprint);
	}
	return false;
}

#if WITH_EDITOR
void UDialogBuilderEdNode::PostEditUndo()
{
	UEdGraphNode::PostEditUndo();
}

void UDialogBuilderEdNode::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);

	
}
#endif