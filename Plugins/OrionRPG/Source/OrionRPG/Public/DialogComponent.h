// Copyright 2025 Ivan Chandra. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "DialogComponent.generated.h"

class UDialogSaveObject;
class USaveGame;
class APlayerController;

//Dialog Delegate



UCLASS(meta=(BlueprintSpawnableComponent), Blueprintable, HideDropdown, ClassGroup = "DialogBuilder", DisplayName = "Dialog Component")
class ORIONRPG_API UDialogComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UDialogComponent();

	//We cache the OwningController, we won't cache pawn as this might change
	UPROPERTY(BlueprintReadOnly, Category = "DialogComponent")
		TObjectPtr<APlayerController> OwningController;

	/** Array of DialogGraph that is Running*/
	UPROPERTY(BlueprintReadWrite, Category = "DialogComponent")
		TArray< TObjectPtr<UDialogBuilderGraph>> DialogGraphs;

	/** Map of Dialog ID and attached Dialog That is Running*/
	UPROPERTY(BlueprintReadWrite, Category = "DialogComponent")
		TMap<FName, TObjectPtr<UDialog>> DialogMap;

	UPROPERTY(BlueprintReadWrite, Category = "DialogComponent")
		TObjectPtr<UDialog> CurrentNavigatedDialog;

public:


	UFUNCTION()
		virtual void DialogObjectiveUpdated();

	UFUNCTION()
		virtual void StartNavigatingDialog(UDialog* Dialog);

	UFUNCTION()
		virtual void DialogFailed(UDialog* Dialog);

	UFUNCTION()
		virtual void DialogCompleted(UDialog* Dialog);

	UFUNCTION()
		virtual void DialogAdded(UDialog* Dialog);

	UFUNCTION()
		virtual void DialogUnlocked(UDialog* Dialog);

	UFUNCTION()
		virtual void BeginSave(FString SaveName);

	UFUNCTION()
		virtual void BeginLoad(FString SaveName);

	UFUNCTION()
		virtual void SaveComplete(FString SaveName);

	UFUNCTION()
		virtual void LoadComplete(FString SaveName);
public:
	UFUNCTION(BlueprintCallable, Category = "DialogComponent")
		bool BeginDialog(UDialogBuilderGraph* DialogAsset);

	UFUNCTION(BlueprintCallable, Category = "DialogComponent")
		bool BeginMultipleDialog(TArray<UDialogBuilderGraph*>  DialogAssets);

	UFUNCTION()
		void CacheDialogGraphDependencies(UDialogBuilderGraph* DialogGraph);

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "DialogComponent")
		class UDialogBuilderGraph* GetDialogGraphInstance(class UDialogBuilderGraph* DialogGraph) const;

	UFUNCTION()
		virtual class UDialogBuilderGraph* MakeDialogGraphInstance(UDialogBuilderGraph* DialogGraphTemplate);

	UFUNCTION(BlueprintCallable, Category = "DialogComponent")
		void ActivateDialog(FName ID);

	UFUNCTION(BlueprintCallable, Category = "DialogComponent")
		void UnlockDialog(FName ID = NAME_None);

	UFUNCTION(BlueprintCallable, Category = "DialogComponent")
		void LockDialog(UDialog* InDialog = nullptr, FName ID = NAME_None);

	UFUNCTION(BlueprintCallable, Category = "DialogComponent")
		void RestartDialog(FName ID, FName NodeID = NAME_None, bool bStartFromNodeID = false);

	UFUNCTION(BlueprintCallable, Category = "DialogComponent")
		void FailDialog(FName ID);

	UFUNCTION(BlueprintCallable, Category = "DialogComponent")
		void CompleteDialog(FName ID);

	UFUNCTION(BlueprintCallable, Category = "DialogComponent")
		void StartNavigateDialog(UDialog* InDialog);


	UFUNCTION(BlueprintCallable, Category = "DialogComponent")
		UDialog* FindDialogFromNodeID(FName NodeIDParam) const;

	UFUNCTION(BlueprintCallable, Category = "DialogComponent")
		UDialog* FindDialog(FName DialogIDParam) const;



	UFUNCTION(BlueprintCallable, Category = "DialogComponent")
		bool IsDialogActiveFromNodeID(FName NodeID) const;

	UFUNCTION(BlueprintCallable, Category = "DialogComponent")
		bool IsDialogActiveFromDialogID(FName ID) const;

	UFUNCTION(BlueprintCallable, Category = "DialogComponent")
		void UpdateDialog();

	UFUNCTION(BlueprintCallable, Category = "DialogComponent")
		void ResetAllDialog();

	UFUNCTION(BlueprintCallable, Category = "DialogComponent")
		void EvaluateNextNodeCondition(UDialog* DialogParam);

	UFUNCTION(BlueprintCallable, Category = "DialogComponent")
		void EvaluateDialogCondition(UDialog* DialogParam);

	UFUNCTION(BlueprintCallable, Category = "DialogComponent")
		void EvaluateDialogState(UDialog* DialogParam, int SumConditionMet);

	UFUNCTION(BlueprintCallable, Category = "DialogComponent")
		void GoToNextNode(UDialog* DialogParam, UDialogBuilderNode* NodeParam);

	UFUNCTION(BlueprintCallable, Category = "DialogComponent")
		void BeginNode(UDialog* DialogParam);

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, BlueprintAuthorityOnly, Category = "Saving")
		void Save(const FString& SaveFileName, int SlotIndex);

	virtual void Save_Implementation(const FString& SaveFileName, int SlotIndex);
	
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, BlueprintAuthorityOnly, Category = "Saving")
		void Load(const FString& SaveFileName, int SlotIndex);

	virtual void Load_Implementation(const FString& SaveFileName, int SlotIndex);

	UFUNCTION(BlueprintCallable, Category = "Saving")
	virtual bool DeleteSave(const FString& SaveName = "DialogBuilderSaveData", const int32 Slot = 0);

public:
	UFUNCTION(BlueprintPure, Category = "DialogComponent")
	virtual APawn* GetOwningPawn() const;

	UFUNCTION(BlueprintPure, Category = "DialogComponent")
	virtual APlayerController* GetOwningController() const;


protected:
	// Called when the game starts
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

		
};
