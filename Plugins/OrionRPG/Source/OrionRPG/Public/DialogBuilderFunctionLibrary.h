// Copyright 2025 Ivan Chandra. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Dialog.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "DialogBuilderFunctionLibrary.generated.h"

class UDialogComponent;

UCLASS()
class ORIONRPG_API UDialogBuilderFunctionLibrary : public UBlueprintFunctionLibrary
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
	* Find the Dialog from the given Dialog ID
	*
	* @return The Dialog.
	*/
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "DialogBuilderLibrary", meta = (WorldContext = "WorldContextObject"))
	static class UDialog* FindDialog(const UObject* WorldContextObject, FName DialogID);

	/**
	* Check if the current objective valid from DialogID and NodeID.
	*
	* @return true if the dialog id and node id match.
	*/
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "DialogBuilderLibrary", meta = (WorldContext = "WorldContextObject"))
	static bool CanProgressDialogObjective(const UObject* WorldContextObject, FName DialogID, FName NodeID);

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
