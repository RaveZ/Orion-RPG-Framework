// Copyright 2025 Ivan Chandra. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "Event/OrionEvent.h"	
#include "DialogBuilderNode.generated.h"

class UDialogBuilderGraph;
class UOrionDecorator;
class UOrionEvent;
class UDialogBuilderEdge;


UENUM(BlueprintType)
enum class EDialogNodeLimits : uint8
{
	Unlimited,
	Limited
};

/**
 * Enum to define the backlinking behavior at the end of a dialogue node.
 */
UENUM(BlueprintType)
enum class EDialogueBacklinkRule : uint8
{
	EndDialogue UMETA(DisplayName = "End Dialogue"),
	BackToLastSelector UMETA(DisplayName = "Back to Last Selector"),
	JumpBackToNode UMETA(DisplayName = "Jump Back to Node")
};

DECLARE_MULTICAST_DELEGATE(FDialogNodeEventFinishedSignature);

UCLASS(Blueprintable, EditInlineNew)
class ORIONRPG_API UDialogBuilderNode : public UObject
{
	GENERATED_BODY()

public:
	UDialogBuilderNode();
	virtual ~UDialogBuilderNode();

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Detail", meta = (DisplayPriority = 0))
		FName ID;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Detail", meta = (MultiLine = true))
		FText Description;

	/**
	* When a dialogue node concludes without any next nodes,
	* The system provides the player with an option to revisit previous dialogue nodes.
	*/
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Back-Linking Rule")
		EDialogueBacklinkRule BackLinkRule;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Back-Linking Rule", meta = (EditCondition = "BackLinkRule == EDialogueBacklinkRule::JumpBackToNode", HideEditConditionToggle, EditConditionHides, GetOptions = "GetNodeIDOptions" ))
		FName NodeID;
	
	UPROPERTY(Instanced, BlueprintReadOnly, Category = "Events")
		TArray<TObjectPtr<UOrionEvent>> Events;

	UPROPERTY(Instanced, BlueprintReadOnly, Category = Decorators, meta = (DisplayName = "Decorators"))
		TArray<TObjectPtr<UOrionDecorator>> Decorators;

	UPROPERTY(BlueprintReadOnly, Category = "DialogBuilderNode")
		TObjectPtr<UDialogBuilderGraph> DialogGraph;

	UPROPERTY(BlueprintReadOnly, Category = "DialogBuilderNode")
		TArray<TObjectPtr<UDialogBuilderNode>> ParentNodes;

	UPROPERTY(BlueprintReadOnly, Category = "DialogBuilderNode")
		TArray<TObjectPtr<UDialogBuilderNode>> ChildrenNodes;

	UPROPERTY(BlueprintReadOnly, Category = "DialogBuilderNode")
		TMap<TObjectPtr<UDialogBuilderNode>, TObjectPtr<UDialogBuilderEdge>> Edges;

	UFUNCTION(BlueprintCallable, Category = "DialogBuilderNode")
		virtual UDialogBuilderEdge* GetEdge(UDialogBuilderNode* ChildNode);

	UPROPERTY(BlueprintReadWrite, Category = "DialogBuilderNode")
	TObjectPtr<AController> OwningController;

	UPROPERTY(BlueprintReadWrite, Category = "DialogBuilderNode")
	TObjectPtr < UDialogComponent> DialogComponent;
	
	/**Called when all Start Node events finished*/
	FDialogNodeEventFinishedSignature OnAllStartEventFinished;
	
	/**Called when all End Node events finished*/
	FDialogNodeEventFinishedSignature OnAllEndEventFinished;

public:
	virtual void Initialize(bool bLaunchEventOnLoad = false);
	virtual void Deinitialize();

	virtual void Reset();
	virtual void BeginNode();
	virtual void EvaluateNextNode();
	
	/** initialize any asset related data */
	virtual void InitializeFromAsset(UDialogBuilderGraph& OwningDialogGraph);

	UFUNCTION(BlueprintCallable, Category = "DialogBuilderNode")
	bool IsLeafNode() const;
	
	UFUNCTION(BlueprintCallable, Category = "DialogBuilderNode")
		AController* GetOwningController() const;

	UFUNCTION(BlueprintCallable, Category = "DialogBuilderNode")
		APawn* GetOwningPawn() const;

	UFUNCTION(BlueprintCallable, Category = "DialogBuilderNode")
		UDialogComponent* GetDialogComponent() const;

	UFUNCTION(BlueprintCallable, Category = "DialogBuilderNode")
		UDialogBuilderGraph* GetOwningDialogGraph() const;

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "DialogBuilderNode")
		FText GetDescription() const;

	virtual FText GetDescription_Implementation() const;
	
	virtual void BeginEvents(enum EEventLaunchType EventLaunchType);

	/** returns short name of object's class (BTTaskNode_Wait -> Wait) */
	static FString GetShortTypeName(const UObject* Ob);


	virtual void LaunchStartEventQueue();
	virtual void LaunchEndEventQueue();
	
	virtual void EvaluateUniqueID();
protected:
	UFUNCTION(CallInEditor)
	TArray<FName> GetNodeIDOptions();	
protected:
	TObjectPtr<UDialogBuilderNode> NextNode;
	TArray<TObjectPtr<UOrionEvent>> StartEventQueue;
	TArray<TObjectPtr<UOrionEvent>> EndEventQueue;

public:
	//////////////////////////////////////////////////////////////////////////
#if WITH_EDITORONLY_DATA


	UPROPERTY(BlueprintReadOnly, Category = "DialogBuilderNode")
		TSubclassOf<UDialogBuilderGraph> CompatibleGraphType;

	UPROPERTY(BlueprintReadOnly, Category = "DialogBuilderNode")
		FLinearColor BackgroundColor;


	UPROPERTY(BlueprintReadOnly, Category = "DialogBuilderNode")
		EDialogNodeLimits ParentLimitType;

	UPROPERTY(BlueprintReadOnly, Category = "DialogBuilderNode", meta = (ClampMin = "0", EditCondition = "ParentLimitType == EDialogNodeLimits::Limited", EditConditionHides))
		int32 ParentLimit;

	UPROPERTY(BlueprintReadOnly, Category = "DialogBuilderNode")
		EDialogNodeLimits ChildrenLimitType;

	UPROPERTY(BlueprintReadOnly, Category = "DialogBuilderNode", meta = (ClampMin = "0", EditCondition = "ChildrenLimitType == EDialogNodeLimits::Limited", EditConditionHides))
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

	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif
};
