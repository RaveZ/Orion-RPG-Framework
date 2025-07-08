// Copyright 2025 Ivan Chandra. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "OrionInteractionFunctionLibrary.generated.h"

class UOrionInteractionComponent;

UCLASS()
class ORIONRPG_API UOrionInteractionFunctionLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()
public:

	/**
	* Grab the Interaction component from the local pawn or player controller, whichever it exists on.
	*
	* @return The Interaction component.
	*/
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "OrionInteractionLibrary", meta = (WorldContext = "WorldContextObject"))
	static class UOrionInteractionComponent* GetOrionInteractionComponent(const UObject* WorldContextObject);

	/**
	* Find the Interaction component from the supplied target object.
	*
	* @return The Interaction component.
	*/
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "OrionInteractionLibrary", meta = (DefaultToSelf = "Target"))
	static class UOrionInteractionComponent* GetOrionInteractionComponentFromTarget(AActor* Target);

};
