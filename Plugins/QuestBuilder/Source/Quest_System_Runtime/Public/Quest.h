// Copyright 2024 Ivan Chandra. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "UObject/NoExportTypes.h"
#include "Quest.generated.h"

class UQuestCondition;
class UQuestBuilderNode;
class UQuestBuilderEdge;
class UQuestBuilderNode_Root;

UENUM(BlueprintType)
enum class EQuestState : uint8
{
	/**Quest is Active*/
	E_Active		UMETA(DisplayName = "ACTIVE"),
	/**Quest is Completed*/
	E_Complete		UMETA(DisplayName = "COMPLETE"),
	/**Quest is Fail*/
	E_Fail			UMETA(DisplayName = "FAIL"),
	/**Quest is Locked, Waiting to be Unlocked*/
	E_Locked		UMETA(DisplayName = "LOCKED"),
	/**Quest is Unlocked, Waiting to be Activated*/
	E_Unlocked		UMETA(DisplayName = "UNLOCKED"),
};


UCLASS(Blueprintable)
class QUEST_SYSTEM_RUNTIME_API UQuest : public UObject
{
	GENERATED_BODY()
public:
	UQuest();

	UPROPERTY(VisibleDefaultsOnly, Category = "QuestAssets")
	TObjectPtr<UQuestBuilderGraph> QuestGraph;

	/** Quest ID */
	UPROPERTY(BlueprintReadOnly, VisibleDefaultsOnly, Category = "QuestSystem")
		FName QuestID;

	/** Your Quest Name*/
	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "QuestSystem", meta = (MultiLine = true))
		FText QuestName;

	/** Description for your quest*/
	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "QuestSystem", meta = (MultiLine = true))
		FText Description; 

	/**Make your quest can be aborted*/
	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "QuestConditions", meta = (DisplayName = "CanAbortQuest"))
		bool bCanQuestBeAborted = false;

	/**Make your quest can be aborted*/
	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "QuestConditions", meta = (DisplayName = "CanRetakeQuest"))
		bool bCanRetakeQuest = false;

	/**Conditions to change the QuestState to DesiredQuestState*/
	UPROPERTY(Instanced, EditAnywhere, BlueprintReadOnly, Category = "QuestConditions")
		TArray<TObjectPtr<UQuestCondition>> Conditions;

	/**
	* Change the Quest State to this value after condition met
	* If there is no condition, immediately change the quest state
	*/
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "QuestConditions")
		EQuestState DesiredQuestState = EQuestState::E_Active;

	/** List of All Visited Node IDs in Sequence*/
	UPROPERTY(BlueprintReadWrite, Category = "QuestSystem")
		TArray<FName> VisitedNodeIDs;

	/**Your quest state, will be changed based on condition*/
	UPROPERTY(BlueprintReadWrite, Category = "QuestSystem")
		EQuestState QuestState = EQuestState::E_Locked;
	
	UPROPERTY(BlueprintReadWrite, Category = "QuestSystem")
		TObjectPtr<UQuestBuilderNode> CurrentNode;

	UPROPERTY(BlueprintReadOnly, Category = "QuestSystem")
		TArray< TObjectPtr<UQuestBuilderNode>> RootNodes;

	UPROPERTY(BlueprintReadOnly, Category = "QuestSystem")
		TArray< TObjectPtr<UQuestBuilderNode>> AllNodes;

	UPROPERTY(BlueprintReadOnly, Category = "QuestSystem")
		TMap<FName, TObjectPtr<UQuestBuilderNode>> NodeMap;


	UFUNCTION(BlueprintCallable, Category = "QuestSystem")
		int GetLevelNum() const;

	UFUNCTION(BlueprintCallable, Category = "QuestSystem")
		void GetNodesByLevel(int Level, TArray<UQuestBuilderNode*>& Nodes);

	UFUNCTION(BlueprintCallable, Category = "QuestSystem")
		AController* GetOwningController();

	UFUNCTION(BlueprintCallable, Category = "QuestSystem")
		APawn* GetOwningPawn();

	UFUNCTION(BlueprintCallable, Category = "QuestSystem")
		UQuestComponent* GetQuestComponent();

	UFUNCTION(BlueprintCallable, Category = "QuestSystem")
		UQuestBuilderGraph* GetOwningQuestGraph() const;
	void ClearGraph();

#if WITH_EDITOR
protected:
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif

	

};

