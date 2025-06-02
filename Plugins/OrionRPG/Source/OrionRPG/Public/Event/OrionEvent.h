// Copyright 2025 Ivan Chandra. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "QuestData.h"
#include "UObject/NoExportTypes.h"
#include "GameplayTagContainer.h"
#include "OrionEvent.generated.h"

class UQuestBuilderGraph;
class UQuestBuilderNode;
class UQuest;


UENUM(BlueprintType)
enum class EEventLaunchType : uint8
{
	/** will trigger at the beggining of the node */
	E_Start			UMETA(DisplayName = "Start of node"),
	/** will trigger if next node conditions met || at the end of state node */
	E_End			UMETA(DisplayName = "End of node"),
	/** will trigger twice at start and end of node */
	E_Both			UMETA(DisplayName = "Both"),
};

DECLARE_MULTICAST_DELEGATE(FEventFinishedSignature);

UCLASS(Abstract, Blueprintable, EditInlineNew, HideDropdown, AutoExpandCategories = ("Default"))
class ORIONRPG_API UOrionEvent : public UObject
{
	GENERATED_BODY()
public:
	UOrionEvent();

	/** node name override*/
	UPROPERTY(Category = Description, EditAnywhere)
	FString NodeName;

	/**Display quest Tag in property panel?*/
	UPROPERTY(BlueprintReadWrite, Category = "Event")
		bool bUseQuestTag = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Event", meta = (EditCondition = "bUseQuestTag == true", HideEditConditionToggle, EditConditionHides))
		FGameplayTag QuestTag;

	/** Display Node ID in property panel?*/
	UPROPERTY(BlueprintReadWrite, Category = "Event")
		bool bUseNodeTag = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Event", meta = (EditCondition = "bUseNodeTag == true", HideEditConditionToggle, EditConditionHides))
		FGameplayTag NodeTag;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Event")
		EEventLaunchType EventLaunchType = EEventLaunchType::E_Start;

	/**Launch this event for the first time quest loaded*/
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Event", DisplayName = "Launch Event On Load")
		bool bLaunchEventOnLoad = false;
		
	/**Called when event finished*/
	FEventFinishedSignature OnEventFinished;

	UFUNCTION(BlueprintImplementableEvent, DisplayName = "Begin Event", Category = "Event")
		void K2_BeginEvent(AController* OwnerController, APawn* ControlledPawn);

	UFUNCTION(BlueprintCallable, Category = "Event")
	virtual void BeginEvent(AController* OwnerController, APawn* ControlledPawn);


	UFUNCTION(BlueprintImplementableEvent, DisplayName = "Event Finished", Category = "Event")
	void K2_EventFinished();

	UFUNCTION(BlueprintCallable, Category = "Event")
	virtual void EndEvent();

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Event")
	FString GetNodeDisplayText() const;
	virtual FString GetNodeDisplayText_Implementation() const;

	UFUNCTION(BlueprintCallable, Category = "Event")
		UQuestComponent* GetQuestComponent() const;


	/** @return name of node */
	FString GetNodeName();
	FString GetShortTypeName(UObject* Ob);
	FString GetShortTag(FGameplayTag Tag) const;

	// Allows the Object to get a valid UWorld from it's outer.
	virtual UWorld* GetWorld() const override;

#if WITH_EDITOR
protected:
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif
};
