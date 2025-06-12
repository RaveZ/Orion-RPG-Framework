// Copyright 2025 Ivan Chandra. All Rights Reserved.


#include "OrionInventoryEdNode_Edge.h"
#include "DialogBuilderEdge.h"
#include "OrionInventoryEdNode.h"

#define LOCTEXT_NAMESPACE "EdNode_InventorySystemGraphEdge"

UOrionInventoryEdNode_Edge::UOrionInventoryEdNode_Edge()
{
	bCanRenameNode = true;
}

void UOrionInventoryEdNode_Edge::SetEdge(UDialogBuilderEdge* Edge)
{
	InventorySystemGraphEdge = Edge;
}

void UOrionInventoryEdNode_Edge::AllocateDefaultPins()
{
	UEdGraphPin* Inputs = CreatePin(EGPD_Input, TEXT("Edge"), FName(), TEXT("In"));
	Inputs->bHidden = true;
	UEdGraphPin* Outputs = CreatePin(EGPD_Output, TEXT("Edge"), FName(), TEXT("Out"));
	Outputs->bHidden = true;
}

FText UOrionInventoryEdNode_Edge::GetNodeTitle(ENodeTitleType::Type TitleType) const
{
	if (InventorySystemGraphEdge)
	{
		return InventorySystemGraphEdge->GetNodeTitle();
	}
	return FText();
}

void UOrionInventoryEdNode_Edge::PinConnectionListChanged(UEdGraphPin* Pin)
{
	if (Pin->LinkedTo.Num() == 0)
	{
		// Commit suicide; transitions must always have an input and output connection
		Modify();

		// Our parent graph will have our graph in SubGraphs so needs to be modified to record that.
		if (UEdGraph* ParentGraph = GetGraph())
		{
			ParentGraph->Modify();
		}

		DestroyNode();
	}
}

void UOrionInventoryEdNode_Edge::PrepareForCopying()
{
	InventorySystemGraphEdge->Rename(nullptr, this, REN_DontCreateRedirectors | REN_DoNotDirty);
}

void UOrionInventoryEdNode_Edge::CreateConnections(UOrionInventoryEdNode* Start, UOrionInventoryEdNode* End)
{
	Pins[0]->Modify();
	Pins[0]->LinkedTo.Empty();

	Start->GetOutputPin()->Modify();
	Pins[0]->MakeLinkTo(Start->GetOutputPin());

	// This to next
	Pins[1]->Modify();
	Pins[1]->LinkedTo.Empty();

	End->GetInputPin()->Modify();
	Pins[1]->MakeLinkTo(End->GetInputPin());
}

UOrionInventoryEdNode* UOrionInventoryEdNode_Edge::GetStartNode()
{
	if (Pins[0]->LinkedTo.Num() > 0)
	{
		return Cast<UOrionInventoryEdNode>(Pins[0]->LinkedTo[0]->GetOwningNode());
	}
	else
	{
		return nullptr;
	}
}

UOrionInventoryEdNode* UOrionInventoryEdNode_Edge::GetEndNode()
{
	if (Pins[1]->LinkedTo.Num() > 0)
	{
		return Cast<UOrionInventoryEdNode>(Pins[1]->LinkedTo[0]->GetOwningNode());
	}
	else
	{
		return nullptr;
	}
}

#undef LOCTEXT_NAMESPACE