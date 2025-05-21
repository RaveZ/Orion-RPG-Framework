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
	/**Wrapper Data to pick quest id using picker*/
	UPROPERTY(Instanced, EditAnywhere, BlueprintReadWrite, Category = "Quest Data Handle", meta = (FullyExpand = true, AllowPrivateAccess = true))
	TObjectPtr<UQuestHandle> QuestDataHandle;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = QuestDataHandle, meta = (AllowPrivateAccess = true))
	FName QuestID;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = QuestDataHandle, meta = (AllowPrivateAccess = true))
	FName NodeID;

	// Constructor
	FQuestDataHandle();

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


UCLASS(Blueprintable, EditInlineNew, DefaultToInstanced, AutoExpandCategories = ("Default"))
class QUEST_SYSTEM_RUNTIME_API UQuestHandle : public UObject
{
	GENERATED_BODY()

	public:
		UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = QuestDataHandle, meta = (GetOptions = "GetAllQuestID"))
		FName QuestID;

		UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = QuestDataHandle, meta = (GetOptions = "GetAllNodeID"))
		FName NodeID;


		UFUNCTION(CallInEditor, BlueprintCallable)
		virtual TArray<FName> GetAllQuestID();

		UFUNCTION(CallInEditor)
		virtual TArray<FName> GetAllNodeID();
};