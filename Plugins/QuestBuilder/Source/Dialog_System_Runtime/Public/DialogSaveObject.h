// Copyright 2025 Ivan Chandra. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/SaveGame.h"
#include "Dialog.h"
#include "DialogData.h"
#include "DialogSaveObject.generated.h"



UCLASS(Blueprintable)
class DIALOG_SYSTEM_RUNTIME_API UDialogSaveObject : public USaveGame
{
	GENERATED_BODY()
public:

	/** Map of Dialog ID and it's Dialog Data Struct */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = DialogSaveData)
		TMap<FName,FDialogData> DialogMapData;
};
