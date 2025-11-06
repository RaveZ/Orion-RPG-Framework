// Copyright 2025 Ivan Chandra. All Rights Reserved.


#include "Inventory/OrionInventoryItemInstance.h"
#include "Inventory/OrionInventoryItemFragment.h"

#include "Inventory/OrionInventoryNode_ItemDefinition.h"
#include "Net/UnrealNetwork.h"

#if UE_WITH_IRIS
#include "Iris/ReplicationSystem/ReplicationFragmentUtil.h"
#endif // UE_WITH_IRIS

#include UE_INLINE_GENERATED_CPP_BY_NAME(OrionInventoryItemInstance)

class FLifetimeProperty;

#define LOCTEXT_NAMESPACE "OrionInventoryItemInstance"

UOrionInventoryItemInstance::UOrionInventoryItemInstance()
{
}


void UOrionInventoryItemInstance::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ThisClass, StatTags);
	DOREPLIFETIME(ThisClass, ItemDef);
	DOREPLIFETIME(ThisClass, TotalStackCount);
}

#if UE_WITH_IRIS
void UOrionInventoryItemInstance::RegisterReplicationFragments(UE::Net::FFragmentRegistrationContext& Context, UE::Net::EFragmentRegistrationFlags RegistrationFlags)
{
	using namespace UE::Net;

	// Build descriptors and allocate PropertyReplicationFragments for this object
	//FReplicationFragmentUtil::CreateAndRegisterFragmentsForObject(this, Context, RegistrationFlags);
}
#endif // UE_WITH_IRIS

void UOrionInventoryItemInstance::AddStatTagStack(FGameplayTag Tag, int32 StackCount)
{
	StatTags.AddStack(Tag, StackCount);
}

void UOrionInventoryItemInstance::RemoveStatTagStack(FGameplayTag Tag, int32 StackCount)
{
	StatTags.RemoveStack(Tag, StackCount);
}

int32 UOrionInventoryItemInstance::GetStatTagStackCount(FGameplayTag Tag) const
{
	return StatTags.GetStackCount(Tag);
}

bool UOrionInventoryItemInstance::HasStatTag(FGameplayTag Tag) const
{
	return StatTags.ContainsTag(Tag);
}

void UOrionInventoryItemInstance::SetItemDef(UOrionInventoryNode_ItemDefinition* InDef)
{
	ItemDef = InDef;
}

bool UOrionInventoryItemInstance::HasRoomForItem() const
{
	return GetItemDef() ? TotalStackCount < GetItemDef()->MaxStackCount : false;
}

void UOrionInventoryItemInstance::SetTotalStackCount(int32 Count)
{
	if (GetItemDef())
	{
		TotalStackCount = FMath::Clamp(Count, 0, GetItemDef()->MaxStackCount);
	}
	else
	{
		TotalStackCount = Count;
	}
}

const UOrionInventoryItemFragment* UOrionInventoryItemInstance::FindFragmentByClass(TSubclassOf<UOrionInventoryItemFragment> FragmentClass) const
{
	if ((ItemDef != nullptr) && (FragmentClass != nullptr))
	{
		return ItemDef->FindFragmentByClass(FragmentClass);
	}

	return nullptr;
}

FItemInstanceData UOrionInventoryItemInstance::GetItemInstanceData() const
{
	UOrionInventoryNode_ItemDefinition* InItemDefinition = GetItemDef();
	FItemInstanceData InstanceData;

	if (InItemDefinition)
	{
		InstanceData.Tag = InItemDefinition->Tag;
		InstanceData.DisplayName = InItemDefinition->DisplayName;
		InstanceData.Description = InItemDefinition->Description;
		InstanceData.Icon = InItemDefinition->Icon;
		InstanceData.StackCount = TotalStackCount;
		InstanceData.MaxStackCount = InItemDefinition->MaxStackCount;
		InstanceData.ItemDefinition = InItemDefinition;
		InstanceData.DropItemFragment = InItemDefinition->GetDropItemFragment();
		InstanceData.Fragments = InItemDefinition->Fragments;
	}
	return InstanceData;
}





#undef LOCTEXT_NAMESPACE

