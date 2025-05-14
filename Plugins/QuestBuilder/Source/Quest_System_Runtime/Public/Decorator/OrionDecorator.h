// Copyright 2025 Ivan Chandra. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "OrionDecorator.generated.h"

class UQuestBuilderGraph;
class UQuestBuilderNode;

UCLASS(Abstract, Blueprintable, EditInlineNew, HideDropdown)
class QUEST_SYSTEM_RUNTIME_API UOrionDecorator : public UObject
{
	GENERATED_BODY()
public:
	UPROPERTY(BlueprintReadOnly, Category = "QuestCondition")
		TObjectPtr<UQuestBuilderGraph> QuestGraph;

	UPROPERTY(BlueprintReadOnly, Category = "QuestCondition")
		TObjectPtr<UQuestBuilderNode> OwningNode;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QuestEvent")
		bool InvertCondition = false;

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
		TObjectPtr<UQuest> Quest;

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "QuestCondition")
		bool PerformConditionCheck(AController* OwnerController, APawn* ControlledPawn) const;

		virtual bool PerformConditionCheck_Implementation(AController* OwnerController, APawn* ControlledPawn) const;

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
		UQuestBuilderGraph* GetOwningQuestGraph() const;

	UFUNCTION(BlueprintCallable, Category = "QuestCondition")
		UQuest* GetOwningQuest() const;

	UFUNCTION(CallInEditor)
	virtual	TArray<FName> GetAllQuestID();

	UFUNCTION(CallInEditor)
	virtual TArray<FName> GetAllNodeID();

	// Allows the Object to get a valid UWorld from it's outer.
	virtual UWorld* GetWorld() const override;
#if WITH_EDITOR
protected:
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif
};
