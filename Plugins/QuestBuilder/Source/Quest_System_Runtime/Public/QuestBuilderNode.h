// Copyright 2024 Ivan Chandra. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "QuestBuilderNode.generated.h"

class UQuestBuilderGraph;
class UQuest;
class UQuestCondition;
class UQuestEvent;
class UQuestBuilderEdge;

UENUM(BlueprintType)
enum class ENodeLimits : uint8
{
	Unlimited,
	Limited
};



UCLASS(Blueprintable, EditInlineNew)
class QUEST_SYSTEM_RUNTIME_API UQuestBuilderNode : public UObject
{
	GENERATED_BODY()

public:
	UQuestBuilderNode();
	virtual ~UQuestBuilderNode();

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "Detail")
		FName ID;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Detail", meta = (MultiLine = true))
		FText Description;
	
	UPROPERTY(Instanced, EditAnywhere, BlueprintReadOnly, Category = "Events")
		TArray<TObjectPtr<UQuestEvent>> Events;

	UPROPERTY(Instanced, EditAnywhere, BlueprintReadOnly, Category = BranchingConditions, meta = (DisplayName = "Branching Conditions"))
		TArray<TObjectPtr<UQuestCondition>> Conditions;

	UPROPERTY(BlueprintReadOnly, Category = "QuestSystemGraphNode")
		TObjectPtr<UQuestBuilderGraph> QuestGraph;

	UPROPERTY(BlueprintReadOnly, Category = "QuestSystemGraphNode")
		TObjectPtr<UQuest> QuestSystem;

	UPROPERTY(BlueprintReadOnly, Category = "QuestSystemGraphNode")
		TArray<TObjectPtr<UQuestBuilderNode>> ParentNodes;

	UPROPERTY(BlueprintReadOnly, Category = "QuestSystemGraphNode")
		TArray<TObjectPtr<UQuestBuilderNode>> ChildrenNodes;

	UPROPERTY(BlueprintReadOnly, Category = "QuestSystemGraphNode")
		TMap<TObjectPtr<UQuestBuilderNode>, TObjectPtr<UQuestBuilderEdge>> Edges;


	UFUNCTION(BlueprintCallable, Category = "QuestSystemGraphNode")
		virtual UQuestBuilderEdge* GetEdge(UQuestBuilderNode* ChildNode);

	UFUNCTION(BlueprintCallable, Category = "QuestSystemGraphNode")
		bool IsLeafNode() const;


	UFUNCTION(BlueprintCallable, Category = "QuestSystemGraphNode")
		AController* GetOwningController() const;

	UFUNCTION(BlueprintCallable, Category = "QuestSystemGraphNode")
		APawn* GetOwningPawn() const;

	UFUNCTION(BlueprintCallable, Category = "QuestSystemGraphNode")
		UQuestComponent* GetQuestComponent() const;

	UFUNCTION(BlueprintCallable, Category = "QuestSystemGraphNode")
		UQuestBuilderGraph* GetOwningQuestGraph() const;

	UFUNCTION(BlueprintCallable, Category = "QuestSystemGraphNode")
		UQuest* GetOwningQuestSystem() const;

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "QuestSystemGraphNode")
		FText GetDescription() const;


	virtual FText GetDescription_Implementation() const;


	/** returns short name of object's class (BTTaskNode_Wait -> Wait) */
	static FString GetShortTypeName(const UObject* Ob);


	//////////////////////////////////////////////////////////////////////////
#if WITH_EDITORONLY_DATA


	UPROPERTY(BlueprintReadOnly, Category = "QuestSystemGraphNode")
		TSubclassOf<UQuestBuilderGraph> CompatibleGraphType;

	UPROPERTY(BlueprintReadOnly, Category = "QuestSystemGraphNode")
		FLinearColor BackgroundColor;


	UPROPERTY(BlueprintReadOnly, Category = "QuestSystemGraphNode")
		ENodeLimits ParentLimitType;

	UPROPERTY(BlueprintReadOnly, Category = "QuestSystemGraphNode", meta = (ClampMin = "0", EditCondition = "ParentLimitType == ENodeLimits::Limited", EditConditionHides))
		int32 ParentLimit;

	UPROPERTY(BlueprintReadOnly, Category = "QuestSystemGraphNode")
		ENodeLimits ChildrenLimitType;

	UPROPERTY(BlueprintReadOnly, Category = "QuestSystemGraphNode", meta = (ClampMin = "0", EditCondition = "ChildrenLimitType == ENodeLimits::Limited", EditConditionHides))
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
#endif
};
