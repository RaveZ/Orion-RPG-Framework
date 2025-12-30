// Copyright 2025 Ivan Chandra. All Rights Reserved.


#include "Inventory/OrionInventoryGraph.h"
#include "Inventory/OrionInventoryComponent.h"
#include "Inventory/OrionInventoryNode.h"
#include "Inventory/OrionInventoryItemFragment.h"
#include "UObject/ConstructorHelpers.h"
#include "UObject/UObjectGlobals.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/PlayerController.h"

#define LOCTEXT_NAMESPACE "OrionInventoryGraph"

UOrionInventoryGraph::UOrionInventoryGraph()
{
	bAssetInitialized = false;
	InventoryType = EOrionInventoryType::E_WeightBased;
	DefaultMaxInventoryWeight = 100.0f;
	DefaultGridDimension = FIntPoint(5, 5);
	GridSize = 75.0f;
	EquipmentDefaultSlots = {
		TAG_Equipment_Slot_Head,
		TAG_Equipment_Slot_Chest,
		TAG_Equipment_Slot_Hands,
		TAG_Equipment_Slot_Legs,
		TAG_Equipment_Slot_Feet,
		TAG_Equipment_Slot_LeftHandWeapon,
		TAG_Equipment_Slot_RightHandWeapon,
		TAG_Equipment_Slot_Accessory1,
		TAG_Equipment_Slot_Accessory2,
	};
	
} 

UOrionInventoryGraph::~UOrionInventoryGraph()
{
	
}

bool UOrionInventoryGraph::Initialize(UOrionInventoryComponent* InitializingComp)
{
	if (!HasAnyFlags(RF_ClassDefaultObject))
	{
		//We need a valid Quest component to make a quest for 
		if (!InitializingComp)
		{
			return false;
		}
		OwningController = InitializingComp->GetOwningController();
		OrionInventoryComponent = InitializingComp;
		//NodeMap.Empty();
		for (auto& Node : AllNodes)
		{
			if (Node)
			{
				Node->OrionInventoryGraph = this;
				Node->OwningController = InitializingComp->GetOwningController();
				Node->OrionInventoryComponent = InitializingComp;
				//NodeMap.Emplace(Node->ID, Node);
			}
		}

	}
	return false;
}

void UOrionInventoryGraph::Deinitialize()
{
	
	for (auto& Node : AllNodes)
	{
		Node->Deinitialize();
	}
}

UWorld* UOrionInventoryGraph::GetWorld() const
{
	if (OrionInventoryComponent)
	{
		return OrionInventoryComponent->GetWorld();
	}

	return nullptr;
}

void UOrionInventoryGraph::PostInitProperties()
{
	Super::PostInitProperties();
	
}

void UOrionInventoryGraph::BeginNode(UOrionInventoryNode* InNode, bool bLaunchEventOnLoad)
{
}

void UOrionInventoryGraph::ClearGraph()
{
	AllNodes.Empty();
	RootNodes.Empty();
	NodeMap.Empty();
	ItemDefinitions.Empty();
}

void UOrionInventoryGraph::InitalizeAsset()
{
	if (!bAssetInitialized)
	{
		DefaultDropItemFragment = NewObject<UOrionInventoryFragment_DropItem>(this);
		
		bAssetInitialized = true;
	}
}

APlayerController* UOrionInventoryGraph::GetOwningController()
{
	return OwningController;
}

APawn* UOrionInventoryGraph::GetOwningPawn()
{
	if (OrionInventoryComponent)
	{
		return OrionInventoryComponent->GetOwningPawn();
	}
	return nullptr;
}

UOrionInventoryComponent* UOrionInventoryGraph::GetOrionInventoryComponent()
{
	return OrionInventoryComponent;
}

#if WITH_EDITORONLY_DATA

void UOrionInventoryGraph::GetAllGraphs(TArray<UEdGraph*>& Graphs) const
{
	

	for (int32 i = 0; i < InventoryGraphPages.Num(); ++i)
	{
		UEdGraph* Graph = InventoryGraphPages[i];
		if (Graph)
		{
			Graphs.Add(Graph);
			Graph->GetAllChildrenGraphs(Graphs);
		}
	}
	
}
#endif // WITH_EDITORONLY_DATA

#undef LOCTEXT_NAMESPACE