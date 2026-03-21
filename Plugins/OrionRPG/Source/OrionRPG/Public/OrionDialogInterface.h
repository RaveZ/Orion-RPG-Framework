#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "DialogData.h"
#include "OrionDialogInterface.generated.h"

UINTERFACE(BlueprintType)
class ORIONRPG_API UOrionDialogInterface : public UInterface
{
	GENERATED_BODY()
};

class ORIONRPG_API IOrionDialogInterface
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Orion|Dialog")
	void OnDialogLineStarted(const FDialogLineData& DialogLineData, const FGameplayTag& SpeakerParticipantTag, const FGameplayTag& ListenerParticipantTag);

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Orion|Dialog")
	void OnDialogLineEnded(const FDialogLineData& DialogLineData, const FGameplayTag& SpeakerParticipantTag, const FGameplayTag& ListenerParticipantTag);
};