// Copyright 2025 Ivan Chandra. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "QuestBuilderFunctionLibrary.generated.h"

class UQuestComponent;

UCLASS()
class QUEST_SYSTEM_RUNTIME_API UQuestBuilderFunctionLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()
public:
	UFUNCTION(CallInEditor, BlueprintCallable)
	static TArray<FName> GetAllQuestID();


	UFUNCTION(CallInEditor, BlueprintCallable)
	static TArray<FName> GetNodeIDFromCachedQuest();

	/**
	* Grab the Quest component from the local pawn or player controller, whichever it exists on.
	*
	* @return The Quest component.
	*/
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "QuestBuilder", meta = (WorldContext = "WorldContextObject"))
	static class UQuestComponent* GetQuestComponent(const UObject* WorldContextObject);

	/**
	* Find the Quest component from the supplied target object.
	*
	* @return The Quest component.
	*/
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "QuestBuilder", meta = (DefaultToSelf = "Target"))
	static class UQuestComponent* GetQuestComponentFromTarget(AActor* Target);


};
