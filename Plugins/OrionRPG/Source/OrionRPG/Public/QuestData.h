// Copyright 2025 Ivan Chandra. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/SaveGame.h"
#include "Quest.h"
#include "UObject/NoExportTypes.h"
#include "Task/QuestTask.h"
#include "QuestData.generated.h"

/** Task Data To Be Saved */
USTRUCT()
struct FTaskData
{
	GENERATED_BODY()
public:
	UPROPERTY(SaveGame)
	int CurrentAmount = 0;

	UPROPERTY(SaveGame)
	ETaskState TaskState = ETaskState::E_Active;
};

/** Node Data To Be Saved */
USTRUCT()
struct FNodeData
{
	GENERATED_BODY()
public:
	UPROPERTY(SaveGame)
	FName NodeID;

	UPROPERTY(SaveGame)
	TArray<FTaskData> TaskDatas;
};



/** Quest Data To Be Saved */
USTRUCT()
struct FQuestData
{
	GENERATED_BODY()
public:
	UPROPERTY(SaveGame)
	FName CurrentNodeID;

	UPROPERTY(SaveGame)
	EQuestState QuestState = EQuestState::E_Locked;

	/** Map of Node ID and it's Node Data Struct */
	UPROPERTY(SaveGame)
	TMap<FName, FNodeData> NodeMapData;

	/** List of All Visited Node IDs in Sequence*/
	UPROPERTY(SaveGame)
	TArray<FName> VisitedNodeIDs;
};

USTRUCT(BlueprintType)
struct FQuestDataHandle
{
	GENERATED_BODY()

public:
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = QuestDataHandle, meta = (AllowPrivateAccess = true))
	FGameplayTag QuestTag;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = QuestDataHandle, meta = (AllowPrivateAccess = true))
	FGameplayTag NodeTag;


};
