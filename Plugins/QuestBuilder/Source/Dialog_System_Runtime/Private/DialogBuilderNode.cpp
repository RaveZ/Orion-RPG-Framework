// Copyright 2025 Ivan Chandra. All Rights Reserved.


#include "DialogBuilderNode.h"
#include "DialogBuilderEdge.h"
#include "Dialog.h"
#include "Event/OrionEvent.h"
#include "Decorator/OrionDecorator.h"
#include "DialogBuilderGraph.h"

#define LOCTEXT_NAMESPACE "DialogBuilderNode"

UDialogBuilderNode::UDialogBuilderNode()
{
#if WITH_EDITORONLY_DATA
	CompatibleGraphType = UDialogBuilderGraph::StaticClass();

	FLinearColor Default(0.05f, 0.05f, 0.05f);
	BackgroundColor = Default;
#endif
}

UDialogBuilderNode::~UDialogBuilderNode()
{
}

UDialogBuilderEdge* UDialogBuilderNode::GetEdge(UDialogBuilderNode* ChildNode)
{
	return Edges.Contains(ChildNode) ? Edges.FindChecked(ChildNode) : nullptr;
}


FText UDialogBuilderNode::GetDescription_Implementation() const
{
	return Description;
}

void UDialogBuilderNode::BeginEvents(EEventRunType EventRunType)
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


FString UDialogBuilderNode::GetShortTypeName(const UObject* Ob)
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



#if WITH_EDITOR
bool UDialogBuilderNode::IsNameEditable() const
{
	return true;
}

FLinearColor UDialogBuilderNode::GetBackgroundColor() const
{
	return BackgroundColor;
}

FText UDialogBuilderNode::GetNodeTitle() const
{
	return FText::FromString(UDialogBuilderNode::GetShortTypeName(this));
}

FText UDialogBuilderNode::GetNodeDescription() const
{
	return FText::FromString(TEXT("This is NodeBase"));
}

FText UDialogBuilderNode::GetEventsText() const
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

FText UDialogBuilderNode::GetConditionsText() const
{
	FString ConditionListText;

	for (const UOrionDecorator* Condition : Conditions)
	{
		if (Condition)
		{
			if(!ConditionListText.IsEmpty()) ConditionListText += TEXT("\n");

			ConditionListText += Condition->GetNodeDisplayText();
		}

	}

	return FText::FromString(ConditionListText);
}

void UDialogBuilderNode::SetNodeTitle(const FText& NewTitle)
{
}

bool UDialogBuilderNode::CanCreateConnection(UDialogBuilderNode* Other, FText& ErrorMessage)
{
	return true;
}

bool UDialogBuilderNode::CanCreateConnectionTo(UDialogBuilderNode* Other, int32 NumberOfChildrenNodes, FText& ErrorMessage)
{
	if (ChildrenLimitType == ENodeLimits::Limited && NumberOfChildrenNodes >= ChildrenLimit)
	{
		ErrorMessage = FText::FromString("Children limit exceeded");
		return false;
	}

	return CanCreateConnection(Other, ErrorMessage);
	
}

bool UDialogBuilderNode::CanCreateConnectionFrom(UDialogBuilderNode* Other, int32 NumberOfParentNodes, FText& ErrorMessage)
{
	if (ParentLimitType == ENodeLimits::Limited && NumberOfParentNodes >= ParentLimit)
	{
		ErrorMessage = FText::FromString("Parent limit exceeded");
		return false;
	}

	return true;
}


#endif

bool UDialogBuilderNode::IsLeafNode() const
{
	return ChildrenNodes.Num() == 0;
}

void UDialogBuilderNode::Initialize()
{
	//empty, override in child class
}

void UDialogBuilderNode::InitializeFromAsset(UDialogBuilderGraph& OwningDialogGraph, UDialog& OwningDialog)
{
	DialogGraph = &OwningDialogGraph;
	Dialog = &OwningDialog;
}


UDialogBuilderGraph* UDialogBuilderNode::GetOwningDialogGraph() const
{
	if (!DialogGraph)
		return nullptr;

	return DialogGraph;
}

UDialog* UDialogBuilderNode::GetOwningDialog() const
{
	return Dialog;
}


AController* UDialogBuilderNode::GetOwningController() const
{
	if (!DialogGraph)
		return nullptr;
	

	return DialogGraph->OwningController;
}

APawn* UDialogBuilderNode::GetOwningPawn() const
{
	if (!DialogGraph)
		return nullptr;

	return DialogGraph->OwningPawn;
}

UDialogComponent* UDialogBuilderNode::GetDialogComponent() const
{
	if (!DialogGraph)
		return nullptr;

	return DialogGraph->DialogComponent;
}

#undef LOCTEXT_NAMESPACE


