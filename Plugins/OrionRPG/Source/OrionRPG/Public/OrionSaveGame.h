// Copyright 2025 Ivan Chandra. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/SaveGame.h"
#include "Quest.h"
#include "QuestData.h"
#include "Inventory/InventoryData.h"
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



USTRUCT(BlueprintType)
struct FOrionPlayInfoSaveData
{
	GENERATED_BODY()

public:
	// Total playtime in seconds
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "PlayInfo")
	float TotalPlayTimeSeconds;

	// Last play date time
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "PlayInfo")
	FDateTime LastPlayDateTime;

	// Created date time
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "PlayInfo")
	FDateTime CreatedDateTime;

	// Map URL when saving the game
	UPROPERTY()
	FString MapURL;

	// Activated data layers when saving the game
	UPROPERTY()
	TSet<FName> DataLayerNames;


public:
	FOrionPlayInfoSaveData()
	{
		TotalPlayTimeSeconds = 0.0f;
		LastPlayDateTime = FDateTime::Now();
		CreatedDateTime = FDateTime::Now();
	}


	FORCEINLINE FString ToString() const
	{
		return FString::Printf(TEXT("(TotalPlayTimeSeconds: %.3f, LastPlayDateTime: %s, CreatedDateTime: %s)")
			, TotalPlayTimeSeconds
			, *LastPlayDateTime.ToString(TEXT("%d-%m-%Y %H:%M:%S"))
			, *CreatedDateTime.ToString(TEXT("%d-%m-%Y %H:%M:%S"))
		);
	}

};


USTRUCT()
struct FOrionPlayerSaveData
{
	GENERATED_BODY()

public:
	UPROPERTY()
	FVector Location;

	UPROPERTY()
	FRotator Rotation;

public:
	FOrionPlayerSaveData()
	{
		Location = FVector::ZeroVector;
		Rotation = FRotator::ZeroRotator;
	}

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

USTRUCT()
struct FOrionInventorySaveData
{
	GENERATED_BODY()

public:
	/**
	* Max weight capacity for the inventory
	* Current weight capacity will be calculated based on the items stored in the inventory
	*/
	UPROPERTY(VisibleAnywhere, Category = InventorySaveData)
	float MaxWeight;

	UPROPERTY(VisibleAnywhere, Category = InventorySaveData)
	FIntPoint GridDimension;

	UPROPERTY(VisibleAnywhere, Category = InventorySaveData)
	TArray<FOrionInventoryItemSaveData> SavedInventoryItems;

	UPROPERTY(VisibleAnywhere, Category = InventorySaveData)
	TArray<FOrionCurrencyData> SavedCurrencies;


};


UCLASS(Blueprintable, BlueprintType)
class ORIONRPG_API UOrionSaveGame : public ULocalPlayerSaveGame
{
	GENERATED_BODY()
public:

	UOrionSaveGame();
	virtual void ResetToDefault() override;
	virtual int32 GetLatestDataVersion() const override;
	virtual void HandlePreSave() override;
	virtual void HandlePostSave(bool bSuccess) override;

	virtual void HandleLoadGame(UWorld* World, bool bFromLoadGame);

public:
	UPROPERTY()
	TArray<FOrionActorSaveData> SavedActors;

	UPROPERTY()
	FOrionQuestSaveData QuestData;

	UPROPERTY()
	FOrionInventorySaveData InventorySaveData;

	UPROPERTY()
	FOrionPlayerSaveData PlayerData;

	UPROPERTY()
	FOrionPlayInfoSaveData PlayInfoData;

private:
	UPROPERTY(Transient)
	TArray<TWeakObjectPtr<AActor>> CachedActors;
	bool bIsSaving;

public:
	UFUNCTION(BlueprintCallable, Category = "Orion|SaveGame")
	void SaveActor(AActor* Actor);
public:
	FORCEINLINE bool IsSavingGame()
	{
		return bIsSaving;
	}

	UFUNCTION(BlueprintPure, Category = "Orion|SaveGame")
	FORCEINLINE FName GetSavedMapName()
	{
		return FName(*PlayInfoData.MapURL);
	}
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
	void OnPreSave();

	UFUNCTION(BlueprintNativeEvent)
	void OnSaved();

	UFUNCTION(BlueprintNativeEvent)
	void OnLoaded();

};