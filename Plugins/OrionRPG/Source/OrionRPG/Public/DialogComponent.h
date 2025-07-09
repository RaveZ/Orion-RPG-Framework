// Copyright 2025 Ivan Chandra. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "DialogComponent.generated.h"

class UDialogSaveObject;
class USaveGame;
class APlayerController;

//Dialog Delegate

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FUpdateDialog, class UDialogBuilderNode*, DialogNode);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FEnterOptionSelection, const TArray<class UDialogBuilderNode_PlayerOption*>&, PlayerOptions);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FPlayerOptionSelected, UDialogBuilderNode_PlayerOption*, PlayerOption);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FBeginDialog, UDialogBuilderGraph*, Dialog);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FEndDialog,UDialogBuilderGraph*, Dialog);

UCLASS(meta=(BlueprintSpawnableComponent), Blueprintable, HideDropdown, ClassGroup = "OrionRPG", DisplayName = "Orion Dialog Component")
class ORIONRPG_API UDialogComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UDialogComponent();

	//We cache the OwningController, we won't cache pawn as this might change
	UPROPERTY(BlueprintReadOnly, Category = "DialogComponent")
		TObjectPtr<APlayerController> OwningController;

	/** Current Dialog That is Running, only one dialog should active at a time*/
	UPROPERTY(BlueprintReadWrite, Category = "DialogComponent")
		TObjectPtr<UDialogBuilderGraph> CurrentActiveDialog;

	UPROPERTY(BlueprintReadWrite, Category = "DialogComponent")
		TMap<FName, TObjectPtr<UDialogBuilderGraph>> DialogMap;
	
	UPROPERTY(BlueprintAssignable, BlueprintCallable, Category = "DialogComponent")
		FUpdateDialog OnDialogUpdated;
	
	UPROPERTY(BlueprintAssignable, BlueprintCallable, Category = "DialogComponent")
		FEnterOptionSelection OnEnterOptionSelection;
	
	UPROPERTY(BlueprintAssignable, BlueprintCallable, Category = "DialogComponent")
		FPlayerOptionSelected OnPLayerOptionSelected;

	UPROPERTY(BlueprintAssignable, BlueprintCallable, Category = "DialogComponent")
		FBeginDialog OnBeginDialog;
	
	UPROPERTY(BlueprintAssignable, BlueprintCallable, Category = "DialogComponent")
		FEndDialog OnEndDialog;
public:
	UFUNCTION()
	virtual void DialogLineUpdated(class UDialogBuilderNode* DialogNode);
	
	UFUNCTION()
	virtual void EnterOptionSelection(const TArray<class UDialogBuilderNode_PlayerOption*>& PlayerOptions);
	
	UFUNCTION()
	virtual void PlayerOptionSelected(class UDialogBuilderNode_PlayerOption* PlayerOption);
	
	UFUNCTION()
	virtual void DialogBegin(UDialogBuilderGraph* Dialog);
	
	UFUNCTION()
	virtual void DialogEnd(UDialogBuilderGraph* Dialog);
	
	UFUNCTION(BlueprintCallable, Category = "DialogComponent")
		bool BeginDialog(UDialogBuilderGraph* DialogAsset);

	UFUNCTION(BlueprintCallable, Category = "DialogComponent")
		void SelectDialogOption(class UDialogBuilderNode_PlayerOption* InOption);

	UFUNCTION()
		virtual class UDialogBuilderGraph* MakeDialogGraphInstance(UDialogBuilderGraph* DialogGraphTemplate);

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
