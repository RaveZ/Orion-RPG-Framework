// Copyright 2025 Ivan Chandra. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "DialogBuilderFunctionLibrary.generated.h"

class UDialogComponent;

UCLASS()
class ORIONRPG_API UDialogBuilderFunctionLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()
public:

	/**
	* Grab the Dialog component from the local pawn or player controller, whichever it exists on.
	*
	* @return The Dialog component.
	*/
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "DialogBuilderLibrary", meta = (WorldContext = "WorldContextObject"))
	static class UDialogComponent* GetDialogComponent(const UObject* WorldContextObject);

	/**
	* Find the Dialog component from the supplied target object.
	*
	* @return The Dialog component.
	*/
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "DialogBuilderLibrary", meta = (DefaultToSelf = "Target"))
	static class UDialogComponent* GetDialogComponentFromTarget(AActor* Target);

	/**
	* Get the currently active dialog.
	* 
	* @return The Dialog Graph reference.
	*/
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "DialogBuilderLibrary", meta = (WorldContext = "WorldContextObject"))
	static class UDialogBuilderGraph* GetCurrentActiveDialog(const UObject* WorldContextObject);

	/**
	* Get the current dialog node.
	*
	* @return The Dialog Node reference.
	*/
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "DialogBuilderLibrary", meta = (WorldContext = "WorldContextObject"))
	static class UDialogBuilderNode* GetCurrentDialogNode(const UObject* WorldContextObject);

	/**
	* Get Last Selector Node.
	*
	* @return The Selector Dialog Node reference.
	*/
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "DialogBuilderLibrary", meta = (WorldContext = "WorldContextObject"))
	static class UDialogBuilderNode_RerouteNode* GetLastSelectorNode(const UObject* WorldContextObject);
};
