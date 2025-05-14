// Copyright 2025 Ivan Chandra. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/SaveGame.h"
#include "Quest.h"
#include "Task/QuestTask.h"
#include "QuestData.h"
#include "QuestSaveObject.generated.h"



UCLASS(Blueprintable)
class QUEST_SYSTEM_RUNTIME_API UQuestSaveObject : public USaveGame
{
	GENERATED_BODY()
public:

	/** Map of Quest ID and it's Quest Data Struct */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = QuestSaveData)
		TMap<FName,FQuestData> QuestMapData;
};
