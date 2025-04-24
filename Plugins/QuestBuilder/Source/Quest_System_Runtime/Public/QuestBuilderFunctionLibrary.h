// Copyright 2025 Ivan Chandra. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "QuestBuilderFunctionLibrary.generated.h"

/**
 * 
 */
UCLASS()
class QUEST_SYSTEM_RUNTIME_API UQuestBuilderFunctionLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()
public:
	UFUNCTION(CallInEditor, BlueprintCallable)
	static TArray<FName> GetAllQuestID();


	UFUNCTION(CallInEditor, BlueprintCallable)
	static TArray<FName> GetNodeIDFromCachedQuest();


};
