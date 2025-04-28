// Copyright 2024 Ivan Chandra. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "QuestCondition.generated.h"

class UQuestSystemGraph;
class UQuestSystemGraphNode;

UCLASS(Abstract, Blueprintable, EditInlineNew, HideDropdown)
class QUEST_SYSTEM_RUNTIME_API UQuestCondition : public UObject
{
	GENERATED_BODY()
public:
	UPROPERTY(BlueprintReadOnly, Category = "QuestCondition")
		TObjectPtr<UQuestSystemGraph> QuestGraph;

	UPROPERTY(BlueprintReadOnly, Category = "QuestCondition")
		TObjectPtr<UQuestSystemGraphNode> OwningNode;

	/**Display quest ID in property panel?*/
	UPROPERTY(BlueprintReadWrite, Category = "QuestEvent")
		bool bUseQuestID = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QuestEvent", meta = (EditCondition = "bUseQuestID == true", HideEditConditionToggle, EditConditionHides, GetOptions = "GetAllQuestID"))
		FName QuestID;

	/** Display Node ID in property panel?*/
	UPROPERTY(BlueprintReadWrite, Category = "QuestEvent")
		bool bUseNodeID = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QuestEvent", meta = (EditCondition = "bUseNodeID == true", HideEditConditionToggle, EditConditionHides, GetOptions = "GetAllNodeID"))
		FName NodeID;
	
	UPROPERTY(BlueprintReadOnly, Category = "QuestCondition")
		TObjectPtr<UQuestSystem> QuestSystem;

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "QuestCondition")
		bool IsConditionMet() const;

		virtual bool IsConditionMet_Implementation() const;

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "QuestCondition")
	FString GetNodeDisplayText() const;

	virtual FString GetNodeDisplayText_Implementation() const;

	UFUNCTION(BlueprintCallable, Category = "QuestCondition")
		AController* GetOwningController() const;

	UFUNCTION(BlueprintCallable, Category = "QuestCondition")
		APawn* GetOwningPawn() const;

	UFUNCTION(BlueprintCallable, Category = "QuestCondition")
		UQuestComponent* GetQuestComponent() const;

	UFUNCTION(BlueprintCallable, Category = "QuestCondition")
		UQuestSystemGraph* GetOwningQuestGraph() const;

	UFUNCTION(BlueprintCallable, Category = "QuestCondition")
		UQuestSystem* GetOwningQuestSystem() const;

	UFUNCTION(CallInEditor)
	virtual	TArray<FName> GetAllQuestID();

	UFUNCTION(CallInEditor)
	virtual TArray<FName> GetAllNodeID();

#if WITH_EDITOR
protected:
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif
};
