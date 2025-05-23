// Copyright 2025 Ivan Chandra. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Quest.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "QuestBuilderFunctionLibrary.generated.h"

class UQuestComponent;

UCLASS()
class ORIONRPG_API UQuestBuilderFunctionLibrary : public UBlueprintFunctionLibrary
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
	* Find the Quest from the given Quest ID
	*
	* @return The Quest.
	*/
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "QuestBuilderLibrary", meta = (WorldContext = "WorldContextObject"))
	static class UQuest* FindQuest(const UObject* WorldContextObject, FName QuestID);

	/**
	* Check if the current objective valid from QuestID and NodeID.
	*
	* @return true if the quest id and node id match.
	*/
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "QuestBuilderLibrary", meta = (WorldContext = "WorldContextObject"))
	static bool CanProgressQuestObjective(const UObject* WorldContextObject, FName QuestID, FName NodeID);

	/**
	* Check if the quest at which state from the given Quest ID
	*
	* @return bool.
	*/
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "QuestBuilderLibrary", meta = (WorldContext = "WorldContextObject"))
	static bool IsQuestAtState(const UObject* WorldContextObject, FName ID, EQuestState QuestState);

	/**
	* Grab the Quest component from the local pawn or player controller, whichever it exists on.
	*
	* @return The Quest component.
	*/
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "QuestBuilderLibrary", meta = (WorldContext = "WorldContextObject"))
	static class UQuest* GetCurrentNavigatedQuest(const UObject* WorldContextObject);
};
