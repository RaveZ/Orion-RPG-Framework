// Copyright 2025 Ivan Chandra. All Rights Reserved.


#include "OrionInventoryEdGraph.h"
#include "OrionInventoryEdNode_Root.h"
#include "OrionInventoryEdNode.h"
#include "OrionInventoryEdSubNode_Item.h"
#include "Inventory/OrionInventoryGraph.h"
#include "Inventory/OrionInventoryNode.h"
#include "Inventory/OrionInventoryNode_ItemDefinition.h"


UOrionInventoryEdGraph::UOrionInventoryEdGraph()
{
	GetOutermost()->PackageMarkedDirtyEvent.AddUObject(this, &UOrionInventoryEdGraph::OnPackageMarkedDirty);
	LastRebuildTime = 0.f;
	LastSortTime = 0.f;
	bLockUpdates = false;
}

UOrionInventoryEdGraph::~UOrionInventoryEdGraph()
{
}



void UOrionInventoryEdGraph::OnPackageMarkedDirty(UPackage* ModifiedPackage, bool bWasDirty)  
{  
   
}


void UOrionInventoryEdGraph::UpdateAsset(bool bForce)
{
	if (bLockUpdates ||
		!OwningInventory)
	{
		return;
	}

	const double CurrentTime = FPlatformTime::Seconds();

	if (!bForce)
	{
		// Prevent the function from being called twice in the same timeframe (e.g., 1 second)
		if (CurrentTime - LastRebuildTime < .5f)
		{
			return;
		}
	}

	LastRebuildTime = CurrentTime;

	UE_LOG(LogTemp, Log, TEXT("UpdateAsset started."));


	UOrionInventoryGraph* InventoryGraph = GetOrionInventoryGraph();

	Clear();

	UpdateAllSubnodeDependencies();


	for (int i = 0; i < Nodes.Num(); ++i)
	{
		if (UOrionInventoryEdNode* InventoryEdNode = Cast<UOrionInventoryEdNode>(Nodes[i]))
		{
			if (InventoryEdNode->NodeInstance == nullptr)
				continue;

			// cache root
			if (UOrionInventoryEdNode_Root* RootNode = Cast<UOrionInventoryEdNode_Root>(Nodes[i]))
			{
				if (RootNode->NodeInstance)
				{
					OwningInventory->RootNodes.Add(Cast<UOrionInventoryNode>(RootNode->NodeInstance));
				}
			}
			UOrionInventoryNode* InventoryNode = InventoryEdNode ? Cast<UOrionInventoryNode>(InventoryEdNode->NodeInstance) : nullptr;
			if (InventoryNode)
			{
				//cache inventory and node dependencies
				NodeMap.Add(InventoryNode, InventoryEdNode);
				OwningInventory->AllNodes.Add(InventoryNode);
				OwningInventory->NodeMap.Emplace(InventoryNode->ID, InventoryNode);

				InventoryNode->OrionInventoryGraph = OwningInventory;
				InventoryNode->Rename(nullptr, InventoryGraph, REN_DontCreateRedirectors | REN_DoNotDirty);

				
			}
			
		}
	}

	SortAllChildrenNodes(true);
}

void UOrionInventoryEdGraph::UpdateAllSubnodeDependencies()
{
	UOrionInventoryGraph* InventoryGraph = GetOrionInventoryGraph();
	for (int i = 0; i < Nodes.Num(); ++i)
	{
		if (UOrionInventoryEdNode* InventoryEdNode = Cast<UOrionInventoryEdNode>(Nodes[i]))
		{
			if (InventoryEdNode->NodeInstance == nullptr)
				continue;
			UOrionInventoryNode* InventoryNode = InventoryEdNode ? Cast<UOrionInventoryNode>(InventoryEdNode->NodeInstance) : nullptr;

			if (InventoryNode)
			{
				// parent chain
				InventoryEdNode->ParentNode = NULL;

				for (int32 iAux = 0; iAux < InventoryEdNode->InventoryItems.Num(); iAux++)
				{
					InventoryEdNode->InventoryItems[iAux]->ParentNode = InventoryEdNode;
					if (UOrionInventoryNode_ItemDefinition* ItemInstance = Cast<UOrionInventoryNode_ItemDefinition>(InventoryEdNode->InventoryItems[iAux]->NodeInstance))
					{
						InventoryGraph->ItemDefinitions.Add(ItemInstance->ItemDefinition.Tag, ItemInstance);
						ItemInstance->Rename(nullptr, InventoryGraph, REN_DontCreateRedirectors | REN_DoNotDirty);
					}
				}

			}
			
		}
	}
}

bool UOrionInventoryEdGraph::IsLocked() const
{
	return bLockUpdates;
}

void UOrionInventoryEdGraph::LockUpdates()
{
	bLockUpdates = true;
}

void UOrionInventoryEdGraph::UnlockUpdates()
{
	bLockUpdates = false;
	UpdateAsset();
}


void UOrionInventoryEdGraph::CreateNewInventoryItemOnNode(UOrionInventoryEdNode* InNode)
{
	UOrionInventoryGraph* InventoryGraph = GetOrionInventoryGraph();
	if (!InventoryGraph)
		return;

	if (!Schema)
		return;
	
	if (!InNode)
		return;

	if (InNode->ParentNode)
		InNode = InNode->ParentNode;

	UClass* ItemEdNodeClass = UOrionInventoryEdSubNode_Item::StaticClass();
	UClass* ItemNodeClass = UOrionInventoryNode_ItemDefinition::StaticClass();

	// Create a new subnode (replace with your subnode class if different)
	UOrionInventoryEdNode* SubNode = NewObject<UOrionInventoryEdNode>(this, ItemEdNodeClass);
	FGraphNodeClassData ItemClassData = FGraphNodeClassData(ItemNodeClass, "ItemClassData");
	SubNode->ClassData = ItemClassData;
	SubNode->NodeInstance = NewObject<UObject>(InventoryGraph, ItemNodeClass);
	SubNode->NodeInstance->SetFlags(RF_Transactional);

	// Add to parent node's subnodes array
	InNode->AddSubNode(SubNode, this);

	// Notify the graph/schema of the change
	NotifyGraphChanged();
}

void UOrionInventoryEdGraph::OnSubNodeDropped()
{
	NotifyGraphChanged();
}

bool UOrionInventoryEdGraph::UpdateUnknownNodeClasses()
{
	bool bUpdated = false;
	for (int32 NodeIdx = 0; NodeIdx < Nodes.Num(); NodeIdx++)
	{
		UOrionInventoryEdNode* MyNode = Cast<UOrionInventoryEdNode>(Nodes[NodeIdx]);
		if (MyNode)
		{
			const bool bUpdatedNode = MyNode->RefreshNodeClass();
			bUpdated = bUpdated || bUpdatedNode;

			for (int32 SubNodeIdx = 0; SubNodeIdx < MyNode->SubNodes.Num(); SubNodeIdx++)
			{
				if (MyNode->SubNodes[SubNodeIdx])
				{
					const bool bUpdatedSubNode = MyNode->SubNodes[SubNodeIdx]->RefreshNodeClass();
					bUpdated = bUpdated || bUpdatedSubNode;
				}
			}
		}
	}

	return bUpdated;
}

void UpdateInventoryGraphNodeErrorMessage(UOrionInventoryEdNode& Node)
{
	// Broke out setting error message in to own function so it can be reused when iterating nodes collection.
	if (Node.NodeInstance)
	{
		Node.ErrorMessage = FGraphNodeClassHelper::GetDeprecationMessage(Node.NodeInstance->GetClass());

		// Only check for node-specific errors if the node is not deprecated
		if (Node.ErrorMessage.IsEmpty())
		{
			Node.UpdateErrorMessage();

			// For node-specific validation we don't want to spam the log with errors
			return;
		}
	}
	else
	{
		// Null instance. Do we have any meaningful class data?
		FString StoredClassName = Node.ClassData.GetClassName();
		StoredClassName.RemoveFromEnd(TEXT("_C"));

		if (!StoredClassName.IsEmpty())
		{
			// There is class data here but the instance was not be created.
			static const FString IsMissingClassMessage(" class missing. Referenced by ");
			Node.ErrorMessage = StoredClassName + IsMissingClassMessage + Node.GetFullName();
		}
	}

	if (Node.HasErrors())
	{
		UE_LOG(LogTemp, Error, TEXT("%s"), *Node.ErrorMessage);
	}
}

void UOrionInventoryEdGraph::UpdateDeprecatedClasses()
{
	// This function sets error messages and logs errors about nodes.

	for (int32 Idx = 0, IdxNum = Nodes.Num(); Idx < IdxNum; ++Idx)
	{
		UOrionInventoryEdNode* Node = Cast<UOrionInventoryEdNode>(Nodes[Idx]);
		if (Node != nullptr)
		{
			UpdateInventoryGraphNodeErrorMessage(*Node);

			for (int32 SubIdx = 0, SubIdxNum = Node->SubNodes.Num(); SubIdx < SubIdxNum; ++SubIdx)
			{
				if (Node->SubNodes[SubIdx] != nullptr)
				{
					UpdateInventoryGraphNodeErrorMessage(*Node->SubNodes[SubIdx]);
				}
			}
		}
	}
}

void UOrionInventoryEdGraph::UpdateClassData()
{
	for (int32 Idx = 0; Idx < Nodes.Num(); Idx++)
	{
		UOrionInventoryEdNode* Node = Cast<UOrionInventoryEdNode>(Nodes[Idx]);
		if (Node)
		{
			Node->UpdateNodeClassData();

			for (int32 SubIdx = 0; SubIdx < Node->SubNodes.Num(); SubIdx++)
			{
				if (UOrionInventoryEdNode* SubNode = Node->SubNodes[SubIdx])
				{
					SubNode->UpdateNodeClassData();
				}
			}
		}
	}
}

void UOrionInventoryEdGraph::RemoveUnknownSubNodes()
{
	for (int32 Index = 0; Index < Nodes.Num(); ++Index)
	{
		UOrionInventoryEdNode* Node = Cast<UOrionInventoryEdNode>(Nodes[Index]);
		if (Node)
		{
			for (int32 SubIdx = Node->SubNodes.Num() - 1; SubIdx >= 0; SubIdx--)
			{
				const bool bIsInventoryItem = Node->InventoryItems.Contains(Node->SubNodes[SubIdx]);

				if (!bIsInventoryItem)
				{
					Node->SubNodes.RemoveAt(SubIdx);
				}
			}
		}
	}
}

void UOrionInventoryEdGraph::SortAllChildrenNodes(bool bForce)
{
	const double CurrentTime = FPlatformTime::Seconds();

	if (!bForce)
	{
		// Prevent the function from being called twice in the same timeframe (e.g., 1 second)
		if (CurrentTime - LastSortTime < .5f)
		{
			return;
		}
	}

	LastSortTime = CurrentTime;
	FTimerHandle TimerHandle;
	if (GEditor)
	{
		GEditor->GetTimerManager()->SetTimer(
			TimerHandle,
			FTimerDelegate::CreateLambda([this]()
				{
					//sort children nodes based on the PosY of InventoryEdNode
					for (auto& EdNode : Nodes)
					{
						RebuildChildOrder(EdNode);

					}
				}),
			.5f, // Delay in seconds
			false // Do not loop
		);
	}
}


void UOrionInventoryEdGraph::RebuildChildOrder(UEdGraphNode* ParentNode)
{
	UOrionInventoryEdNode* InventoryParentEdNode = Cast<UOrionInventoryEdNode>(ParentNode);
	UOrionInventoryNode* InventoryParentNode = InventoryParentEdNode ? Cast<UOrionInventoryNode>(InventoryParentEdNode->NodeInstance) : nullptr;
	if (InventoryParentNode)
	{
		//TODO:
		//Build Order based on the PosX of all Nodes in the graph
	}
}

UOrionInventoryGraph* UOrionInventoryEdGraph::GetOrionInventoryGraph() const
{
	return CastChecked<UOrionInventoryGraph>(GetOuter());
}

bool UOrionInventoryEdGraph::Modify(bool bAlwaysMarkDirty)
{
	bool Rtn = Super::Modify(bAlwaysMarkDirty);

	GetOrionInventoryGraph()->Modify();

	UpdateAllSubnodeDependencies();

	for (int32 i = 0; i < Nodes.Num(); ++i)
	{
		Nodes[i]->Modify();
	}

	return Rtn;
}

#if WITH_EDITOR
void UOrionInventoryEdGraph::PostEditUndo()
{
	Super::PostEditUndo();

	NotifyGraphChanged();
	UpdateAsset();
}

void UOrionInventoryEdGraph::CollectAllNodeInstances(TSet<UObject*>& NodeInstances)
{
	for (int32 Idx = 0; Idx < Nodes.Num(); Idx++)
	{
		UOrionInventoryEdNode* MyNode = Cast<UOrionInventoryEdNode>(Nodes[Idx]);
		if (MyNode)
		{
			NodeInstances.Add(MyNode->NodeInstance);

			for (int32 SubIdx = 0; SubIdx < MyNode->SubNodes.Num(); SubIdx++)
			{
				if (MyNode->SubNodes[SubIdx])
				{
					NodeInstances.Add(MyNode->SubNodes[SubIdx]->NodeInstance);
				}
			}
		}
	}
}
void UOrionInventoryEdGraph::UpdateVersion_UnifiedSubNodes()
{
	for (int32 NodeIdx = 0; NodeIdx < Nodes.Num(); NodeIdx++)
	{
		UOrionInventoryEdNode* MyNode = Cast<UOrionInventoryEdNode>(Nodes[NodeIdx]);
		if (MyNode == nullptr)
		{
			continue;
		}

		MyNode->SubNodes.Reset(MyNode->InventoryItems.Num());

		for (int32 SubIdx = 0; SubIdx < MyNode->InventoryItems.Num(); SubIdx++)
		{
			MyNode->SubNodes.Add(MyNode->InventoryItems[SubIdx]);
		}
	}
}
#endif // WITH_EDITOR

void UOrionInventoryEdGraph::Clear()
{
	if (OwningInventory)
	{
		OwningInventory->ClearGraph();
	}
	NodeMap.Reset();

}
