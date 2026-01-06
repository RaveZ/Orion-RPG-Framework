// Copyright 2025 Ivan Chandra. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "Inventory/OrionInventoryItemFragment.h"
#include "Templates/SubclassOf.h"
#include "OrionInventoryFunctionLibrary.generated.h"

class UOrionInventoryItemFragment;
class UOrionEquipmentInstance;
class UOrionInventoryNode_ItemDefinition;

UCLASS()
class ORIONRPG_API UOrionInventoryFunctionLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()
public:

	/**
	* Grab the Inventory component from the local pawn or player controller, whichever it exists on.
	*
	* @return The Inventory component.
	*/
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "OrionLibrary|Inventory", meta = (WorldContext = "WorldContextObject"))
	static class UOrionInventoryComponent* GetInventoryComponent(const UObject* WorldContextObject);

	/**
	* Find the Inventory component from the supplied target object.
	*
	* @return The Inventory component.
	*/
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "OrionLibrary|Inventory", meta = (DefaultToSelf = "Target"))
	static class UOrionInventoryComponent* GetInventoryComponentFromTarget(AActor* Target);

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "OrionLibrary|Inventory", meta = (WorldContext = "WorldContextObject"))
	static class UOrionInventoryGraph* GetInventoryDB(const UObject* WorldContextObject);

	UFUNCTION(BlueprintCallable, Category = "OrionLibrary|Inventory", meta = (DeterminesOutputType = FragmentClass))
	static const UOrionInventoryItemFragment* FindItemDefinitionFragment(TSubclassOf<UOrionInventoryNode_ItemDefinition> ItemDef, TSubclassOf<UOrionInventoryItemFragment> FragmentClass);

	/**
	 * Add Item to Inventory
	 *
	 * @param InOptionalInstigator: Optional Instigator, determine to whom this item we should give on replicated network
	 * @param ItemTag:				The Item Tag to add
	 * @param StackCount:			The Stack Count to add
	 *
	 * @return	StackRemainder:	The Remainder Stack Count that couldn't be added (if any)
	 * @return	ItemData:			ItemInstanceData
	 */
	UFUNCTION(BlueprintCallable, Category = "OrionLibrary|Inventory", meta = (WorldContext = "WorldContextObject"))
	static void AddInventoryItem(const UObject* WorldContextObject, int32& StackRemainder, FItemInstanceData& OutItemData, AActor* InOptionalInstigator, UPARAM(meta = (Categories = "Item")) FGameplayTag ItemTag, int32 StackCount = 1);

	/**
	 * Drop Item from Inventory
	 *
	 * @param InOptionalInstigator: Optional Instigator, determine to whom this item we should give on replicated network
	 * @param ItemTag:				The Item Tag to drop
	 * @param StackCount:			The Stack Count to drop
	 *
	 * @return	ItemData:			ItemInstanceData
	 */
	UFUNCTION(BlueprintCallable, Category = "OrionLibrary|Inventory", meta = (WorldContext = "WorldContextObject"))
	static void DropInventoryItem(const UObject* WorldContextObject, FItemInstanceData& OutItemData, AActor* InOptionalInstigator, UPARAM(meta = (Categories = "Item")) FGameplayTag ItemTag, int32 StackCount = 1);

	/**
	 * Get Item Instance data
	 *
	 * @param InOptionalInstigator: Optional Instigator
	 * @param ItemTag:				The Item Tag
	 *
	 * @return	bSuccess:			return true if item found in Inventory List
	 * @return	ItemData:			ItemInstanceData
	 */
	UFUNCTION(BlueprintPure, Category = "OrionLibrary|Inventory", meta = (WorldContext = "WorldContextObject"))
	static bool GetItemData(const UObject* WorldContextObject, FItemInstanceData& OutItemData, AActor* InOptionalInstigator, UPARAM(meta = (Categories = "Item")) FGameplayTag ItemTag);


	/**
	 * Assign Item Index
	 *
	 */
	UFUNCTION(BlueprintCallable , Category = "OrionLibrary|Inventory", meta = (WorldContext = "WorldContextObject"))
	static void AssignItemIndex(const UObject* WorldContextObject, UOrionInventoryItemInstance* ItemInstance, int32 TileIndex);


	UFUNCTION(BlueprintCallable, Category = "OrionLibrary|Inventory", meta = (WorldContext = "WorldContextObject"))
	static void AssignGridItemToSlots(const UObject* WorldContextObject, class UOrionGridItem* InGridItem, int32 StartingIndex);

	UFUNCTION(BlueprintCallable, Category = "OrionLibrary|Inventory", meta = (WorldContext = "WorldContextObject"))
	static void CreateDragItem(const UObject* WorldContextObject, class UOrionInventoryItemInstance* InItemInstance, int32 FromIndex);

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "OrionLibrary|Inventory", meta = (WorldContext = "WorldContextObject"))
	static EOrionInventoryType GetInventoryType(const UObject* WorldContextObject);

	UFUNCTION(BlueprintCallable, Category = "OrionLibrary|Inventory", meta = (WorldContext = "WorldContextObject"))
	static TArray<UOrionInventoryItemInstance*> GetAllInventoryItem(const UObject* WorldContextObject);

	UFUNCTION(BlueprintCallable, Category = "OrionLibrary|Inventory", meta = (WorldContext = "WorldContextObject"))
	static UOrionEquipmentInstance* EquipItem(const UObject* WorldContextObject, FOrionEquipmentDefinition EquipmentDefinition);

	UFUNCTION(BlueprintCallable, Category = "OrionLibrary|Inventory", meta = (WorldContext = "WorldContextObject"))
	static void UnequipItem(const UObject* WorldContextObject, UOrionEquipmentInstance* EquipmentInstance);

	UFUNCTION(BlueprintCallable, Category = "OrionLibrary|Inventory", meta = (WorldContext = "WorldContextObject"))
	static void UnequipItemFromSlot(const UObject* WorldContextObject, UPARAM(meta = (Categories = "EquipSlot")) FGameplayTag InEquipSlot);

	UFUNCTION(BlueprintCallable, Category = "OrionLibrary|Inventory", meta = (WorldContext = "WorldContextObject"))
	static void LootItemFromActor(const UObject* WorldContextObject, AActor* InActor, AActor* InOptionalInstigator);

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "OrionLibrary|Inventory", meta = (WorldContext = "WorldContextObject"))
	static UOrionInventoryNode_ItemDefinition* FindItemDefinitionByTag(const UObject* WorldContextObject, UPARAM(meta = (Categories = "Item")) FGameplayTag ItemTag);

	UFUNCTION(BlueprintCallable, Category = "OrionLibrary|Inventory", meta = (WorldContext = "WorldContextObject"))
	static bool GetCurrency(const UObject* WorldContextObject, UPARAM(meta = (Categories = "Currency"))FGameplayTag InCurrencyTag, FOrionCurrencyData& OutCurrency, AActor* InOptionalInstigator);

	UFUNCTION(BlueprintCallable, Category = "OrionLibrary|Inventory", meta = (WorldContext = "WorldContextObject"))
	static bool AddCurrency(const UObject* WorldContextObject, UPARAM(meta = (Categories = "Currency"))FGameplayTag InCurrencyTag, int32 Value, FOrionCurrencyData& OutCurrency, AActor* InOptionalInstigator);

};
