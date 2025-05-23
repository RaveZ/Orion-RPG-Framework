// Copyright 2025 Ivan Chandra. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "UObject/NoExportTypes.h"
#include "Dialog.generated.h"

class UOrionDecorator;
class UDialogBuilderNode;
class UDialogBuilderEdge;
class UDialogBuilderNode_Root;

UENUM(BlueprintType)
enum class EDialogState : uint8
{
	/**Dialog is Active*/
	E_Active		UMETA(DisplayName = "ACTIVE"),
	/**Dialog is Completed*/
	E_Complete		UMETA(DisplayName = "COMPLETE"),
	/**Dialog is Fail*/
	E_Fail			UMETA(DisplayName = "FAIL"),
	/**Dialog is Locked, Waiting to be Unlocked*/
	E_Locked		UMETA(DisplayName = "LOCKED"),
	/**Dialog is Unlocked, Waiting to be Activated*/
	E_Unlocked		UMETA(DisplayName = "UNLOCKED"),
};


UCLASS(Blueprintable)
class ORIONRPG_API UDialog : public UObject
{
	GENERATED_BODY()
public:
	UDialog();

	UPROPERTY(VisibleDefaultsOnly, Category = "DialogAssets")
	TObjectPtr<UDialogBuilderGraph> DialogGraph;

	/** Dialog ID */
	UPROPERTY(BlueprintReadOnly, VisibleInstanceOnly, Category = "Dialog")
		FName ID;

	/** Your Dialog Name*/
	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "Dialog", meta = (MultiLine = true))
		FText DialogName;

	/** Description for your dialog*/
	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "Dialog", meta = (MultiLine = true))
		FText Description; 

	/**Make your dialog can be aborted*/
	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "DialogConditions", meta = (DisplayName = "CanAbortDialog"))
		bool bCanDialogBeAborted = false;

	/**Make your dialog can be aborted*/
	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "DialogConditions", meta = (DisplayName = "CanRetakeDialog"))
		bool bCanRetakeDialog = false;

	/**Decorators to change the DialogState to DesiredDialogState*/
	UPROPERTY(Instanced, EditAnywhere, BlueprintReadOnly, Category = "DialogConditions")
		TArray<TObjectPtr<UOrionDecorator>> Decorators;

	/**
	* Change the Dialog State to this value after condition met
	* If there is no condition, immediately change the dialog state
	*/
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "DialogConditions")
		EDialogState DesiredDialogState = EDialogState::E_Active;

	/** List of All Visited Node IDs in Sequence*/
	UPROPERTY(BlueprintReadWrite, Category = "Dialog")
		TArray<FName> VisitedNodeIDs;

	/**Your dialog state, will be changed based on condition*/
	UPROPERTY(BlueprintReadWrite, Category = "Dialog")
		EDialogState DialogState = EDialogState::E_Locked;
	
	UPROPERTY(BlueprintReadWrite, Category = "Dialog")
		TObjectPtr<UDialogBuilderNode> CurrentNode;

	UPROPERTY(BlueprintReadOnly, Category = "Dialog")
		TArray< TObjectPtr<UDialogBuilderNode>> RootNodes;

	UPROPERTY(BlueprintReadOnly, Category = "Dialog")
		TArray< TObjectPtr<UDialogBuilderNode>> AllNodes;

	UPROPERTY(BlueprintReadOnly, Category = "Dialog")
		TMap<FName, TObjectPtr<UDialogBuilderNode>> NodeMap;

public:

	virtual bool Initialize(class UDialogComponent* InitializingComp);
	virtual void Deinitialize();

	UFUNCTION(BlueprintCallable, Category = "Dialog")
		int GetLevelNum() const;

	UFUNCTION(BlueprintCallable, Category = "Dialog")
		void GetNodesByLevel(int Level, TArray<UDialogBuilderNode*>& Nodes);

	UFUNCTION(BlueprintCallable, Category = "Dialog")
		AController* GetOwningController();

	UFUNCTION(BlueprintCallable, Category = "Dialog")
		APawn* GetOwningPawn();

	UFUNCTION(BlueprintCallable, Category = "Dialog")
		UDialogComponent* GetDialogComponent();

	UFUNCTION(BlueprintCallable, Category = "Dialog")
		UDialogBuilderGraph* GetOwningDialogGraph() const;
	void ClearGraph();

	virtual void EvaluateUniqueID();

#if WITH_EDITOR
protected:
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif

	

};

