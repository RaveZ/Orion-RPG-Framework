// Copyright 2025 Ivan Chandra. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "GameplayTagContainer.h"
#include "OrionDecorator.generated.h"

class UQuestBuilderGraph;
class UQuestBuilderNode;

UCLASS(Abstract, Blueprintable, EditInlineNew, HideDropdown)
class ORIONRPG_API UOrionDecorator : public UObject
{
	GENERATED_BODY()
public:
	/** node name override*/
	UPROPERTY(Category = Description, EditAnywhere)
	FString NodeName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QuestEvent")
		bool InvertCondition = false;

	/**Display quest Tag in property panel?*/
	UPROPERTY(BlueprintReadWrite, Category = "QuestEvent")
		bool bUseQuestTag = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QuestEvent", meta = (EditCondition = "bUseQuestTag == true", HideEditConditionToggle, EditConditionHides, Categories = "Quest"))
		FGameplayTag QuestTag;

	/** Display Node ID in property panel?*/
	UPROPERTY(BlueprintReadWrite, Category = "QuestEvent")
		bool bUseNodeTag = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QuestEvent", meta = (EditCondition = "bUseNodeTag == true", HideEditConditionToggle, EditConditionHides, Categories = "Quest"))
		FGameplayTag NodeTag;

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "OrionDecorator")
		bool PerformConditionCheck(AController* OwnerController, APawn* ControlledPawn) const;

		virtual bool PerformConditionCheck_Implementation(AController* OwnerController, APawn* ControlledPawn) const;

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "OrionDecorator")
	FString GetNodeDisplayText() const;

	virtual FString GetNodeDisplayText_Implementation() const;

	UFUNCTION(BlueprintCallable, Category = "OrionDecorator")
		UQuestComponent* GetQuestComponent() const;

	UFUNCTION(BlueprintCallable, Category = "OrionDecorator")
		FString GetShortTag(FGameplayTag Tag, int32 Level = 1) const;

	/** @return name of node */
	FString GetNodeName();
	FString GetShortTypeName(UObject* Ob);

	// Allows the Object to get a valid UWorld from it's outer.
	virtual UWorld* GetWorld() const override;
#if WITH_EDITOR
protected:
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif
};
