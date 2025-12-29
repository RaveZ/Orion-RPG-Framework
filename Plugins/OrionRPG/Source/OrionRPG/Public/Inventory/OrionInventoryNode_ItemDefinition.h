// Copyright 2025 Ivan Chandra. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "OrionInventoryNode.h"
#include "Inventory/InventoryData.h"
#include "NativeGameplayTags.h"
#include "OrionInventoryNode_ItemDefinition.generated.h"

class UOrionInventoryItemFragment;
//////////////////////////////////////////////////////////////////////

/**
 * UOrionInventoryNode_ItemDefinition
 */
UCLASS(Blueprintable, Const, AutoExpandCategories = "Fragments")
class ORIONRPG_API UOrionInventoryNode_ItemDefinition : public UOrionInventoryNode
{
	GENERATED_BODY()

public:
	UOrionInventoryNode_ItemDefinition();

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Inventory", meta = (UIMin = "1"))
	FIntPoint GridSize{ 1, 1 };

	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (Categories = "Item"), Category = "Inventory")
	FGameplayTag Tag;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Inventory")
	FText DisplayName;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Inventory")
	FText Description;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Inventory")
	TObjectPtr<UTexture2D> Icon;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (UIMin = "1"), Category = "Inventory")
	int32 MaxStackCount;

	UPROPERTY(EditAnywhere, meta = (Units = "Kilograms", UIMin = "0"), Category = "Inventory")
	float Weight;


	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Fragments", Instanced, meta = (FullyExpand = true))
	TArray<TObjectPtr<UOrionInventoryItemFragment>> Fragments;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fragments", meta = (InlineEditConditionToggle))
	bool bCanDropItem = true;

	/**Override for drop item fragment in case for different behavior*/
	UPROPERTY(EditAnywhere, Category = "Fragments", Instanced, meta = (EditCondition = "bCanDropItem"))
	TObjectPtr<class UOrionInventoryFragment_DropItem> DropItemFragmentOverride;

	UPROPERTY(BlueprintReadOnly, Category = "Fragments")
	TObjectPtr<UOrionInventoryFragment_DropItem> DropItemFragment;

public:
	void LoadPrerequisites();
	const UOrionInventoryItemFragment* FindFragmentByClass(TSubclassOf<UOrionInventoryItemFragment> FragmentClass) const;
	UOrionInventoryFragment_DropItem* GetDropItemFragment();

public:
#if WITH_EDITOR

	//Node Setting
	virtual FText GetNodeTitle() const override;
	virtual void SetNodeTitle(const FText& NewTitle);
	virtual FText GetNodeDescription() const override;
#endif
};
