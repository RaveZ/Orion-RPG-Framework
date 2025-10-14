// Copyright 2025 Ivan Chandra. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "OrionInventoryFunctionLibrary.generated.h"

class UOrionInventoryItemFragment;
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

	UFUNCTION(BlueprintCallable, meta = (DeterminesOutputType = FragmentClass))
	static const UOrionInventoryItemFragment* FindItemDefinitionFragment(TSubclassOf<UOrionInventoryNode_ItemDefinition> ItemDef, TSubclassOf<UOrionInventoryItemFragment> FragmentClass);
};
