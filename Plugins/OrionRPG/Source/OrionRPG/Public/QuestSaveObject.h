// Copyright 2025 Ivan Chandra. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/SaveGame.h"
#include "Quest.h"
#include "QuestData.h"
#include "QuestSaveObject.generated.h"



UCLASS(Blueprintable)
class ORIONRPG_API UQuestSaveObject : public USaveGame
{
	GENERATED_BODY()
public:

	/** Array of saved QuestIDs*/
	UPROPERTY(VisibleAnywhere, Category = QuestSaveData)
	TArray<FName> QuestAssetIDs;

	/** Map of Quest ID and it's Quest Data Struct */
	UPROPERTY(VisibleAnywhere, Category = QuestSaveData)
		TMap<FName,FQuestData> QuestMapData;
};
