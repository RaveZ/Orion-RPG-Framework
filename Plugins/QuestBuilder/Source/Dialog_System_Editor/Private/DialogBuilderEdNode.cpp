// Copyright 2025 Ivan Chandra. All Rights Reserved.


#include "DialogBuilderEdNode.h"
#include "DialogBuilderSetting.h"
#include "DialogBuilderNode.h"
#include "Dialog.h"
#include "DialogBuilderNode_Root.h"
#include "DialogBuilderNode_Objective.h"
#include "DialogBuilderNode_State.h"
#include "DialogBuilderEditorUtils.h"

UDialogBuilderEdNode::UDialogBuilderEdNode()
{
	bIsReadOnly = false;
	bCanRenameNode = true;
}

UDialogBuilderEdNode::~UDialogBuilderEdNode()
{
}


UDialogBuilderEdGraph* UDialogBuilderEdNode::GetDialogBuilderEdGraph()
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
		const UDialogBuilderNode* DialogNode = Cast<UDialogBuilderNode>(NodeInstance);
		return DialogNode->GetNodeTitle();
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
		UDialogBuilderEdGraph* DialogEdGraph = Cast<UDialogBuilderEdGraph>(GetGraph());
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
				if (UDialogBuilderNode_Objective* ObjectiveDialogNode = Cast<UDialogBuilderNode_Objective>(NodeInstance))
				{
					ObjectiveDialogNode->ID = FDialogBuilderEditorUtils::FindUniqueNodeName("ObjectiveNode");
				}
				if (UDialogBuilderNode_State* StateDialogNode = Cast<UDialogBuilderNode_State>(NodeInstance))
				{
					StateDialogNode->ID = FDialogBuilderEditorUtils::FindUniqueNodeName("StateNode");
				}
			}
			UDialogBuilderNode* DialogNode = NodeInstance ?  Cast<UDialogBuilderNode>(NodeInstance) : nullptr;
			if (DialogNode)
			{
				DialogEdGraph->AddToNodeIDMap(DialogNode->ID, DialogNode);
			}
		}
	}
}

void UDialogBuilderEdNode::InitializeInstance()
{
	UDialogBuilderNode* DialogNode = Cast<UDialogBuilderNode>(NodeInstance);
	UDialogBuilderGraph* DialogAsset = DialogNode ? Cast<UDialogBuilderGraph>(DialogNode->GetOuter()) : nullptr;
	UDialog* Dialog = GetDialogBuilderEdGraph() ? GetDialogBuilderEdGraph()->Dialog : nullptr;
	if (Dialog && DialogNode && DialogAsset)
	{
		DialogNode->InitializeFromAsset(*DialogAsset, *Dialog);

	}
}

bool UDialogBuilderEdNode::CanUserDeleteNode() const
{
	return bIsReadOnly ? false : Super::CanUserDeleteNode();
}

void UDialogBuilderEdNode::PrepareForCopying()
{
	UDialogBuilderNode* DialogNode = Cast<UDialogBuilderNode>(NodeInstance);
	if (DialogNode)
	{
		DialogNode->Rename(nullptr, this, REN_DontCreateRedirectors | REN_DoNotDirty);

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
	UDialogBuilderNode* DialogNode = Cast<UDialogBuilderNode>(NodeInstance);
	if (DialogNode)
	{
		return DialogNode->GetNodeDescription();
	}
	return FText::GetEmpty();
}

FText UDialogBuilderEdNode::GetEventsText() const
{
	UDialogBuilderNode* DialogNode = Cast<UDialogBuilderNode>(NodeInstance);
	if (DialogNode)
	{
		return DialogNode->GetEventsText();
	}
	return FText::GetEmpty();
}

FText UDialogBuilderEdNode::GetConditionsText() const
{
	UDialogBuilderNode* DialogNode = Cast<UDialogBuilderNode>(NodeInstance);
	if (DialogNode)
	{
		return DialogNode->GetConditionsText();
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
	UDialogBuilderNode* DialogNode = Cast<UDialogBuilderNode>(NodeInstance);
	if (DialogNode)
	{
		return DialogNode && DialogNode->GetClass()->HasAnyClassFlags(CLASS_CompiledFromBlueprint);
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