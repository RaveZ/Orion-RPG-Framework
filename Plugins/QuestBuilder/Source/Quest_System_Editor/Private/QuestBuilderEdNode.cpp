// Copyright 2024 Ivan Chandra. All Rights Reserved.


#include "QuestBuilderEdNode.h"
#include "QuestSystemGraphNode.h"

UQuestBuilderEdNode::UQuestBuilderEdNode()
{
	bIsReadOnly = false;
	bCanRenameNode = true;
}

UQuestBuilderEdNode::~UQuestBuilderEdNode()
{
}

void UQuestBuilderEdNode::SetQuestSystemGraphNode(UQuestSystemGraphNode* InNode)
{
	QuestSystemGraphNode = InNode;
}

UQuestBuilderEdGraph* UQuestBuilderEdNode::GetQuestSystemGraphEdGraph()
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
	if (QuestSystemGraphNode == nullptr)
	{
		return Super::GetNodeTitle(TitleType);
	}
	else
	{
		return QuestSystemGraphNode->GetNodeTitle();
	}
}

bool UQuestBuilderEdNode::CanDuplicateNode() const
{
	return bIsReadOnly ? false : Super::CanDuplicateNode();
}

bool UQuestBuilderEdNode::CanUserDeleteNode() const
{
	return bIsReadOnly ? false : Super::CanUserDeleteNode();
}

void UQuestBuilderEdNode::PrepareForCopying()
{
	QuestSystemGraphNode->Rename(nullptr, this, REN_DontCreateRedirectors | REN_DoNotDirty);
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
	return QuestSystemGraphNode->GetNodeDescription();
}

FText UQuestBuilderEdNode::GetEventsText() const
{
	return QuestSystemGraphNode->GetEventsText();
}

FText UQuestBuilderEdNode::GetConditionsText() const
{
	return QuestSystemGraphNode->GetConditionsText();
}

FLinearColor UQuestBuilderEdNode::GetBackgroundColor() const
{
	return QuestSystemGraphNode == nullptr ? FLinearColor::Black : QuestSystemGraphNode->GetBackgroundColor();
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
	return QuestSystemGraphNode && QuestSystemGraphNode->GetClass()->HasAnyClassFlags(CLASS_CompiledFromBlueprint);
}

void UQuestBuilderEdNode::PostEditUndo()
{
	UEdGraphNode::PostEditUndo();
}
