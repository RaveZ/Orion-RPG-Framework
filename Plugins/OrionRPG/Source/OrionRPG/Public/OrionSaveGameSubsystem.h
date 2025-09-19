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
	UPROPERTY(EditDefaultsOnly, Category = "SaveSubsystem")
	TSubclassOf<UOrionSaveGame> SaveGameClass;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "SaveSubsystem")
	TArray<TObjectPtr<UOrionSaveGame>> SaveGameSlots;

	UPROPERTY()
	int32 CurrentSaveSlot;

	UPROPERTY()
	TObjectPtr<UOrionSaveGame> CurrentSaveGame;


	UPROPERTY(BlueprintAssignable, BlueprintCallable, Category = "SaveSubsystem")
	FOrionSaveGameMulticastSignature OnSaved;

	UPROPERTY(BlueprintAssignable, BlueprintCallable, Category = "SaveSubsystem")
	FOrionSaveGameMulticastSignature OnLoaded;

private:
	FString TravelURL;
	bool bIsLoadingSaveGame;

public:
	UOrionSaveGameSubsystem();
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;

private:
	void InitializeLocalPlayerSaveGameSlots(ULocalPlayer* LocalPlayer);

	void HandlePreLoadMap(const FString& MapURL);
	void HandlePostLoadMap(UWorld* World);

	FString Platform_CreateSaveGameName(ULocalPlayer* LocalPlayer, int32 SaveSlotIndex) const;
	FString Platform_CreateSaveSnapshotName(ULocalPlayer* LocalPlayer, int32 SaveSlotIndex) const;

public:
	// Start new at save slot
	// @param Slot - Save slot index
	void StartNewGame(int32 Slot);

	// Save game at current save slot
	UFUNCTION(BlueprintCallable, Category = "SaveSubsystem")
	void SaveGame();

	// Load game from save slot
	// @param Slot - Save slot index
	UFUNCTION(BlueprintCallable, Category = "SaveSubsystem")
	void LoadGame(int32 Slot);

	// Get save game object from current save slot
	// @return NULL if not using save slot (e.g PIE)
	FORCEINLINE UOrionSaveGame* GetCurrentSaveGameObject() const
	{
		return (CurrentSaveSlot == -1) ? nullptr : SaveGameSlots[CurrentSaveSlot];
	}


	void SaveGame(FString SlotName, FOrionSaveGameSignature SaveCallback);

	void LoadGame(FString SlotName, FOrionSaveGameSignature LoadCallback);



// ================================================================================================ //
// CONSOLE COMMANDS
// ================================================================================================ //
public:

	UFUNCTION(Exec)
	void orionSaveGame();

	UFUNCTION(Exec)
	void orionLoadGame(int32 Slot);
};