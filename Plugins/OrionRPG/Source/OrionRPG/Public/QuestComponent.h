// Copyright 2025 Ivan Chandra. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Components/ActorComponent.h"
#include "QuestComponent.generated.h"

class UQuestSaveObject;
class UQuest;
class USaveGame;
class APlayerController;

//Quest Delegate
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FQuestObjectiveUpdated);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FStartNavigateQuest, UQuest*, Quest);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FQuestFailed, UQuest*, Quest);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FQuestCompleted, UQuest*, Quest);
	
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FQuestAdded, UQuest*, Quest);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FQuestUnlocked, UQuest*, Quest);

//Save/Load Delegate
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnBeginSave, FString, SaveFileName);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnSaveComplete, FString, SaveFileName);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnBeginLoad, FString, SaveFileName);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnLoadComplete, FString, SaveFileName);

UCLASS(meta=(BlueprintSpawnableComponent), Blueprintable, HideDropdown, ClassGroup = "QuestBuilder", DisplayName = "Quest Component")
class ORIONRPG_API UQuestComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UQuestComponent();

	//We cache the OwningController, we won't cache pawn as this might change
	UPROPERTY(BlueprintReadOnly, Category = "QuestComponent")
		TObjectPtr<APlayerController> OwningController;

	/** Array of QuestGraph that is Running*/
	UPROPERTY(BlueprintReadWrite, Category = "QuestComponent")
		TArray< TObjectPtr<UQuestBuilderGraph>> QuestGraphs;

	/** Map of Quest ID and attached Quest That is Running*/
	UPROPERTY(BlueprintReadWrite, Category = "QuestComponent")
		TMap<FName, TObjectPtr<UQuest>> QuestMap;

	UPROPERTY(BlueprintReadWrite, Category = "QuestComponent")
		TObjectPtr<UQuest> CurrentNavigatedQuest;

public:
	UPROPERTY(BlueprintAssignable, BlueprintCallable, Category = "QuestComponent")
		FQuestObjectiveUpdated OnQuestObjectiveUpdated;

	UPROPERTY(BlueprintAssignable, BlueprintCallable, Category = "QuestComponent")
		FStartNavigateQuest OnStartNavigateQuest;

	UPROPERTY(BlueprintAssignable, BlueprintCallable, Category = "QuestComponent")
		FQuestFailed OnQuestFailed;

	UPROPERTY(BlueprintAssignable, BlueprintCallable, Category = "QuestComponent")
		FQuestCompleted OnQuestCompleted;

	UPROPERTY(BlueprintAssignable, BlueprintCallable, Category = "QuestComponent")
		FQuestAdded OnQuestAdded;

	UPROPERTY(BlueprintAssignable, BlueprintCallable, Category = "QuestComponent")
		FQuestAdded OnQuestUnlocked;

	/**Called when a save has begun*/
	UPROPERTY(BlueprintAssignable, BlueprintCallable, Category = "Saving/Loading")
		FOnBeginSave OnBeginSave;

	/**Called when a save has completed*/
	UPROPERTY(BlueprintAssignable, BlueprintCallable, Category = "Saving/Loading")
		FOnSaveComplete OnSaveComplete;

	/**Called when a load has begun*/
	UPROPERTY(BlueprintAssignable, BlueprintCallable, Category = "Saving/Loading")
		FOnBeginLoad OnBeginLoad;

	/**Called when a load has completed*/
	UPROPERTY(BlueprintAssignable, BlueprintCallable, Category = "Saving/Loading")
		FOnLoadComplete OnLoadComplete;

	UFUNCTION()
		virtual void QuestObjectiveUpdated();

	UFUNCTION()
		virtual void StartNavigatingQuest(UQuest* Quest);

	UFUNCTION()
		virtual void QuestFailed(UQuest* Quest);

	UFUNCTION()
		virtual void QuestCompleted(UQuest* Quest);

	UFUNCTION()
		virtual void QuestAdded(UQuest* Quest);

	UFUNCTION()
		virtual void QuestUnlocked(UQuest* Quest);

	UFUNCTION()
		virtual void BeginSave(FString SaveName);

	UFUNCTION()
		virtual void BeginLoad(FString SaveName);

	UFUNCTION()
		virtual void SaveComplete(FString SaveName);

	UFUNCTION()
		virtual void LoadComplete(FString SaveName);
public:
	UFUNCTION(BlueprintCallable, Category = "QuestComponent")
		bool BeginQuestGraph(UQuestBuilderGraph* QuestGraphAsset);

	UFUNCTION(BlueprintCallable, Category = "QuestComponent")
		bool BeginMultipleQuestGraph(TArray<UQuestBuilderGraph*>  QuestGraphAsset);
	
	UFUNCTION()
		virtual class UQuest* MakeQuestInstance(UQuest* QuestTemplate);

	UFUNCTION(BlueprintCallable, Category = "QuestComponent")
		void ActivateQuestFromTag(FGameplayTag QuestTag);

	UFUNCTION(BlueprintCallable, Category = "QuestComponent")
		void UnlockQuestFromTag(FGameplayTag QuestTag);

	UFUNCTION(BlueprintCallable, Category = "QuestComponent")
		void LockQuestFromTag(FGameplayTag QuestTag);

	UFUNCTION(BlueprintCallable, Category = "QuestComponent")
		void RestartQuestFromTag(FGameplayTag QuestTag, FGameplayTag NodeTag = FGameplayTag(), bool bStartFromNodeTag = false);

	UFUNCTION(BlueprintCallable, Category = "QuestComponent")
		void FailQuestFromTag(FGameplayTag QuestTag);

	UFUNCTION(BlueprintCallable, Category = "QuestComponent")
		void CompleteQuestFromTag(FGameplayTag QuestTag);

	UFUNCTION(BlueprintCallable, Category = "QuestComponent")
		void StartNavigateQuest(UQuest* InQuest);

	UFUNCTION(BlueprintCallable, Category = "QuestComponent")
		void ActivateQuest(UQuest* InQuest);

	UFUNCTION(BlueprintCallable, Category = "QuestComponent")
		void UnlockQuest(UQuest* InQuest);

	UFUNCTION(BlueprintCallable, Category = "QuestComponent")
		void LockQuest(UQuest* InQuest);

	UFUNCTION(BlueprintCallable, Category = "QuestComponent")
		void RestartQuest(UQuest* InQuest);

	UFUNCTION(BlueprintCallable, Category = "QuestComponent")
		void FailQuest(UQuest* InQuest);

	UFUNCTION(BlueprintCallable, Category = "QuestComponent")
		void CompleteQuest(UQuest* InQuest);

	UFUNCTION(BlueprintCallable, Category = "QuestComponent")
		bool IsQuestAtState(FGameplayTag QuestTag, enum EQuestState QuestState);

	UFUNCTION(BlueprintCallable, Category = "QuestComponent")
		UQuest* FindQuest(FGameplayTag QuestTag) const;

	UFUNCTION(BlueprintCallable, Category = "QuestComponent")
		void UpdateQuest();

	UFUNCTION(BlueprintCallable, Category = "QuestComponent")
		void ResetAllQuest();

	UFUNCTION(BlueprintCallable, Category = "QuestComponent")
		void EvaluateNextNodeCondition(UQuest* QuestParam);

	UFUNCTION(BlueprintCallable, Category = "QuestComponent")
		void EvaluateQuestCondition(UQuest* QuestParam);

	UFUNCTION(BlueprintCallable, Category = "QuestComponent")
		void EvaluateQuestState(UQuest* QuestParam, int SumConditionMet);

	UFUNCTION(BlueprintCallable, Category = "QuestComponent")
		void GoToNextNode(UQuest* QuestParam, UQuestBuilderNode* NodeParam);

	UFUNCTION(BlueprintCallable, Category = "QuestComponent")
		void BeginNode(UQuest* QuestParam, bool bLaunchEventOnLoad = false);

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, BlueprintAuthorityOnly, Category = "Saving")
		void Save(const FString& SaveFileName, int SlotIndex);

	virtual void Save_Implementation(const FString& SaveFileName, int SlotIndex);
	
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, BlueprintAuthorityOnly, Category = "Saving")
		void Load(const FString& SaveFileName, int SlotIndex);

	virtual void Load_Implementation(const FString& SaveFileName, int SlotIndex);

	UFUNCTION(BlueprintCallable, Category = "Saving")
	virtual bool DeleteSave(const FString& SaveName = "QuestBuilderSaveData", const int32 Slot = 0);

public:
	UFUNCTION(BlueprintPure, Category = "QuestComponent")
	virtual APawn* GetOwningPawn() const;

	UFUNCTION(BlueprintPure, Category = "QuestComponent")
	virtual APlayerController* GetOwningController() const;


protected:
	// Called when the game starts
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

		
};
