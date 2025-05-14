// Copyright 2024 Ivan Chandra. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "QuestData.h"
#include "UObject/NoExportTypes.h"
#include "OrionEvent.generated.h"

class UQuestBuilderGraph;
class UQuestBuilderNode;
class UQuest;


UENUM(BlueprintType)
enum class EEventRunType : uint8
{
	/** will trigger at the beggining of the node */
	E_Start			UMETA(DisplayName = "START"),
	/** will trigger if next node conditions met || at the end of state node */
	E_End			UMETA(DisplayName = "END"),
	/** will trigger twice at start and end of node */
	E_Both			UMETA(DisplayName = "BOTH"),
};

UCLASS(Abstract, Blueprintable, EditInlineNew, HideDropdown, AutoExpandCategories = ("Default"))
class QUEST_SYSTEM_RUNTIME_API UOrionEvent : public UObject
{
	GENERATED_BODY()
public:
	UOrionEvent();

	UPROPERTY(BlueprintReadOnly, Category = "QuestEvent")
		TObjectPtr<UQuestBuilderGraph> QuestGraph;
		

	UPROPERTY(BlueprintReadOnly, Category = "QuestEvent")
		TObjectPtr<UQuestBuilderNode> OwningNode;

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

	UPROPERTY(BlueprintReadWrite, Category = "QuestEvent")
		bool StartLaunched = false;

	UPROPERTY(BlueprintReadWrite, Category = "QuestEvent")
		bool EndLaunched = false;

	UPROPERTY(BlueprintReadOnly, Category = "QuestEvent")
		TObjectPtr<UQuest> Quest;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "QuestEvent")
		EEventRunType EventRunType = EEventRunType::E_Start;


	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "QuestEvent")
		void BeginEvent(AController* OwnerController, APawn* ControlledPawn);

	virtual void BeginEvent_Implementation(AController* OwnerController, APawn* ControlledPawn);

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "QuestEvent")
	FString GetNodeDisplayText() const;
	virtual FString GetNodeDisplayText_Implementation() const;

	UFUNCTION(BlueprintCallable, Category = "QuestEvent")
		AController* GetOwningController() const;

	UFUNCTION(BlueprintCallable, Category = "QuestEvent")
		APawn* GetOwningPawn() const;

	UFUNCTION(BlueprintCallable, Category = "QuestEvent")
		UQuestComponent* GetQuestComponent() const;

	UFUNCTION(BlueprintCallable, Category = "QuestEvent")
		UQuestBuilderGraph* GetOwningQuestGraph() const;

	UFUNCTION(BlueprintCallable, Category = "QuestEvent")
		UQuest* GetOwningQuest() const;

	UFUNCTION(CallInEditor, BlueprintCallable)
	virtual TArray<FName> GetAllQuestID();

	UFUNCTION(CallInEditor)
	virtual TArray<FName> GetAllNodeID();

	// Allows the Object to get a valid UWorld from it's outer.
	virtual UWorld* GetWorld() const override;

#if WITH_EDITOR
protected:
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif
};
