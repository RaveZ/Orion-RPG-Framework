// Copyright 2025 Ivan Chandra. All Rights Reserved.

#include "Inventory/OrionInventoryNode.h"
#include "Inventory/OrionInventoryGraph.h"
#include "Inventory/OrionInventoryComponent.h"

#define LOCTEXT_NAMESPACE "OrionInventoryNode"

UOrionInventoryNode::UOrionInventoryNode()
{
#if WITH_EDITORONLY_DATA
	CompatibleGraphType = UOrionInventoryGraph::StaticClass();

	FLinearColor Default(0.05f, 0.05f, 0.05f);
	BackgroundColor = Default;
#endif
}

UOrionInventoryNode::~UOrionInventoryNode()
{
}

void UOrionInventoryNode::Initialize(bool bLaunchEventOnLoad)
{
	
}


void UOrionInventoryNode::Deinitialize()
{
	
}

void UOrionInventoryNode::Reset()
{
	//empty in base 
}

void UOrionInventoryNode::BeginNode()
{
	//empty in base 
}

void UOrionInventoryNode::InitializeFromAsset(UOrionInventoryGraph& OwningOrionInventoryGraph)
{
	OrionInventoryGraph = &OwningOrionInventoryGraph;
}


UOrionInventoryGraph* UOrionInventoryNode::GetOwningOrionInventoryGraph() const
{
	return OrionInventoryGraph;
}

AController* UOrionInventoryNode::GetOwningController() const
{
	return OwningController;
}

APawn* UOrionInventoryNode::GetOwningPawn() const
{
	if (OrionInventoryComponent)
		return OrionInventoryComponent->GetOwningPawn();
	return nullptr;
}

UOrionInventoryComponent* UOrionInventoryNode::GetOrionInventoryComponent() const
{
	return OrionInventoryComponent;
}

FText UOrionInventoryNode::GetDescription_Implementation() const
{
	return FText();
}


FString UOrionInventoryNode::GetShortTypeName(const UObject* Ob)
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
bool UOrionInventoryNode::IsNameEditable() const
{
	return true;
}

FLinearColor UOrionInventoryNode::GetBackgroundColor() const
{
	return BackgroundColor;
}

FText UOrionInventoryNode::GetNodeTitle() const
{
	return FText::FromString(UOrionInventoryNode::GetShortTypeName(this));
}

FText UOrionInventoryNode::GetNodeDescription() const
{
	return FText::FromString(TEXT("This is NodeBase"));
}

void UOrionInventoryNode::SetNodeTitle(const FText& NewTitle)
{
}

bool UOrionInventoryNode::CanCreateConnection(UOrionInventoryNode* Other, FText& ErrorMessage)
{
	return true;
}

bool UOrionInventoryNode::CanCreateConnectionTo(UOrionInventoryNode* Other, int32 NumberOfChildrenNodes, FText& ErrorMessage)
{
	if (ChildrenLimitType == EOrionInventoryNodeLimits::Limited && NumberOfChildrenNodes >= ChildrenLimit)
	{
		ErrorMessage = FText::FromString("Children limit exceeded");
		return false;
	}

	return CanCreateConnection(Other, ErrorMessage);
	
}

bool UOrionInventoryNode::CanCreateConnectionFrom(UOrionInventoryNode* Other, int32 NumberOfParentNodes, FText& ErrorMessage)
{
	if (ParentLimitType == EOrionInventoryNodeLimits::Limited && NumberOfParentNodes >= ParentLimit)
	{
		ErrorMessage = FText::FromString("Parent limit exceeded");
		return false;
	}

	return true;
}

void UOrionInventoryNode::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);

}

void UOrionInventoryNode::PreEditChange(FProperty* PropertyAboutToChange)
{
	Super::PreEditChange(PropertyAboutToChange);
}


#endif

#undef LOCTEXT_NAMESPACE


