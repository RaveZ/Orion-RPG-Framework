// Copyright 2025 Ivan Chandra. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/SaveGame.h"
#include "Quest.h"
#include "QuestData.h"
#include "UObject/Interface.h"
#include "OrionSaveGame.generated.h"

USTRUCT()
struct FOrionActorSaveData
{
	GENERATED_BODY()

public:
	UPROPERTY()
	FName ActorName;

	UPROPERTY()
	FTransform ActorTransform;

	UPROPERTY()
	TArray<uint8> ByteData;
};

USTRUCT()
struct FOrionQuestSaveData
{
	GENERATED_BODY()

public:
	/** Array of saved QuestIDs*/
	UPROPERTY(VisibleAnywhere, Category = QuestSaveData)
	TArray<FName> QuestAssetIDs;

	UPROPERTY(VisibleAnywhere, Category = QuestSaveData)
	FName NavigatedQuestID;

	UPROPERTY(VisibleAnywhere, Category = QuestSaveData)
	FName NavigatedObjectiveID;

	/** Map of Quest ID and it's Quest Data Struct */
	UPROPERTY(VisibleAnywhere, Category = QuestSaveData)
	TMap<FName, FQuestData> QuestMapData;

};


UCLASS(Blueprintable)
class ORIONRPG_API UOrionSaveGame : public USaveGame
{
	GENERATED_BODY()
public:
	UPROPERTY()
	TArray<FOrionActorSaveData> SavedActors;


	UPROPERTY()
	FOrionQuestSaveData QuestData;
};


UINTERFACE()
class UOrionSaveGameInterface : public UInterface
{
	GENERATED_BODY()
};

class ORIONRPG_API IOrionSaveGameInterface
{
	GENERATED_BODY()

public:

	UFUNCTION(BlueprintNativeEvent)
	void OnSaved();

	UFUNCTION(BlueprintNativeEvent)
	void OnLoaded();

};