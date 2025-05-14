// Copyright 2025 Ivan Chandra. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Dialog.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "DialogBuilderFunctionLibrary.generated.h"

class UDialogComponent;

UCLASS()
class DIALOG_SYSTEM_RUNTIME_API UDialogBuilderFunctionLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()
public:
	UFUNCTION(CallInEditor, BlueprintCallable)
	static TArray<FName> GetAllDialogID();


	UFUNCTION(CallInEditor, BlueprintCallable)
	static TArray<FName> GetNodeIDFromCachedDialog();

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
	* Find the Dialog from the given Node ID
	*
	* @return The Dialog.
	*/
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "DialogBuilderLibrary", meta = (WorldContext = "WorldContextObject"))
	static class UDialog* FindDialogFromNodeID(const UObject* WorldContextObject, FName NodeID);

	/**
	* Check if the Current Node ID match with the given Node ID
	*
	* @return bool.
	*/
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "DialogBuilderLibrary", meta = (WorldContext = "WorldContextObject"))
	static bool IsCurrentNodeIDMatch(const UObject* WorldContextObject, FName NodeID);

	/**
	* Check if the dialog at which state from the given Dialog ID
	*
	* @return bool.
	*/
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "DialogBuilderLibrary", meta = (WorldContext = "WorldContextObject"))
	static bool IsDialogAtState(const UObject* WorldContextObject, FName ID, EDialogState DialogState);

	/**
	* Grab the Dialog component from the local pawn or player controller, whichever it exists on.
	*
	* @return The Dialog component.
	*/
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "DialogBuilderLibrary", meta = (WorldContext = "WorldContextObject"))
	static class UDialog* GetCurrentNavigatedDialog(const UObject* WorldContextObject);
};
