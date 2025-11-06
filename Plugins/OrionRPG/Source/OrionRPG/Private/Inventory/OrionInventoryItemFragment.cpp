// Copyright 2025 Ivan Chandra. All Rights Reserved.


#include "Inventory/OrionInventoryItemFragment.h"


#define LOCTEXT_NAMESPACE "OrionInventoryItemFragment"

//////////////////////////////////////////////////////////////////////

// Item fragment
UOrionInventoryItemFragment::UOrionInventoryItemFragment()
{
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

// Equippable item fragment
UOrionInventoryFragment_EquippableItem::UOrionInventoryFragment_EquippableItem()
{

}

//////////////////////////////////////////////////////////////////////

// Consumable item fragment
UOrionInventoryFragment_ConsumableItem::UOrionInventoryFragment_ConsumableItem()
{
}


#undef LOCTEXT_NAMESPACE

