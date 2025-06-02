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
	/**Quest is Unlocked, Waiting to be Activated*/
	E_Unlocked		UMETA(DisplayName = "UNLOCKED"),
	/**Quest is Locked, Waiting to be Unlocked*/
	E_Locked		UMETA(DisplayName = "LOCKED"),
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FQuestObjectiveAdded, UQuestBuilderNode_Objective*, Objective);

UCLASS(Blueprintable, BlueprintType)
class ORIONRPG_API UQuest : public UObject
{
	GENERATED_BODY()
public:
	UQuest();

	UPROPERTY(VisibleDefaultsOnly, Category = "QuestAssets")
	TObjectPtr<UQuestBuilderGraph> QuestGraph;

	/** Quest ID */
	UPROPERTY(BlueprintReadOnly, Category = "Quest")
	FName ID;

	/**Category For this Quest*/
	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "Quest", meta = (GetOptions = "GetQuestCategoryList"))
	FName QuestCategory;
	
	/** Gameplay Tag for this quest, each tag should be unique*/
	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "Quest")
	FGameplayTag QuestTag;

	/** Your Quest Name*/
	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "Quest", meta = (MultiLine = true))
		FText QuestName;

	/** Description for your quest*/
	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "Quest", meta = (MultiLine = true))
		FText Description; 

	/**
	* Change the Quest State to this value the first time this quest registered
	*/
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Quest",  meta=(ValidEnumValues="E_Active, E_Unlocked, E_Locked"))
	EQuestState DesiredQuestState = EQuestState::E_Active;
	
	/**Can Abort Quest*/
	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "Quest", meta = (DisplayName = "CanAbortQuest"))
		bool bCanQuestBeAborted = false;

	/**Can Retake Quest*/
	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "Quest", meta = (DisplayName = "CanRetakeQuest"))
		bool bCanRetakeQuest = false;


	/** List of All Visited Node IDs in Sequence*/
	UPROPERTY(BlueprintReadWrite, Category = "Quest")
		TArray<FName> VisitedNodeIDs;

	/**Your quest state, will be changed based on condition*/
	UPROPERTY(BlueprintReadWrite, Category = "Quest")
		EQuestState QuestState = EQuestState::E_Locked;
	
	UPROPERTY(BlueprintReadWrite, Category = "Quest")
		TArray<TObjectPtr<UQuestBuilderNode>> CurrentNodes;

	UPROPERTY(BlueprintReadOnly, Category = "Quest")
		TArray< TObjectPtr<UQuestBuilderNode>> RootNodes;

	UPROPERTY(BlueprintReadOnly, Category = "Quest")
		TArray< TObjectPtr<UQuestBuilderNode>> AllNodes;

	UPROPERTY(BlueprintReadOnly, Category = "Quest")
		TMap<FName, TObjectPtr<UQuestBuilderNode>> NodeMap;

	UPROPERTY(BlueprintReadWrite, Category = "Quest")
		TObjectPtr<AController> OwningController;

	UPROPERTY(BlueprintReadWrite, Category = "Quest")
		TObjectPtr < UQuestComponent> QuestComponent;

	UPROPERTY(BlueprintAssignable, BlueprintCallable, Category = "QuestComponent")
		FQuestObjectiveAdded OnQuestObjectiveAdded;
public:

	virtual bool Initialize(class UQuestComponent* InitializingComp);
	virtual void Deinitialize();
	
	UFUNCTION(BlueprintCallable, Category = "Quest")
	void UpdateQuest();
	
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Quest")
	TArray<UQuestBuilderNode*> GetVisitedNodes();
	
	UFUNCTION(BlueprintCallable, Category = "Quest")
	void SortVisitedNodes();
	
	UFUNCTION(BlueprintCallable, Category = "Quest")
		void BeginNode(UQuestBuilderNode* InNode, bool bLaunchEventOnLoad = false);
	
	UFUNCTION(BlueprintCallable, Category = "Quest")
		void BeginMultipleNodes(TArray<UQuestBuilderNode*> InNodes, bool bLaunchEventOnLoad = false);
	
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

	UFUNCTION(CallInEditor, BlueprintCallable)
	virtual TArray<FName> GetQuestCategoryList();
	virtual void EvaluateUniqueID();

#if WITH_EDITOR
protected:
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif

	

};

