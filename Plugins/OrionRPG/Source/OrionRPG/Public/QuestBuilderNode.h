// Copyright 2025 Ivan Chandra. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "Event/OrionEvent.h"	
#include "QuestBuilderNode.generated.h"

class UQuestBuilderGraph;
class UQuest;
class UOrionDecorator;
class UOrionEvent;
class UQuestBuilderEdge;


UENUM(BlueprintType)
enum class EQuestNodeLimits : uint8
{
	Unlimited,
	Limited
};


DECLARE_MULTICAST_DELEGATE(FNodeEventFinishedSignature);

UCLASS(Blueprintable, EditInlineNew)
class ORIONRPG_API UQuestBuilderNode : public UObject
{
	GENERATED_BODY()

public:
	UQuestBuilderNode();
	virtual ~UQuestBuilderNode();

	/**Asset ID*/
	UPROPERTY(BlueprintReadOnly, Category = "Description", meta = (DisplayPriority = 0))
		FName ID;

	/** Gameplay Tag for this node, each tag should be unique */
	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "Description", DisplayName = "Node Tag (Optional)", meta = (Categories = "Quest"))
		FGameplayTag NodeTag;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Description", meta = (MultiLine = true))
		FText Description;

	/** Add this Node Description to Journal? */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Description", DisplayName = "Add To Journal?")
		bool bAddToJournal;

	/** node name override*/
	UPROPERTY(Category = "Description", EditAnywhere)
		FString NodeName;
	
	UPROPERTY(Instanced, BlueprintReadOnly, Category = "Events")
		TArray<TObjectPtr<UOrionEvent>> Events;

	UPROPERTY(Instanced, BlueprintReadOnly, Category = Decorators, meta = (DisplayName = "Decorators"))
		TArray<TObjectPtr<UOrionDecorator>> Decorators;

	UPROPERTY(BlueprintReadOnly, Category = "QuestBuilderNode")
		TObjectPtr<UQuestBuilderGraph> QuestGraph;

	UPROPERTY(BlueprintReadOnly, Category = "QuestBuilderNode")
		TObjectPtr<UQuest> Quest;

	UPROPERTY(BlueprintReadOnly, Category = "QuestBuilderNode")
		TArray<TObjectPtr<UQuestBuilderNode>> ParentNodes;

	UPROPERTY(BlueprintReadOnly, Category = "QuestBuilderNode")
		TArray<TObjectPtr<UQuestBuilderNode>> ChildrenNodes;

	UPROPERTY(BlueprintReadOnly, Category = "QuestBuilderNode")
		TMap<TObjectPtr<UQuestBuilderNode>, TObjectPtr<UQuestBuilderEdge>> Edges;

	UFUNCTION(BlueprintCallable, Category = "QuestBuilderNode")
		virtual UQuestBuilderEdge* GetEdge(UQuestBuilderNode* ChildNode);

	UPROPERTY(BlueprintReadWrite, Category = "Quest")
		TObjectPtr<AController> OwningController;

	UPROPERTY(BlueprintReadWrite, Category = "Quest")
		TObjectPtr < UQuestComponent> QuestComponent;

	/**Called when all Start Node events finished*/
	FNodeEventFinishedSignature OnAllStartEventFinished;
	
	/**Called when all End Node events finished*/
	FNodeEventFinishedSignature OnAllEndEventFinished;

	UFUNCTION(BlueprintCallable, Category = "QuestBuilderNode")
		bool IsLeafNode() const;

	virtual void Initialize(bool bLaunchEventOnLoad = false);
	virtual void Deinitialize();

	virtual void Reset();
	
	virtual void BeginNode();
	virtual void EvaluateNextNode();
	
	/** initialize any asset related data */
	virtual void InitializeFromAsset(UQuestBuilderGraph& OwningQuestGraph, UQuest& OwningQuest);


	UFUNCTION(BlueprintCallable, Category = "QuestBuilderNode")
		AController* GetOwningController() const;

	UFUNCTION(BlueprintCallable, Category = "QuestBuilderNode")
		APawn* GetOwningPawn() const;

	UFUNCTION(BlueprintCallable, Category = "QuestBuilderNode")
		UQuestComponent* GetQuestComponent() const;

	UFUNCTION(BlueprintCallable, Category = "QuestBuilderNode")
		UQuestBuilderGraph* GetOwningQuestGraph() const;

	UFUNCTION(BlueprintCallable, Category = "QuestBuilderNode")
		UQuest* GetOwningQuest() const;

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "QuestBuilderNode")
		FText GetNodeDisplayName() const;

	virtual FText GetNodeDisplayName_Implementation() const;

	virtual void LaunchStartEventQueue();
	virtual void LaunchEndEventQueue();

	/** returns short name of object's class (BTTaskNode_Wait -> Wait) */
	static FString GetShortTypeName(const UObject* Ob);

	// Allows the Object to get a valid UWorld from it's outer.
	virtual UWorld* GetWorld() const override;

	UFUNCTION(BlueprintCallable, Category = "QuestBuilderNode")
	FString GetShortTag(FGameplayTag Tag, int32 Level = 1) const;
protected:
	TArray<UQuestBuilderNode*> NextNodes;
	TArray<UOrionEvent*> StartEventQueue;
	TArray<UOrionEvent*> EndEventQueue;

public:
	//////////////////////////////////////////////////////////////////////////
#if WITH_EDITORONLY_DATA


	UPROPERTY(BlueprintReadOnly, Category = "QuestBuilderNode")
		TSubclassOf<UQuestBuilderGraph> CompatibleGraphType;

	UPROPERTY(BlueprintReadOnly, Category = "QuestBuilderNode")
		FLinearColor BackgroundColor;


	UPROPERTY(BlueprintReadOnly, Category = "QuestBuilderNode")
		EQuestNodeLimits ParentLimitType;

	UPROPERTY(BlueprintReadOnly, Category = "QuestBuilderNode", meta = (ClampMin = "0", EditCondition = "ParentLimitType == EDialogNodeLimits::Limited", EditConditionHides))
		int32 ParentLimit;

	UPROPERTY(BlueprintReadOnly, Category = "QuestBuilderNode")
		EQuestNodeLimits ChildrenLimitType;

	UPROPERTY(BlueprintReadOnly, Category = "QuestBuilderNode", meta = (ClampMin = "0", EditCondition = "ChildrenLimitType == EDialogNodeLimits::Limited", EditConditionHides))
		int32 ChildrenLimit;


#endif

#if WITH_EDITOR
	virtual bool IsNameEditable() const;

	virtual FLinearColor GetBackgroundColor() const;

	virtual FText GetNodeTitle() const;

	virtual FText GetNodeDescription() const;

	virtual FText GetEventsText() const;

	virtual FText GetConditionsText() const;

	virtual void SetNodeTitle(const FText& NewTitle);

	virtual bool CanCreateConnection(UQuestBuilderNode* Other, FText& ErrorMessage);

	virtual bool CanCreateConnectionTo(UQuestBuilderNode* Other, int32 NumberOfChildrenNodes, FText& ErrorMessage);
	virtual bool CanCreateConnectionFrom(UQuestBuilderNode* Other, int32 NumberOfParentNodes, FText& ErrorMessage);

	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif
};
