// Copyright 2025 Ivan Chandra. All Rights Reserved.


#include "Inventory/OrionInventoryItemFragment.h"
#include "Inventory/OrionInventoryItemInstance.h"
#include "Inventory/OrionInventoryNode_ItemDefinition.h"
#include "Inventory/OrionInventoryComponent.h"
#include "Inventory/OrionInventoryFunctionLibrary.h"
#include "Engine/StreamableManager.h"
#include "Engine/AssetManager.h"
#include "Engine/World.h"


#define LOCTEXT_NAMESPACE "OrionInventoryItemFragment"

//////////////////////////////////////////////////////////////////////

// Item fragment
UOrionInventoryItemFragment::UOrionInventoryItemFragment()
{
	bHasMenuAction = true;
	ActionDisplayName = FText::FromString("Action");
}


bool UOrionInventoryItemFragment::K2_CanExecuteAction_Implementation() const
{
	return bHasMenuAction;
}

void UOrionInventoryItemFragment::K2_ExecuteAction_Implementation()
{

}

FText UOrionInventoryItemFragment::K2_GetActionDisplayName_Implementation() const
{
	return ActionDisplayName;
}

UWorld* UOrionInventoryItemFragment::GetWorld() const
{
	if (HasAllFlags(RF_ClassDefaultObject))
	{
		// If we are a CDO, we must return nullptr instead of calling Outer->GetWorld() to fool UObject::ImplementsGetWorld.
		return nullptr;
	}


	UObject* Outer = GetOuter();

	while (Outer)
	{
		UWorld* World = Outer->GetWorld();
		if (World)
		{
			return World;
		}

		Outer = Outer->GetOuter();
	}

	return nullptr;
}


//////////////////////////////////////////////////////////////////////

// Consumable item fragment
UOrionInventoryFragment_ConsumableItem::UOrionInventoryFragment_ConsumableItem()
{
	ActionDisplayName = FText::FromString("Consume");
}

//////////////////////////////////////////////////////////////////////

// Inspect item fragment
UOrionInventoryFragment_InspectItem::UOrionInventoryFragment_InspectItem()
{
	ActionDisplayName = FText::FromString("Inspect");
}


void UOrionInventoryFragment_InspectItem::K2_ExecuteAction_Implementation()
{
	// Use ActorToInspect if set, otherwise fallback to pickup actor class
	if (UWorld* World = GetWorld())
	{
		UOrionInventoryComponent* InvComp = UOrionInventoryFunctionLibrary::GetInventoryComponent(World);
		if (InvComp)
		{
			InvComp->InspectItemInstance(OwningItemInstance, ActorToInspect);
		}
	}

}
	


//////////////////////////////////////////////////////////////////////

// Drop item fragment
UOrionInventoryFragment_DropItem::UOrionInventoryFragment_DropItem()
{
	ActionDisplayName = FText::FromString("Drop Item");
}

void UOrionInventoryFragment_DropItem::K2_ExecuteAction_Implementation()
{
	if (UWorld* World =  GetWorld())
	{
		UOrionInventoryComponent* InvComp = UOrionInventoryFunctionLibrary::GetInventoryComponent(World);
		if (InvComp)
		{
			InvComp->Server_DropItemInstance(OwningItemInstance, 1);
		}
	}
}



#undef LOCTEXT_NAMESPACE
