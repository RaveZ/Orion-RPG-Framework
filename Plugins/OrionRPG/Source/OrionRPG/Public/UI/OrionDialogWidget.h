#pragma once

#include "CoreMinimal.h"
#include "UI/OrionActivatableWidget.h"
#include "GameplayTagContainer.h"
#include "OrionDialogWidget.generated.h"

UCLASS(BlueprintType, Blueprintable)
class ORIONRPG_API UOrionDialogWidget : public UOrionActivatableWidget
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "Dialog")
	void SetDialogData(
		const FText& InSpeakerName,
		const FText& InDialogLine,
		const FGameplayTag& InSpeakerParticipantTag,
		const FGameplayTag& InListenerParticipantTag);

	UPROPERTY(BlueprintReadOnly, Category = "Dialog")
	FText SpeakerName;

	UPROPERTY(BlueprintReadOnly, Category = "Dialog")
	FText DialogLine;

	UPROPERTY(BlueprintReadOnly, Category = "Dialog")
	FGameplayTag SpeakerParticipantTag;

	UPROPERTY(BlueprintReadOnly, Category = "Dialog")
	FGameplayTag ListenerParticipantTag;

	UFUNCTION(BlueprintImplementableEvent, Category = "Dialog")
	void BP_OnDialogDataUpdated();
};