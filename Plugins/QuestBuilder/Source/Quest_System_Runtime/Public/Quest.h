// Copyright 2025 Ivan Chandra. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "UObject/NoExportTypes.h"
#include "Quest.generated.h"

class UOrionDecorator;
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
	UPROPERTY(BlueprintReadOnly, VisibleInstanceOnly, Category = "Quest")
		FName ID;

	/** Your Quest Name*/
	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "Quest", meta = (MultiLine = true))
		FText QuestName;

	/** Description for your quest*/
	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "Quest", meta = (MultiLine = true))
		FText Description; 

	/**Make your quest can be aborted*/
	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "QuestConditions", meta = (DisplayName = "CanAbortQuest"))
		bool bCanQuestBeAborted = false;

	/**Make your quest can be aborted*/
	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "QuestConditions", meta = (DisplayName = "CanRetakeQuest"))
		bool bCanRetakeQuest = false;

	/**Conditions to change the QuestState to DesiredQuestState*/
	UPROPERTY(Instanced, EditAnywhere, BlueprintReadOnly, Category = "QuestConditions")
		TArray<TObjectPtr<UOrionDecorator>> Conditions;

	/**
	* Change the Quest State to this value after condition met
	* If there is no condition, immediately change the quest state
	*/
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "QuestConditions")
		EQuestState DesiredQuestState = EQuestState::E_Active;

	/** List of All Visited Node IDs in Sequence*/
	UPROPERTY(BlueprintReadWrite, Category = "Quest")
		TArray<FName> VisitedNodeIDs;

	/**Your quest state, will be changed based on condition*/
	UPROPERTY(BlueprintReadWrite, Category = "Quest")
		EQuestState QuestState = EQuestState::E_Locked;
	
	UPROPERTY(BlueprintReadWrite, Category = "Quest")
		TObjectPtr<UQuestBuilderNode> CurrentNode;

	UPROPERTY(BlueprintReadOnly, Category = "Quest")
		TArray< TObjectPtr<UQuestBuilderNode>> RootNodes;

	UPROPERTY(BlueprintReadOnly, Category = "Quest")
		TArray< TObjectPtr<UQuestBuilderNode>> AllNodes;

	UPROPERTY(BlueprintReadOnly, Category = "Quest")
		TMap<FName, TObjectPtr<UQuestBuilderNode>> NodeMap;

public:

	virtual bool Initialize(class UQuestComponent* InitializingComp);
	virtual void Deinitialize();

	UFUNCTION(BlueprintCallable, Category = "Quest")
		int GetLevelNum() const;

	UFUNCTION(BlueprintCallable, Category = "Quest")
		void GetNodesByLevel(int Level, TArray<UQuestBuilderNode*>& Nodes);

	UFUNCTION(BlueprintCallable, Category = "Quest")
		AController* GetOwningController();

	UFUNCTION(BlueprintCallable, Category = "Quest")
		APawn* GetOwningPawn();

	UFUNCTION(BlueprintCallable, Category = "Quest")
		UQuestComponent* GetQuestComponent();

	UFUNCTION(BlueprintCallable, Category = "Quest")
		UQuestBuilderGraph* GetOwningQuestGraph() const;
	void ClearGraph();

#if WITH_EDITOR
protected:
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif

	

};

