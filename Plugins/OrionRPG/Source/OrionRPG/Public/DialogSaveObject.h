// Copyright 2025 Ivan Chandra. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/SaveGame.h"
#include "Dialog.h"
#include "DialogData.h"
#include "DialogSaveObject.generated.h"



UCLASS(Blueprintable)
class ORIONRPG_API UDialogSaveObject : public USaveGame
{
	GENERATED_BODY()
public:

	/** Array of saved DialogGraphIDs*/
	UPROPERTY(VisibleAnywhere, Category = DialogSaveData)
	TArray<FName> DialogAssetIDs;

	/** Map of Dialog ID and it's Dialog Data Struct */
	UPROPERTY(VisibleAnywhere, Category = DialogSaveData)
		TMap<FName,FDialogData> DialogMapData;
};
