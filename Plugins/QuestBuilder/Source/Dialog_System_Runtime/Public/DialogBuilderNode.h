// Copyright 2025 Ivan Chandra. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "Event/OrionEvent.h"	
#include "DialogBuilderNode.generated.h"

class UDialogBuilderGraph;
class UDialog;
class UOrionDecorator;
class UOrionEvent;
class UDialogBuilderEdge;


UENUM(BlueprintType)
enum class ENodeLimits : uint8
{
	Unlimited,
	Limited
};



UCLASS(Blueprintable, EditInlineNew)
class DIALOG_SYSTEM_RUNTIME_API UDialogBuilderNode : public UObject
{
	GENERATED_BODY()

public:
	UDialogBuilderNode();
	virtual ~UDialogBuilderNode();

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Detail")
		FName ID;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Detail", meta = (MultiLine = true))
		FText Description;
	
	UPROPERTY(Instanced, EditAnywhere, BlueprintReadOnly, Category = "Events")
		TArray<TObjectPtr<UOrionEvent>> Events;

	UPROPERTY(Instanced, EditAnywhere, BlueprintReadOnly, Category = BranchingConditions, meta = (DisplayName = "Branching Decorators"))
		TArray<TObjectPtr<UOrionDecorator>> Decorators;

	UPROPERTY(BlueprintReadOnly, Category = "DialogBuilderNode")
		TObjectPtr<UDialogBuilderGraph> DialogGraph;

	UPROPERTY(BlueprintReadOnly, Category = "DialogBuilderNode")
		TObjectPtr<UDialog> Dialog;

	UPROPERTY(BlueprintReadOnly, Category = "DialogBuilderNode")
		TArray<TObjectPtr<UDialogBuilderNode>> ParentNodes;

	UPROPERTY(BlueprintReadOnly, Category = "DialogBuilderNode")
		TArray<TObjectPtr<UDialogBuilderNode>> ChildrenNodes;

	UPROPERTY(BlueprintReadOnly, Category = "DialogBuilderNode")
		TMap<TObjectPtr<UDialogBuilderNode>, TObjectPtr<UDialogBuilderEdge>> Edges;

	UFUNCTION(BlueprintCallable, Category = "DialogBuilderNode")
		virtual UDialogBuilderEdge* GetEdge(UDialogBuilderNode* ChildNode);

	UFUNCTION(BlueprintCallable, Category = "DialogBuilderNode")
		bool IsLeafNode() const;

	virtual void Initialize();

	/** initialize any asset related data */
	virtual void InitializeFromAsset(UDialogBuilderGraph& OwningDialogGraph, UDialog& OwningDialog);


	UFUNCTION(BlueprintCallable, Category = "DialogBuilderNode")
		AController* GetOwningController() const;

	UFUNCTION(BlueprintCallable, Category = "DialogBuilderNode")
		APawn* GetOwningPawn() const;

	UFUNCTION(BlueprintCallable, Category = "DialogBuilderNode")
		UDialogComponent* GetDialogComponent() const;

	UFUNCTION(BlueprintCallable, Category = "DialogBuilderNode")
		UDialogBuilderGraph* GetOwningDialogGraph() const;

	UFUNCTION(BlueprintCallable, Category = "DialogBuilderNode")
		UDialog* GetOwningDialog() const;

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "DialogBuilderNode")
		FText GetDescription() const;

	virtual FText GetDescription_Implementation() const;

	virtual void BeginEvents(enum EEventRunType EventRunType);

	/** returns short name of object's class (BTTaskNode_Wait -> Wait) */
	static FString GetShortTypeName(const UObject* Ob);


	//////////////////////////////////////////////////////////////////////////
#if WITH_EDITORONLY_DATA


	UPROPERTY(BlueprintReadOnly, Category = "DialogBuilderNode")
		TSubclassOf<UDialogBuilderGraph> CompatibleGraphType;

	UPROPERTY(BlueprintReadOnly, Category = "DialogBuilderNode")
		FLinearColor BackgroundColor;


	UPROPERTY(BlueprintReadOnly, Category = "DialogBuilderNode")
		ENodeLimits ParentLimitType;

	UPROPERTY(BlueprintReadOnly, Category = "DialogBuilderNode", meta = (ClampMin = "0", EditCondition = "ParentLimitType == ENodeLimits::Limited", EditConditionHides))
		int32 ParentLimit;

	UPROPERTY(BlueprintReadOnly, Category = "DialogBuilderNode")
		ENodeLimits ChildrenLimitType;

	UPROPERTY(BlueprintReadOnly, Category = "DialogBuilderNode", meta = (ClampMin = "0", EditCondition = "ChildrenLimitType == ENodeLimits::Limited", EditConditionHides))
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

	virtual bool CanCreateConnection(UDialogBuilderNode* Other, FText& ErrorMessage);

	virtual bool CanCreateConnectionTo(UDialogBuilderNode* Other, int32 NumberOfChildrenNodes, FText& ErrorMessage);
	virtual bool CanCreateConnectionFrom(UDialogBuilderNode* Other, int32 NumberOfParentNodes, FText& ErrorMessage);
#endif
};
