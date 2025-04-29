// Copyright 2024 Ivan Chandra. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "UObject/NoExportTypes.h"
#include "QuestSystem.generated.h"

class UQuestCondition;
class UQuestSystemGraphNode;
class UQuestSystemGraphEdge;
class UQuestSystemGraphNode_Root;

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
class QUEST_SYSTEM_RUNTIME_API UQuestSystem : public UObject
{
	GENERATED_BODY()
public:
	UQuestSystem();

	UPROPERTY(VisibleDefaultsOnly, Category = "QuestAssets")
	TObjectPtr<UQuestSystemGraph> QuestGraph;

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
	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "QuestConditions")
		bool bCanQuestBeAborted = false;
	/**
	* TRUE : activate this quest immediately
	* FALSE : Use Quest Condition To Activate/Unlock the Quest
	*/
	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "QuestConditions", meta = (DisplayName = "AutoActivateQuest?"))
		bool bAutoActivateQuest = true;

	/**Conditions to unlock the Quest*/
	UPROPERTY(Instanced, EditAnywhere, BlueprintReadOnly, Category = "QuestConditions", meta = (EditCondition = "bAutoActivateQuest == false", EditConditionHides))
		TArray<TObjectPtr<UQuestCondition>> Conditions;

	/**
	* TRUE : Unlock Quest
	* FALSE : Activate Quest Immediately
	*/
	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "QuestConditions", meta = (EditCondition = "bAutoActivateQuest == false", EditConditionHides, DisplayName = "UnlockQuest?"))
		bool bUnlockQuest = false;


	/** List of All Visited Node IDs in Sequence*/
	UPROPERTY(BlueprintReadWrite, Category = "QuestSystem")
		TArray<FName> VisitedNodeIDs;

	/**Your quest state, will be changed based on condition*/
	UPROPERTY(BlueprintReadWrite, Category = "QuestSystem")
		EQuestState QuestState = EQuestState::E_Locked;
	
	UPROPERTY(BlueprintReadWrite, Category = "QuestSystem")
		TObjectPtr<UQuestSystemGraphNode> CurrentNode;

	UPROPERTY(BlueprintReadOnly, Category = "QuestSystem")
		TArray< TObjectPtr<UQuestSystemGraphNode>> RootNodes;

	UPROPERTY(BlueprintReadOnly, Category = "QuestSystem")
		TArray< TObjectPtr<UQuestSystemGraphNode>> AllNodes;

	UPROPERTY(BlueprintReadOnly, Category = "QuestSystem")
		TMap<FName, TObjectPtr<UQuestSystemGraphNode>> NodeMap;


	UFUNCTION(BlueprintCallable, Category = "QuestSystem")
		int GetLevelNum() const;

	UFUNCTION(BlueprintCallable, Category = "QuestSystem")
		void GetNodesByLevel(int Level, TArray<UQuestSystemGraphNode*>& Nodes);

	UFUNCTION(BlueprintCallable, Category = "QuestSystem")
		AController* GetOwningController();

	UFUNCTION(BlueprintCallable, Category = "QuestSystem")
		APawn* GetOwningPawn();

	UFUNCTION(BlueprintCallable, Category = "QuestSystem")
		UQuestComponent* GetQuestComponent();

	UFUNCTION(BlueprintCallable, Category = "QuestSystem")
		UQuestSystemGraph* GetOwningQuestGraph() const;
	void ClearGraph();

#if WITH_EDITOR
protected:
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif

	

};

