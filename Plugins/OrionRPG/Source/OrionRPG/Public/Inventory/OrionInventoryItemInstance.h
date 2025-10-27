// Copyright 2025 Ivan Chandra. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "OrionGameplayTagStack.h"
#include "Inventory/InventoryData.h"
#include "Inventory/OrionInventoryNode.h"
#include "Templates/SubclassOf.h"
#include "OrionInventoryItemInstance.generated.h"

class UOrionInventoryNode_ItemDefinition;
class UOrionInventoryItemFragment;

UCLASS(BlueprintType)
class ORIONRPG_API UOrionInventoryItemInstance : public UOrionInventoryNode
{
	GENERATED_BODY()

protected:
	UOrionInventoryItemInstance();

	//~UObject interface
	virtual bool IsSupportedForNetworking() const override { return true; }
	//~End of UObject interface

public:
	UOrionInventoryNode_ItemDefinition* GetItemDef() const { return ItemDef; }
	template <typename ResultClass>
	const ResultClass* FindFragmentByClass() const { return (ResultClass*)FindFragmentByClass(ResultClass::StaticClass()); }

	void SetItemDef(UOrionInventoryNode_ItemDefinition* InDef);

	int32 GetTotalStackCount() const { return TotalStackCount; }
	bool HasRoomForItem() const;
	void SetTotalStackCount(int32 Count);

public:
	// Adds a specified number of stacks to the tag (does nothing if StackCount is below 1)
	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category = Inventory)
	void AddStatTagStack(FGameplayTag Tag, int32 StackCount);

	// Removes a specified number of stacks from the tag (does nothing if StackCount is below 1)
	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category = Inventory)
	void RemoveStatTagStack(FGameplayTag Tag, int32 StackCount);

	// Returns the stack count of the specified tag (or 0 if the tag is not present)
	UFUNCTION(BlueprintCallable, Category = Inventory)
	int32 GetStatTagStackCount(FGameplayTag Tag) const;

	// Returns true if there is at least one stack of the specified tag
	UFUNCTION(BlueprintCallable, Category = Inventory)
	bool HasStatTag(FGameplayTag Tag) const;

	UFUNCTION(BlueprintCallable, BlueprintPure = false, meta = (DeterminesOutputType = FragmentClass))
	const UOrionInventoryItemFragment* FindFragmentByClass(TSubclassOf<UOrionInventoryItemFragment> FragmentClass) const;

	UFUNCTION(BlueprintPure, Category = Inventory)
	FItemInstanceData GetItemInstanceData() const;

private:
#if UE_WITH_IRIS
	/** Register all replication fragments */
	virtual void RegisterReplicationFragments(UE::Net::FFragmentRegistrationContext& Context, UE::Net::EFragmentRegistrationFlags RegistrationFlags) override;
#endif // UE_WITH_IRIS


	friend struct FLyraInventoryList;

private:
	UPROPERTY(Replicated)
	int32 TotalStackCount{0};

	UPROPERTY(Replicated)
	FOrionGameplayTagStackContainer StatTags;

	// The item definition
	UPROPERTY(Replicated)
	TObjectPtr<UOrionInventoryNode_ItemDefinition> ItemDef;


};
