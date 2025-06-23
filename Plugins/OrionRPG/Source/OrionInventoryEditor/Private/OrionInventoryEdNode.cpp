// Copyright 2025 Ivan Chandra. All Rights Reserved.


#include "OrionInventoryEdNode.h"
#include "EdGraphSchema_OrionInventory.h"
#include "SGraphEditorActionMenuInventory.h"
#include "Inventory/OrionInventoryGraph.h"
#include "Inventory/OrionInventoryNode.h"
#include "Inventory/OrionInventoryNode_Item.h"
#include "Inventory/OrionInventoryNode_ItemContainer.h"
#include "OrionInventoryEdSubNode_Item.h"
#include "OrionInventoryEditorUtils.h"
#include "GraphDiffControl.h"
#include "AssetRegistry/AssetRegistryModule.h"

#define LOCTEXT_NAMESPACE "OrionInventoryEditor"

UOrionInventoryEdNode::UOrionInventoryEdNode()
{
	bIsReadOnly = false;
	bCanRenameNode = false;
	CopySubNodeIndex = 0;
}

UOrionInventoryEdNode::~UOrionInventoryEdNode()
{
}


UOrionInventoryEdGraph* UOrionInventoryEdNode::GetOrionInventoryEdGraph()
{
	return Cast<UOrionInventoryEdGraph>(GetGraph());
}

FText UOrionInventoryEdNode::GetNodeTitle(ENodeTitleType::Type TitleType) const
{
	if (NodeInstance == nullptr)
	{
		return Super::GetNodeTitle(TitleType);
	}
	else
	{
		const UOrionInventoryNode* InventoryNode = Cast<UOrionInventoryNode>(NodeInstance);
		return InventoryNode->GetNodeTitle();
	}
}

FText UOrionInventoryEdNode::GetTooltipText() const
{
	FText TooltipDesc;

	if (TooltipDesc.IsEmpty())
	{
		if (!NodeInstance)
		{
			FString StoredClassName = ClassData.GetClassName();
			StoredClassName.RemoveFromEnd(TEXT("_C"));

			TooltipDesc = FText::Format(LOCTEXT("NodeClassError", "Class {0} not found, make sure it's saved!"), FText::FromString(StoredClassName));
		}
		else
		{
			if (ErrorMessage.Len() > 0)
			{
				TooltipDesc = FText::FromString(ErrorMessage);
			}
			else
			{
				if (NodeInstance->GetClass()->HasAnyClassFlags(CLASS_CompiledFromBlueprint) && !NodeInstance->GetClass()->GetPackage()->HasAnyPackageFlags(PKG_Cooked))
				{
					FAssetData AssetData(NodeInstance->GetClass()->ClassGeneratedBy);
					FString Description = AssetData.GetTagValueRef<FString>(GET_MEMBER_NAME_CHECKED(UBlueprint, BlueprintDescription));
					if (!Description.IsEmpty())
					{
						Description.ReplaceInline(TEXT("\\n"), TEXT("\n"));
						TooltipDesc = FText::FromString(MoveTemp(Description));
					}
				}
				else
				{
					TooltipDesc = NodeInstance->GetClass()->GetToolTipText();
				}
			}
		}
	}
	if (NodeInstance->GetClass()->HasAnyClassFlags(CLASS_CompiledFromBlueprint) && !NodeInstance->GetClass()->GetPackage()->HasAnyPackageFlags(PKG_Cooked))
	{
		TooltipDesc = FText::FromString(TooltipDesc.ToString() + "\n**Double click node to open blueprint");
	}

	return TooltipDesc;
}

void UOrionInventoryEdNode::FindDiffs(UEdGraphNode* OtherNode, FDiffResults& Results)
{
	Super::FindDiffs(OtherNode, Results);

	if (UOrionInventoryEdNode* OtherGraphNode = Cast<UOrionInventoryEdNode>(OtherNode))
	{
		if (NodeInstance && OtherGraphNode->NodeInstance)
		{
			FDiffSingleResult Diff;
			Diff.Diff = EDiffType::NODE_PROPERTY;
			Diff.Node1 = this;
			Diff.Node2 = OtherNode;
			Diff.ToolTip = LOCTEXT("DIF_NodeInstancePropertyToolTip", "A property of the node instance has changed");
			Diff.Category = EDiffType::MODIFICATION;

			DiffProperties(NodeInstance->GetClass(), OtherGraphNode->NodeInstance->GetClass(), NodeInstance, OtherGraphNode->NodeInstance, Results, Diff);
		}
	}
	UOrionInventoryEdNode* OtherInventoryEdNode = Cast<UOrionInventoryEdNode>(OtherNode);
	if (OtherInventoryEdNode)
	{
		auto DiffSubNodes = [&Results](const FText& NodeTypeDisplayName, const TArray<UOrionInventoryEdNode*>& LhsSubNodes, const TArray<UOrionInventoryEdNode*>& RhsSubNodes)
		{
			TArray<FGraphDiffControl::FNodeMatch> NodeMatches;
			TSet<const UEdGraphNode*> MatchedRhsNodes;
			FGraphDiffControl::FNodeDiffContext AdditiveDiffContext;
			AdditiveDiffContext.NodeTypeDisplayName = NodeTypeDisplayName;
			AdditiveDiffContext.bIsRootNode = false;

			// march through the all the nodes in the rhs and look for matches 
			for (UEdGraphNode* RhsSubNode : RhsSubNodes)
			{
				FGraphDiffControl::FNodeMatch NodeMatch;
				NodeMatch.NewNode = RhsSubNode;

				// Do two passes, exact and soft
				for (UEdGraphNode* LhsSubNode : LhsSubNodes)
				{
					if (FGraphDiffControl::IsNodeMatch(LhsSubNode, RhsSubNode, true, &NodeMatches))
					{
						NodeMatch.OldNode = LhsSubNode;
						break;
					}
				}

				if (NodeMatch.NewNode == nullptr)
				{
					for (UEdGraphNode* LhsSubNode : LhsSubNodes)
					{
						if (FGraphDiffControl::IsNodeMatch(LhsSubNode, RhsSubNode, false, &NodeMatches))
						{
							NodeMatch.OldNode = LhsSubNode;
							break;
						}
					}
				}

				// if we found a corresponding node in the lhs graph, track it (so we can prevent future matches with the same nodes)
				if (NodeMatch.IsValid())
				{
					NodeMatches.Add(NodeMatch);
					MatchedRhsNodes.Add(NodeMatch.OldNode);
				}

				NodeMatch.Diff(AdditiveDiffContext, Results);
			}
			FGraphDiffControl::FNodeDiffContext SubtractiveDiffContext = AdditiveDiffContext;
			SubtractiveDiffContext.DiffMode = FGraphDiffControl::EDiffMode::Subtractive;
			SubtractiveDiffContext.DiffFlags = FGraphDiffControl::EDiffFlags::NodeExistance;

			// go through the lhs nodes to catch ones that may have been missing from the rhs graph
			for (UEdGraphNode* LhsSubNode : LhsSubNodes)
			{
				// if this node has already been matched, move on
				if (!LhsSubNode || MatchedRhsNodes.Find(LhsSubNode))
				{
					continue;
				}

				// There can't be a matching node in RhsGraph because it would have been found above
				FGraphDiffControl::FNodeMatch NodeMatch;
				NodeMatch.NewNode = LhsSubNode;

				NodeMatch.Diff(SubtractiveDiffContext, Results);
			}
		};
		DiffSubNodes(LOCTEXT("InventoryItemDiffDisplayName", "InventoryItem"), InventoryItems, OtherInventoryEdNode->InventoryItems);
	}
}

bool UOrionInventoryEdNode::CanDuplicateNode() const
{
	return bIsReadOnly ? false : Super::CanDuplicateNode();
}

void UOrionInventoryEdNode::DestroyNode()
{
	if (ParentNode)
	{
		ParentNode->RemoveSubNode(this);
	}

	UEdGraphNode::DestroyNode();
}

void UOrionInventoryEdNode::PostPlacedNewNode()
{
	// NodeInstance can be already spawned by paste operation, don't override it

	UClass* NodeClass = ClassData.GetClass(true);
	if (NodeClass)
	{
		UEdGraph* MyGraph = GetGraph();
		UOrionInventoryEdGraph* InventoryEdGraph = Cast<UOrionInventoryEdGraph>(GetGraph());
		UObject* GraphOwner = MyGraph ? MyGraph->GetOuter() : nullptr;
		if (GraphOwner)
		{
			if (NodeInstance == nullptr)
			{
				NodeInstance = NewObject<UObject>(GraphOwner, NodeClass);
				NodeInstance->SetFlags(RF_Transactional);
			}
			if (NodeInstance)
			{
				if (UOrionInventoryNode_ItemContainer* ItemContainerNode = Cast<UOrionInventoryNode_ItemContainer>(NodeInstance))
				{
					FindUniqueNodeName("ItemContainer");
				}
				if (UOrionInventoryNode_Item* ItemNode = Cast<UOrionInventoryNode_Item>(NodeInstance))
				{
					FindUniqueNodeName("Item");
				}
			}
			

			InitializeInstance();
		}
	}
	if (GetOrionInventoryEdGraph())
	{
		GetOrionInventoryEdGraph()->UpdateAsset();
	}
}

void UOrionInventoryEdNode::InitializeInstance()
{
	UOrionInventoryNode* InventoryNode = Cast<UOrionInventoryNode>(NodeInstance);
	UOrionInventoryGraph* InventoryAsset = InventoryNode ? Cast<UOrionInventoryGraph>(InventoryNode->GetOuter()) : nullptr;
	if (InventoryNode && InventoryAsset)
	{
		InventoryNode->InitializeFromAsset(*InventoryAsset);
	}
}

bool UOrionInventoryEdNode::CanUserDeleteNode() const
{
	return bIsReadOnly ? false : Super::CanUserDeleteNode();
}

void UOrionInventoryEdNode::PrepareForCopying()
{
	if (NodeInstance)
	{
		// Temporarily take ownership of the node instance, so that it is not deleted when cutting
		NodeInstance->Rename(nullptr, this, REN_DontCreateRedirectors | REN_DoNotDirty);
	}
}

void UOrionInventoryEdNode::AutowireNewNode(UEdGraphPin* FromPin)
{
	Super::AutowireNewNode(FromPin);

	if (FromPin != nullptr) {
		if (GetSchema()->TryCreateConnection(FromPin, GetInputPin()))
		{
			FromPin->GetOwningNode()->NodeConnectionListChanged();
		}
	}
}


void UOrionInventoryEdNode::UpdateSubnodeDependencies()
{
}


void UOrionInventoryEdNode::AddSubNode(UOrionInventoryEdNode* SubNode, UEdGraph* ParentGraph)
{
	const FScopedTransaction Transaction(LOCTEXT("AddNode", "Add Node"));
	

	SubNode->SetFlags(RF_Transactional);

	// set outer to be the graph so it doesn't go away
	SubNode->Rename(nullptr, ParentGraph, REN_NonTransactional);
	SubNode->ParentNode = this;

	SubNode->CreateNewGuid();
	SubNode->PostPlacedNewNode();
	SubNode->AllocateDefaultPins();
	SubNode->AutowireNewNode(nullptr);

	SubNode->NodePosX = 0;
	SubNode->NodePosY = 0;

	SubNodes.Add(SubNode);
	OnSubNodeAdded(SubNode);

	ParentGraph->NotifyGraphChanged();
	ParentGraph->Modify();
	Modify();
	
}

void UOrionInventoryEdNode::RemoveSubNode(UOrionInventoryEdNode* SubNode)
{
	Modify();
	SubNodes.RemoveSingle(SubNode);

	OnSubNodeRemoved(SubNode);
	GetGraph()->Modify();
}

void UOrionInventoryEdNode::RemoveAllSubNodes()
{
	SubNodes.Reset();
	GetGraph()->Modify();
}

void UOrionInventoryEdNode::OnSubNodeRemoved(UOrionInventoryEdNode* SubNode)
{
	const int32 InventoryItemIdx = InventoryItems.IndexOfByKey(SubNode);

	if (InventoryItemIdx >= 0)
	{
		InventoryItems.RemoveAt(InventoryItemIdx);
	}

}

void UOrionInventoryEdNode::OnSubNodeAdded(UOrionInventoryEdNode* NodeTemplate)
{
	UOrionInventoryEdNode* InventoryEdNode = Cast<UOrionInventoryEdNode>(NodeTemplate);

	if (Cast<UOrionInventoryEdSubNode_Item>(NodeTemplate))
	{
		InventoryItems.Add(NodeTemplate);
	}
}

int32 UOrionInventoryEdNode::FindSubNodeDropIndex(UOrionInventoryEdNode* SubNode) const
{
	const int32 SubIdx = SubNodes.IndexOfByKey(SubNode) + 1;
	const int32 InventoryItemIdx = InventoryItems.IndexOfByKey(SubNode) + 1;
	const int32 CombinedIdx = (SubIdx & 0xff) | ((InventoryItemIdx & 0xff) << 8);
	return CombinedIdx;
}

void UOrionInventoryEdNode::InsertSubNodeAt(UOrionInventoryEdNode* SubNode, int32 DropIndex)
{
	const int32 SubIdx = (DropIndex & 0xff) - 1;
	const int32 InventoryItemIdx = ((DropIndex >> 8) & 0xff) - 1;

	if (SubIdx >= 0)
	{
		SubNodes.Insert(SubNode, SubIdx);
	}
	else
	{
		SubNodes.Add(SubNode);
	}

	UOrionInventoryEdNode* TypedNode = Cast<UOrionInventoryEdNode>(SubNode);
	const bool bIsInventoryItem = Cast<UOrionInventoryNode_Item>(SubNode->NodeInstance) != nullptr;

	if (TypedNode)
	{
		if (bIsInventoryItem)
		{
			if (InventoryItemIdx >= 0)
			{
				InventoryItems.Insert(TypedNode, InventoryItemIdx);
			}
			else
			{
				InventoryItems.Add(TypedNode);
			}

		}

	}
	GetGraph()->Modify();
}

bool UOrionInventoryEdNode::IsSubNode() const
{
	return bIsSubNode || (ParentNode != nullptr);
}

bool UOrionInventoryEdNode::RefreshNodeClass()
{
	bool bUpdated = false;
	if (NodeInstance == nullptr)
	{
		if (FGraphNodeClassHelper::IsClassKnown(ClassData))
		{
			PostPlacedNewNode();
			bUpdated = (NodeInstance != nullptr);
		}
		else
		{
			FGraphNodeClassHelper::AddUnknownClass(ClassData);
		}
	}

	return bUpdated;
}

void UOrionInventoryEdNode::UpdateNodeClassData()
{
	if (NodeInstance)
	{
		UpdateNodeClassDataFrom(NodeInstance->GetClass(), ClassData);
		UpdateErrorMessage();
	}
}

void UOrionInventoryEdNode::UpdateErrorMessage()
{
	ErrorMessage = ClassData.GetDeprecatedMessage();
}

bool UOrionInventoryEdNode::HasErrors() const
{
	return ErrorMessage.Len() > 0 || NodeInstance == nullptr;
}

void UOrionInventoryEdNode::UpdateNodeClassDataFrom(UClass* InstanceClass, FGraphNodeClassData& UpdatedData)
{
	if (InstanceClass)
	{
		if (UBlueprint* BPOwner = Cast<UBlueprint>(InstanceClass->ClassGeneratedBy))
		{
			UpdatedData = FGraphNodeClassData(BPOwner->GetName(), BPOwner->GetOutermost()->GetName(), InstanceClass->GetName(), InstanceClass);
		}
		else if (UBlueprintGeneratedClass* BPGC = Cast<UBlueprintGeneratedClass>(InstanceClass))
		{
			UpdatedData = FGraphNodeClassData(BPGC->GetClassPathName(), BPGC);
		}
		else
		{
			UpdatedData = FGraphNodeClassData(InstanceClass, FGraphNodeClassHelper::GetDeprecationMessage(InstanceClass));
		}
	}
}

void UOrionInventoryEdNode::PostCopyNode()
{
	ResetNodeOwner();
}

void UOrionInventoryEdNode::ResetNodeOwner()
{
	if (NodeInstance)
	{
		UEdGraph* MyGraph = GetGraph();
		UObject* GraphOwner = MyGraph ? MyGraph->GetOuter() : nullptr;

		NodeInstance->Rename(NULL, GraphOwner, REN_DontCreateRedirectors | REN_DoNotDirty);
		NodeInstance->ClearFlags(RF_Transient);

		for (auto& SubNode : SubNodes)
		{
			SubNode->ResetNodeOwner();
		}
	}
}

FName UOrionInventoryEdNode::GetNameIcon() const
{
	return FName("BTEditor.Graph.BTNode.Icon");
}

void UOrionInventoryEdNode::FindUniqueNodeName(const FString& InBaseName)
{
	// Start with the base name
	FString BaseName = InBaseName;
	FString UniqueName = InBaseName;// Iterate through the inventory graph pages until we find a unique name

	int32 UnderscoreIndex;
	if (BaseName.FindLastChar(TEXT('_'), UnderscoreIndex))
	{
		FString Suffix = BaseName.Mid(UnderscoreIndex + 1);
		if (Suffix.IsNumeric())
		{
			BaseName = BaseName.Left(UnderscoreIndex);
		}
	}

	UOrionInventoryNode* InventoryNode = NodeInstance ? Cast<UOrionInventoryNode>(NodeInstance) : nullptr;
	UOrionInventoryEdGraph* InventoryEdGraph = GetOrionInventoryEdGraph();

	if (InventoryNode && InventoryEdGraph)
	{
		for (int i = 0;; i++)
		{
			bool bIsUnique = true;


			for (auto& Node : InventoryEdGraph->Nodes)
			{
				UOrionInventoryEdNode* NodeEdNode = Cast<UOrionInventoryEdNode>(Node);
				if (NodeEdNode && NodeEdNode->NodeInstance)
				{
					UOrionInventoryNode* OtherInventoryNode = Cast<UOrionInventoryNode>(NodeEdNode->NodeInstance);
					if (OtherInventoryNode && OtherInventoryNode != InventoryNode)
					{
						if (FName(UniqueName) == OtherInventoryNode->ID)
						{
							bIsUnique = false;
							break;
						}
					}
				}
			}


			// If the current name is unique, return it
			if (bIsUnique)
			{
				InventoryNode->ID = FName(UniqueName);
				return;
			}

			// If the current name is not unique, append a number to it and try again
			UniqueName = FString::Printf(TEXT("%s_%d"), *BaseName, i + 1);
		}
	}
}

FText UOrionInventoryEdNode::GetDescription() const
{
	
	UOrionInventoryNode* InventoryNode = Cast<UOrionInventoryNode>(NodeInstance);
	if (InventoryNode)
	{
		return InventoryNode->GetNodeDescription();
	}
	
	

	FString StoredClassName = ClassData.GetClassName();
	StoredClassName.RemoveFromEnd(TEXT("_C"));

	return FText::Format(LOCTEXT("NodeClassError", "Class {0} not found, make sure it's saved!"), FText::FromString(StoredClassName));

}

FLinearColor UOrionInventoryEdNode::GetBackgroundColor() const
{
	return FLinearColor(0.15f, 0.15f, 0.15f);
}

UEdGraphPin* UOrionInventoryEdNode::GetInputPin() const
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

UEdGraphPin* UOrionInventoryEdNode::GetOutputPin() const
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


bool UOrionInventoryEdNode::UsesBlueprint() const
{
	if (NodeInstance)
	{
		return NodeInstance && NodeInstance->GetClass()->HasAnyClassFlags(CLASS_CompiledFromBlueprint);
	}
	return false;
}

void UOrionInventoryEdNode::CreateAddInventoryItemSubMenu(UToolMenu* Menu, UEdGraph* Graph) const
{
	TSharedRef<SGraphEditorActionMenuInventory> Widget =
		SNew(SGraphEditorActionMenuInventory)
		.GraphObj(Graph)
		.GraphNode((UOrionInventoryEdNode*)this)
		.SubNodeFlags(EInventorySubNode::Item)
		.AutoExpandActionMenu(true);

	FToolMenuSection& Section = Menu->FindOrAddSection("Section");
	Section.AddEntry(FToolMenuEntry::InitWidget("InventoryItemWidget", Widget, FText(), true));
}


void UOrionInventoryEdNode::AddContextMenuActionsInventoryItems(UToolMenu* Menu, const FName SectionName, UGraphNodeContextMenuContext* Context) const
{
	FToolMenuSection& Section = Menu->FindOrAddSection(SectionName);
	Section.AddSubMenu(
		"AddInventoryItem",
		LOCTEXT("AddInventoryItem", "Add InventoryItem..."),
		LOCTEXT("AddInventoryItemTooltip", "Adds new InventoryItem as a subnode"),
		FNewToolMenuDelegate::CreateUObject(this, &UOrionInventoryEdNode::CreateAddInventoryItemSubMenu, (UEdGraph*)Context->Graph));

}


#if WITH_EDITOR
void UOrionInventoryEdNode::PostEditImport()
{
	ResetNodeOwner();

	if (NodeInstance)
	{
		InitializeInstance();
	}
}
void UOrionInventoryEdNode::PostEditUndo()
{
	Super::PostEditUndo();

	ResetNodeOwner();

	if (ParentNode)
	{
		ParentNode->SubNodes.AddUnique(this);
	}

	UOrionInventoryEdNode* MyParentNode = Cast<UOrionInventoryEdNode>(ParentNode);
	if (MyParentNode)
	{
		const bool bIsInventoryItem = Cast<UOrionInventoryNode_Item>(NodeInstance) != nullptr;

		if (bIsInventoryItem)
		{
			MyParentNode->InventoryItems.AddUnique(this);
		}
	}
}
void UOrionInventoryEdNode::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);

	
}
#endif

#undef LOCTEXT_NAMESPACE