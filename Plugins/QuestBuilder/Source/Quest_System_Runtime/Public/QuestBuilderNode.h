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
enum class EDialogNodeLimits : uint8
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

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Detail", meta = (DisplayPriority = 0))
		FName ID;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Detail", meta = (MultiLine = true))
		FText Description;
	
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

	UFUNCTION(BlueprintCallable, Category = "QuestBuilderNode")
		bool IsLeafNode() const;

	virtual void Initialize();
	virtual void Deinitialize();

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
		FText GetDescription() const;

	virtual FText GetDescription_Implementation() const;

	virtual void BeginEvents(enum EEventRunType EventRunType);

	/** returns short name of object's class (BTTaskNode_Wait -> Wait) */
	static FString GetShortTypeName(const UObject* Ob);


	virtual void EvaluateUniqueID();
public:
	//////////////////////////////////////////////////////////////////////////
#if WITH_EDITORONLY_DATA


	UPROPERTY(BlueprintReadOnly, Category = "QuestBuilderNode")
		TSubclassOf<UQuestBuilderGraph> CompatibleGraphType;

	UPROPERTY(BlueprintReadOnly, Category = "QuestBuilderNode")
		FLinearColor BackgroundColor;


	UPROPERTY(BlueprintReadOnly, Category = "QuestBuilderNode")
		EDialogNodeLimits ParentLimitType;

	UPROPERTY(BlueprintReadOnly, Category = "QuestBuilderNode", meta = (ClampMin = "0", EditCondition = "ParentLimitType == EDialogNodeLimits::Limited", EditConditionHides))
		int32 ParentLimit;

	UPROPERTY(BlueprintReadOnly, Category = "QuestBuilderNode")
		EDialogNodeLimits ChildrenLimitType;

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
