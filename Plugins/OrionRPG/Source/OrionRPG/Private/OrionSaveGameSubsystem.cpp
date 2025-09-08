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

void UOrionSaveGameSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
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
			LoadCallback.ExecuteIfBound(bSuccess);
			OnLoaded.IsBound() ? OnLoaded.Broadcast(bSuccess) : void();
		}
		
	}

	
}
