// Copyright 2025 Ivan Chandra. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "QuestSystem.h"
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
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "QuestBuilderLibrary", meta = (WorldContext = "WorldContextObject"))
	static class UQuestComponent* GetQuestComponent(const UObject* WorldContextObject);

	/**
	* Find the Quest component from the supplied target object.
	*
	* @return The Quest component.
	*/
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "QuestBuilderLibrary", meta = (DefaultToSelf = "Target"))
	static class UQuestComponent* GetQuestComponentFromTarget(AActor* Target);

	/**
	* Find the Quest from the given Node ID
	*
	* @return The Quest.
	*/
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "QuestBuilderLibrary", meta = (WorldContext = "WorldContextObject"))
	static class UQuestSystem* FindQuestFromNodeID(const UObject* WorldContextObject, FName NodeID);

	/**
	* Check if the Current Node ID match with the given Node ID
	*
	* @return bool.
	*/
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "QuestBuilderLibrary", meta = (WorldContext = "WorldContextObject"))
	static bool IsCurrentNodeIDMatch(const UObject* WorldContextObject, FName NodeID);

	/**
	* Check if the quest at which state from the given Quest ID
	*
	* @return bool.
	*/
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "QuestBuilderLibrary", meta = (WorldContext = "WorldContextObject"))
	static bool IsQuestAtState(const UObject* WorldContextObject, FName QuestID, EQuestState QuestState);

	/**
	* Grab the Quest component from the local pawn or player controller, whichever it exists on.
	*
	* @return The Quest component.
	*/
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "QuestBuilderLibrary", meta = (WorldContext = "WorldContextObject"))
	static class UQuestSystem* GetCurrentNavigatedQuest(const UObject* WorldContextObject);
};
