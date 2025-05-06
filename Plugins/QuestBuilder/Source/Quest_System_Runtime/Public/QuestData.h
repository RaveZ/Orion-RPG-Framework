// Copyright 2024 Ivan Chandra. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/SaveGame.h"
#include "Quest.h"
#include "Task/QuestTask.h"
#include "QuestData.generated.h"

/** Task Data To Be Saved */
USTRUCT(BlueprintType)
struct FTaskData
{
	GENERATED_BODY()
public:
	UPROPERTY(BlueprintReadWrite, EditAnywhere, meta = (DisplayName = "CurrentAmount", MakeStructureDefaultValue = "0"), Category = TaskData)
	int CurrentAmount = 0;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = TaskData)
	ETaskState TaskState = ETaskState::E_Active;
};

/** Event Data To Be Saved */
USTRUCT(BlueprintType)
struct FEventData
{
	GENERATED_BODY()
public:
	UPROPERTY(BlueprintReadWrite, EditAnywhere, meta = (DisplayName = "StartLaunched?"), Category = EventData)
	bool StartLaunched = false;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, meta = (DisplayName = "EndLaunched?"), Category = EventData)
	bool EndLaunched = false;
};

/** Node Data To Be Saved */
USTRUCT(BlueprintType)
struct FNodeData
{
	GENERATED_BODY()
public:
	UPROPERTY(BlueprintReadWrite, EditAnywhere, meta = (DisplayName = "NodeID", MakeStructureDefaultValue = "None"), Category = NodeData)
	FName NodeID;


	UPROPERTY(BlueprintReadWrite, EditAnywhere, meta = (DisplayName = "EventDatas"), Category = NodeData)
	TArray<FEventData> EventDatas;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, meta = (DisplayName = "TaskDatas"), Category = NodeData)
	TArray<FTaskData> TaskDatas;
};



/** Quest Data To Be Saved */
USTRUCT(BlueprintType)
struct FQuestData
{
	GENERATED_BODY()
public:
	UPROPERTY(BlueprintReadWrite, EditAnywhere, meta = (DisplayName = "QuestID", MakeStructureDefaultValue = "None"), Category = QuestData)
	FName CurrentNodeID;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, meta = (DisplayName = "QuestState"), Category = QuestData)
	EQuestState QuestState = EQuestState::E_Locked;

	/** Map of Node ID and it's Node Data Struct */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, meta = (DisplayName = "NodeMapData"), Category = QuestData)
	TMap<FName, FNodeData> NodeMapData;

	/** List of All Visited Node IDs in Sequence*/
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = QuestData)
	TArray<FName> VisitedNodeIDs;
};

USTRUCT(BlueprintType)
struct FQuestDataHandle
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = QuestDataHandle, meta = (GetOptions = "Quest_System_Runtime.QuestBuilderFunctionLibrary.GetAllQuestID"))
	FName QuestID;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = QuestDataHandle, meta = (GetOptions = "Quest_System_Runtime.QuestBuilderFunctionLibrary.GetNodeIDFromCachedQuest"))
	FName NodeID;

	/** Returns true if this handle is specifically pointing to nothing */
	bool IsNull() const
	{
		return QuestID.IsNone() && NodeID.IsNone();
	}

	QUEST_SYSTEM_RUNTIME_API void PostSerialize(const FArchive& Ar);
};

template<>
struct TStructOpsTypeTraits< FQuestDataHandle > : public TStructOpsTypeTraitsBase2< FQuestDataHandle >
{
	enum
	{
		WithPostSerialize = true,
	};
};

