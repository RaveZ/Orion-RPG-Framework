// Copyright 2024 Ivan Chandra. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "QuestComponent.generated.h"

class UQuestSaveObject;
class USaveGame;
class AController;

//Quest Delegate
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FQuestObjectiveUpdated);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FStartNavigateQuest, UQuestSystem*, Quest);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FQuestFailed, UQuestSystem*, Quest);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FQuestCompleted, UQuestSystem*, Quest);
	
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FQuestAdded, UQuestSystem*, Quest);


//Save/Load Delegate
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnBeginSave, FString, SaveFileName);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnSaveComplete, FString, SaveFileName);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnBeginLoad, FString, SaveFileName);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnLoadComplete, FString, SaveFileName);

UCLASS(meta=(BlueprintSpawnableComponent), Blueprintable, HideDropdown, ClassGroup = "QuestBuilder", DisplayName = "Quest Component")
class QUEST_SYSTEM_RUNTIME_API UQuestComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UQuestComponent();

	UPROPERTY(BlueprintReadOnly, Category = "QuestComponent")
		TObjectPtr<AController> OwningController;

	/** Array of QuestGraph that is Running*/
	UPROPERTY(BlueprintReadWrite, Category = "QuestComponent")
		TArray< TObjectPtr<UQuestSystemGraph>> QuestGraphs;

	/** Map of Quest ID and attached Quest System That is Running*/
	UPROPERTY(BlueprintReadWrite, Category = "QuestComponent")
		TMap<FName, TObjectPtr<UQuestSystem>> QuestSystemMap;

	UPROPERTY(BlueprintReadOnly, Category = "QuestComponent")
		bool bQuestLoaded = false;

	UPROPERTY(BlueprintReadWrite, Category = "QuestComponent")
		TObjectPtr<UQuestSystem> CurrentNavigatedQuest;
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
		virtual void StartNavigatingQuest(UQuestSystem* Quest);

	UFUNCTION()
		virtual void QuestFailed(UQuestSystem* Quest);

	UFUNCTION()
		virtual void QuestCompleted(UQuestSystem* Quest);

	UFUNCTION()
		virtual void QuestAdded(UQuestSystem* Quest);

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
		bool BeginQuest(UQuestSystemGraph* QuestAsset);

	UFUNCTION(BlueprintCallable, Category = "QuestComponent")
		bool BeginMultipleQuest(TArray<UQuestSystemGraph*>  QuestAssets);

	UFUNCTION()
		void CacheQuestGraphDependencies(UQuestSystemGraph* QuestGraph);

	UFUNCTION()
		void EvaluateQuestSaveLoadState(UQuestSystemGraph* QuestGraph);

	UFUNCTION(BlueprintCallable, Category = "QuestComponent")
		void ActivateQuest(FName QuestID);

	UFUNCTION(BlueprintCallable, Category = "QuestComponent")
		void RestartQuest(FName QuestID, FName NodeID = NAME_None, bool bStartFromNodeID = false);

	UFUNCTION(BlueprintCallable, Category = "QuestComponent")
		void FailQuest(FName QuestID);

	UFUNCTION(BlueprintCallable, Category = "QuestComponent")
		void CompleteQuest(FName QuestID);


	UFUNCTION(BlueprintCallable, Category = "QuestComponent")
		void StartNavigateQuest(UQuestSystem* InQuest);
	
	UFUNCTION(BlueprintPure, Category = "QuestComponent")
		virtual APawn* GetOwningPawn() const;

	UFUNCTION(BlueprintCallable, Category = "QuestComponent")
		UQuestSystem* FindQuestFromNodeID(FName NodeIDParam) const;

	UFUNCTION(BlueprintCallable, Category = "QuestComponent")
		bool IsQuestActiveFromNodeID(FName NodeID) const;

	UFUNCTION(BlueprintCallable, Category = "QuestComponent")
		bool IsQuestActiveFromQuestID(FName QuestID) const;

	UFUNCTION(BlueprintCallable, Category = "QuestComponent")
		void UpdateQuest();

	UFUNCTION(BlueprintCallable, Category = "QuestComponent")
		void EvaluateNextNodeCondition(UQuestSystem* QuestParam);

	UFUNCTION(BlueprintCallable, Category = "QuestComponent")
		void EvaluateQuestCondition(UQuestSystem* QuestParam);

	UFUNCTION(BlueprintCallable, Category = "QuestComponent")
		void EvaluateQuestState(UQuestSystem* QuestParam, int SumConditionMet);

	UFUNCTION(BlueprintCallable, Category = "QuestComponent")
		void GoToNextNode(UQuestSystem* QuestParam, UQuestSystemGraphNode* NodeParam);

	UFUNCTION(BlueprintCallable, Category = "QuestComponent")
		void BeginNode(UQuestSystem* QuestParam);

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Saving")
		void Save(const FString& SaveFileName, int SlotIndex);

	virtual void Save_Implementation(const FString& SaveFileName, int SlotIndex);
	
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Saving")
		void Load(const FString& SaveFileName, int SlotIndex);

	virtual void Load_Implementation(const FString& SaveFileName, int SlotIndex);

	UFUNCTION(BlueprintCallable, Category = "Saving")
	virtual bool DeleteSave(const FString& SaveName = "QuestBuilderSaveData", const int32 Slot = 0);

	UFUNCTION()
	bool ShouldUpdateQuest(UQuestSystem* QuestParam);
protected:
	// Called when the game starts
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

		
};
