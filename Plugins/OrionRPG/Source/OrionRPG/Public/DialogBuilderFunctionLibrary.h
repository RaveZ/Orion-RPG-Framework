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
};
