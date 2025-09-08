// Copyright 2025 Ivan Chandra. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "OrionSaveGameSubsystem.generated.h"

class UOrionSaveGame;

DECLARE_DYNAMIC_DELEGATE_OneParam(FOrionSaveGameSignature, const bool, bSuccess);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOrionSaveGameMulticastSignature, const bool, bSuccess);

UCLASS()
class ORIONRPG_API UOrionSaveGameSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:

	UPROPERTY()
	TObjectPtr<UOrionSaveGame> CurrentSaveGame;


	UPROPERTY(BlueprintAssignable, BlueprintCallable, Category = "SaveSubsystem")
	FOrionSaveGameMulticastSignature OnSaved;

	UPROPERTY(BlueprintAssignable, BlueprintCallable, Category = "SaveSubsystem")
	FOrionSaveGameMulticastSignature OnLoaded;


	UFUNCTION(BlueprintCallable, Category = "SaveSubsystem")
	void SaveGame(FString SlotName, FOrionSaveGameSignature SaveCallback);

	UFUNCTION(BlueprintCallable, Category = "SaveSubsystem")
	void LoadGame(FString SlotName, FOrionSaveGameSignature LoadCallback);


	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
};