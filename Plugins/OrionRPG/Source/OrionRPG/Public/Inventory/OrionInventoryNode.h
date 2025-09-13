// Copyright 2025 Ivan Chandra. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "Event/OrionEvent.h"
#include "OrionInventoryNode.generated.h"

class UOrionDecorator;
class UOrionEvent;


UENUM(BlueprintType)
enum class EOrionInventoryNodeLimits : uint8
{
	Unlimited,
	Limited
};


UCLASS(Blueprintable, EditInlineNew)
class ORIONRPG_API UOrionInventoryNode : public UObject
{
	GENERATED_BODY()

public:
	UOrionInventoryNode();
	virtual ~UOrionInventoryNode();

	UPROPERTY(BlueprintReadOnly, Category = "Detail", meta = (DisplayPriority = 0))
		FName ID;

	UPROPERTY(BlueprintReadOnly, Category = "OrionInventoryNode")
		TObjectPtr<UOrionInventoryGraph> OrionInventoryGraph;

	UPROPERTY(BlueprintReadWrite, Category = "OrionInventoryNode")
	TObjectPtr<AController> OwningController;

	UPROPERTY(BlueprintReadWrite, Category = "OrionInventoryNode")
	TObjectPtr < UOrionInventoryComponent> OrionInventoryComponent;

public:
	virtual void Initialize(bool bLaunchEventOnLoad = false);
	virtual void Deinitialize();

	virtual void Reset();
	virtual void BeginNode();
	
	/** initialize any asset related data */
	virtual void InitializeFromAsset(UOrionInventoryGraph& OwningOrionInventoryGraph);
	
	UFUNCTION(BlueprintCallable, Category = "OrionInventoryNode")
		AController* GetOwningController() const;

	UFUNCTION(BlueprintCallable, Category = "OrionInventoryNode")
		APawn* GetOwningPawn() const;

	UFUNCTION(BlueprintCallable, Category = "OrionInventoryNode")
		UOrionInventoryComponent* GetOrionInventoryComponent() const;

	UFUNCTION(BlueprintCallable, Category = "OrionInventoryNode")
		UOrionInventoryGraph* GetOwningOrionInventoryGraph() const;

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "OrionInventoryNode")
		FText GetDescription() const;

	virtual FText GetDescription_Implementation() const;

	/** returns short name of object's class (BTTaskNode_Wait -> Wait) */
	static FString GetShortTypeName(const UObject* Ob);
	
protected:
	TObjectPtr<UOrionInventoryNode> NextNode;
	TArray<TObjectPtr<UOrionEvent>> StartEventQueue;
	TArray<TObjectPtr<UOrionEvent>> EndEventQueue;

public:
	//////////////////////////////////////////////////////////////////////////
#if WITH_EDITORONLY_DATA


	UPROPERTY(BlueprintReadOnly, Category = "OrionInventoryNode")
		TSubclassOf<UOrionInventoryGraph> CompatibleGraphType;

	UPROPERTY(BlueprintReadOnly, Category = "OrionInventoryNode")
		FLinearColor BackgroundColor;


	UPROPERTY(BlueprintReadOnly, Category = "OrionInventoryNode")
		EOrionInventoryNodeLimits ParentLimitType;

	UPROPERTY(BlueprintReadOnly, Category = "OrionInventoryNode", meta = (ClampMin = "0", EditCondition = "ParentLimitType == EOrionInventoryNodeLimits::Limited", EditConditionHides))
		int32 ParentLimit;

	UPROPERTY(BlueprintReadOnly, Category = "OrionInventoryNode")
		EOrionInventoryNodeLimits ChildrenLimitType;

	UPROPERTY(BlueprintReadOnly, Category = "OrionInventoryNode", meta = (ClampMin = "0", EditCondition = "ChildrenLimitType == EOrionInventoryNodeLimits::Limited", EditConditionHides))
		int32 ChildrenLimit;

#endif

#if WITH_EDITOR
	virtual bool IsNameEditable() const;

	virtual FLinearColor GetBackgroundColor() const;

	virtual FText GetNodeTitle() const;

	virtual FText GetNodeDescription() const;

	virtual void SetNodeTitle(const FText& NewTitle);

	virtual bool CanCreateConnection(UOrionInventoryNode* Other, FText& ErrorMessage);

	virtual bool CanCreateConnectionTo(UOrionInventoryNode* Other, int32 NumberOfChildrenNodes, FText& ErrorMessage);
	virtual bool CanCreateConnectionFrom(UOrionInventoryNode* Other, int32 NumberOfParentNodes, FText& ErrorMessage);

	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
	virtual void PreEditChange(FProperty* PropertyAboutToChange) override;
#endif
};
