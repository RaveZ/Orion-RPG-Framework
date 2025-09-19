// Copyright 2025 Ivan Chandra. All Rights Reserved.


#include "OrionSaveGameSubsystem.h"
#include "OrionSaveGame.h"
#include "Engine/World.h"
#include "QuestComponent.h"
#include "QuestBuilderFunctionLibrary.h"
#include "Kismet/GameplayStatics.h"
#include "Serialization/MemoryWriter.h"
#include "Serialization/MemoryReader.h"
#include "Serialization/ObjectAndNameAsStringProxyArchive.h"

UOrionSaveGameSubsystem::UOrionSaveGameSubsystem()
{
	bIsLoadingSaveGame = false;
	SaveGameClass = UOrionSaveGame::StaticClass();
	SaveGameSlots.SetNum(8);
	CurrentSaveSlot = 0;
}

void UOrionSaveGameSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	UGameInstance* GI = GetGameInstance();
	//Delegates
	{
		FCoreUObjectDelegates::PreLoadMap.AddUObject(this, &UOrionSaveGameSubsystem::HandlePreLoadMap);
		FCoreUObjectDelegates::PostLoadMapWithWorld.AddUObject(this, &UOrionSaveGameSubsystem::HandlePostLoadMap);
		GI->OnLocalPlayerAddedEvent.AddUObject(this, &UOrionSaveGameSubsystem::InitializeLocalPlayerSaveGameSlots);
	}
}


void UOrionSaveGameSubsystem::InitializeLocalPlayerSaveGameSlots(ULocalPlayer* LocalPlayer)
{
	check(LocalPlayer);

	for (int32 i = 0; i < SaveGameSlots.Num(); ++i)
	{
		const FString SaveName = Platform_CreateSaveGameName(LocalPlayer, i);
		UE_LOG(LogTemp, Log, TEXT("OrionSaveSubsystem: Initialize save game slot (Name: %s, Slot: %i)"), *SaveName, i);
		SaveGameSlots[i] = Cast<UOrionSaveGame>(ULocalPlayerSaveGame::LoadOrCreateSaveGameForLocalPlayer(SaveGameClass, LocalPlayer, SaveName));
	}
}

void UOrionSaveGameSubsystem::HandlePreLoadMap(const FString& MapURL)
{
	UE_LOG(LogTemp, Log, TEXT("OrionSaveSubsystem: HandlePreLoadMap (MapURL: %s)"), *MapURL);
}

void UOrionSaveGameSubsystem::HandlePostLoadMap(UWorld* World)
{
	FString CurrentMapName = World->GetMapName();
	CurrentMapName.RemoveFromStart(World->StreamingLevelsPrefix);
	UE_LOG(LogTemp, Log, TEXT("OrionSaveSubsystem: HandlePostLoadMap [%s]"), *CurrentMapName);

	APlayerController* PC = World->GetFirstPlayerController();
	

	{
		if (UOrionSaveGame* SaveGame = GetCurrentSaveGameObject())
		{
			SaveGame->HandlePostLoadLevel(World, bIsLoadingSaveGame);
		}

		bIsLoadingSaveGame = false;
	}
}

FString UOrionSaveGameSubsystem::Platform_CreateSaveGameName(ULocalPlayer* LocalPlayer, int32 SaveSlotIndex) const
{
	const FString SaveGameName = FString::Printf(TEXT("save_%i"), SaveSlotIndex);

#if (UE_BUILD_SHIPPING && PLATFORM_WINDOWS)

	check(LocalPlayer);
	const FUniqueNetIdRepl UniqueNetId = LocalPlayer->GetUniqueNetIdForPlatformUser();
	check(UniqueNetId.IsValid());
	return FString::Printf(TEXT("%s/%s"), *UniqueNetId->ToString(), *SaveGameName);

#else
	return SaveGameName;

#endif // UE_BUILD_SHIPPING
}

FString UOrionSaveGameSubsystem::Platform_CreateSaveSnapshotName(ULocalPlayer* LocalPlayer, int32 SaveSlotIndex) const
{
	const FString SaveSnapshotName = FString::Printf(TEXT("save_snapshot_%i"), SaveSlotIndex);

#if (UE_BUILD_SHIPPING && PLATFORM_WINDOWS)

	check(LocalPlayer);
	const FUniqueNetIdRepl UniqueNetId = LocalPlayer->GetUniqueNetIdForPlatformUser();
	check(UniqueNetId.IsValid());
	return FString::Printf(TEXT("%s/%s"), *UniqueNetId->ToString(), *SaveSnapshotName);

#else
	return SaveSnapshotName;

#endif // UE_BUILD_SHIPPING
}

void UOrionSaveGameSubsystem::StartNewGame(int32 Slot)
{
}

void UOrionSaveGameSubsystem::SaveGame()
{
	UOrionSaveGame* SaveGameObject = SaveGameSlots[CurrentSaveSlot];
	check(SaveGameObject);

	UE_LOG(LogTemp, Log, TEXT("OrionSaveSubsystem: Save game (SaveSlot: %i, SaveName: %s)"), CurrentSaveSlot, *SaveGameObject->GetSaveSlotName());
	SaveGameObject->AsyncSaveGameToSlotForLocalPlayer();
}

void UOrionSaveGameSubsystem::LoadGame(int32 Slot)
{
	if (Slot < 0 || Slot >= SaveGameSlots.Num())
	{
		UE_LOG(LogTemp, Error, TEXT("OrionSaveSubsystem: Fail to load game from slot (%i). Invalid save slot!"), Slot);
		return;
	}

	UGameInstance* GI = GetGameInstance();
	check(GI);
	ULocalPlayer* LocalPlayer = GI->GetLocalPlayerByIndex(0);
	check(LocalPlayer);

	CurrentSaveSlot = Slot;

	UOrionSaveGame* SaveGameObject = SaveGameSlots[CurrentSaveSlot];
	check(SaveGameObject);

	const FOrionPlayInfoSaveData& PlayInfo = SaveGameObject->PlayInfoData;

	TravelURL = FString::Printf(TEXT("%s"), *PlayInfo.MapURL);

	UE_LOG(LogTemp, Log, TEXT("OrionSaveSubsystem: Load game (SaveSlot: %i, SaveName: %s, TravelURL: %s)"), CurrentSaveSlot, *SaveGameObject->GetSaveSlotName(), *TravelURL);

	bIsLoadingSaveGame = true;
	GetWorld()->ServerTravel(TravelURL, true);
}

void UOrionSaveGameSubsystem::SaveGame(FString SlotName, FOrionSaveGameSignature SaveCallback)
{
	if (UGameplayStatics::DoesSaveGameExist(SlotName, 0))
	{
		CurrentSaveGame = Cast<UOrionSaveGame>(UGameplayStatics::LoadGameFromSlot(SlotName, 0));
	}
	else
	{
		CurrentSaveGame = CastChecked<UOrionSaveGame>(UGameplayStatics::CreateSaveGameObject(UOrionSaveGame::StaticClass()));
	}
	//QUEST
	UWorld* World = GetWorld();
	if (World)
	{
		UQuestComponent* QuestComp = UQuestBuilderFunctionLibrary::GetQuestComponent(GetWorld());
		if(QuestComp)
		{
			CurrentSaveGame->QuestData = QuestComp->GetSaveGameData();
		}
	}
	

	//ACTOR DATA
	bool bSuccess = false;
	CurrentSaveGame->SavedActors.Empty();

	TArray<AActor*> SaveGameActors;
	UGameplayStatics::GetAllActorsWithInterface(GetWorld(), UOrionSaveGameInterface::StaticClass(), SaveGameActors);

	for (AActor* Actor : SaveGameActors)
	{
		FOrionActorSaveData ActorData;
		ActorData.ActorName = Actor->GetFName();

		FMemoryWriter MyMemoryWriter(ActorData.ByteData);

		FObjectAndNameAsStringProxyArchive Ar(MyMemoryWriter, true);

		Ar.ArIsSaveGame = true;
		Ar.ArNoDelta = true;
		Actor->Serialize(Ar);

		CurrentSaveGame->SavedActors.Add(ActorData);
	}

	bSuccess = UGameplayStatics::SaveGameToSlot(CurrentSaveGame, SlotName, 0);
	SaveCallback.ExecuteIfBound(bSuccess);
	OnSaved.IsBound() ? OnSaved.Broadcast(bSuccess) : void();

	if (bSuccess)
	{
		for (AActor* Actor : SaveGameActors)
		{
			IOrionSaveGameInterface::Execute_OnSaved(Actor);
		}
	}
}

void UOrionSaveGameSubsystem::LoadGame(FString SlotName, FOrionSaveGameSignature LoadCallback)
{
	bool bSuccess = false;
	if (UGameplayStatics::DoesSaveGameExist(SlotName, 0))
	{
		CurrentSaveGame = Cast<UOrionSaveGame>(UGameplayStatics::LoadGameFromSlot(SlotName, 0));
		if (CurrentSaveGame)
		{
			bSuccess = true;
			//QUEST
			UWorld* World = GetWorld();
			if (World)
			{
				UQuestComponent* QuestComp = UQuestBuilderFunctionLibrary::GetQuestComponent(GetWorld());
				if (QuestComp)
				{
					QuestComp->InitializeFromSaveGame(CurrentSaveGame->QuestData);
				}
			}


			//ACTOR DATA
			TArray<AActor*> SaveGameActors;
			UGameplayStatics::GetAllActorsWithInterface(GetWorld(), UOrionSaveGameInterface::StaticClass(), SaveGameActors);

			// Build TMap<FName, AActor*> for fast lookup
			TMap<FName, AActor*> ActorMap;
			for (AActor* Actor : SaveGameActors)
			{
				if (Actor)
				{
					ActorMap.Add(Actor->GetFName(), Actor);
				}
			}

			// Restore saved data using the map
			for (FOrionActorSaveData ActorData : CurrentSaveGame->SavedActors)
			{
				AActor* FoundActor = ActorMap.FindRef(ActorData.ActorName);
				if (FoundActor)
				{
					FMemoryReader MyMemoryReader(ActorData.ByteData);

					FObjectAndNameAsStringProxyArchive Ar(MyMemoryReader, true);
					Ar.ArNoDelta = true;
					Ar.ArIsSaveGame = true;

					FoundActor->Serialize(Ar);

					IOrionSaveGameInterface::Execute_OnLoaded(FoundActor);
				}
			}
		}
		
	}
	LoadCallback.ExecuteIfBound(bSuccess);
	OnLoaded.IsBound() ? OnLoaded.Broadcast(bSuccess) : void();

	
}

void UOrionSaveGameSubsystem::orionSaveGame()
{
	SaveGame();
}

void UOrionSaveGameSubsystem::orionLoadGame(int32 Slot)
{
	LoadGame(Slot);
}
