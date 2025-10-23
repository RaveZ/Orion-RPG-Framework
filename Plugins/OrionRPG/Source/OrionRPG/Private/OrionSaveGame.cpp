// Copyright 2025 Ivan Chandra. All Rights Reserved.


#include "OrionSaveGame.h"
#include "Inventory/OrionInventoryFunctionLibrary.h"
#include "Inventory/OrionInventoryComponent.h"
#include "Engine/GameInstance.h"
#include "Engine/LocalPlayer.h"
#include "OrionSetting.h"
#include "QuestBuilderFunctionLibrary.h"
#include "QuestComponent.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/Character.h"
#include "Kismet/GameplayStatics.h"
#include "Serialization/MemoryWriter.h"
#include "Serialization/MemoryReader.h"
#include "Serialization/ObjectAndNameAsStringProxyArchive.h"
#include "WorldPartition/DataLayer/DataLayerManager.h"
#include "Engine/World.h"

UOrionSaveGame::UOrionSaveGame()
{
	bIsSaving = false;
}

void UOrionSaveGame::ResetToDefault()
{
	PlayInfoData = FOrionPlayInfoSaveData();
	PlayerData = FOrionPlayerSaveData();
	QuestData = FOrionQuestSaveData();
	InventorySaveData = FOrionInventorySaveData();
	SavedActors.Reset();

	Super::ResetToDefault();
}

int32 UOrionSaveGame::GetLatestDataVersion() const
{
	return 1;
}

void UOrionSaveGame::HandlePreSave()
{
	UOrionSetting* OrionSetting = GetMutableDefault<UOrionSetting>();
	bIsSaving = true;
	Super::HandlePreSave();
	UWorld* World = OwningPlayer->GetWorld();
	check(World);

	APlayerController* PC = World->GetFirstPlayerController<APlayerController>();
	check(PC);

	APawn* PlayerChar = PC->GetPawn<APawn>();
	check(PlayerChar);

	// Play info data
	{
		const FDateTime CurrentDateTime = FDateTime::Now();
		const FTimespan Duration = CurrentDateTime - PlayInfoData.CreatedDateTime;
		PlayInfoData.LastPlayDateTime = CurrentDateTime;
		PlayInfoData.TotalPlayTimeSeconds = Duration.GetTotalSeconds();

		FString MapName = World->GetMapName();
		MapName.RemoveFromStart(World->StreamingLevelsPrefix);

		FString MapPath;
		const FString PathName = World->GetPathName();
		{
			int32 TokenIndex = INDEX_NONE;

			if (PathName.FindLastChar(TEXT('/'), TokenIndex))
			{
				MapPath = PathName.LeftChop(PathName.Len() - TokenIndex);
			}
		}

		PlayInfoData.MapURL = FString::Printf(TEXT("%s/%s"), *MapPath, *MapName);


		if (OrionSetting->bSaveDataLayer)
		{
			if (UDataLayerManager* DataLayerManager = UDataLayerManager::GetDataLayerManager(World))
			{
				PlayInfoData.DataLayerNames = DataLayerManager->GetEffectiveActiveDataLayerNames();
			}
		}
	}


	// Player data
	{
		PlayerData.Location = PlayerChar->GetActorLocation();
		PlayerData.Rotation = PlayerChar->GetActorRotation();
	}

	//Quest Data
	{
		UQuestComponent* QuestComp = UQuestBuilderFunctionLibrary::GetQuestComponentFromTarget(PC);
		if (QuestComp)
		{
			QuestData = QuestComp->GetSaveGameData();
		}
	}

	//Inventory
	{
		UOrionInventoryComponent* InvComp = UOrionInventoryFunctionLibrary::GetInventoryComponentFromTarget(PC);
		if (InvComp)
		{
			InventorySaveData = InvComp->GetSaveGameData();
		}


	}
	CachedActors.Empty();

	TArray<AActor*> SaveGameActors;
	UGameplayStatics::GetAllActorsWithInterface(World, UOrionSaveGameInterface::StaticClass(), SaveGameActors);

	for (AActor* Actor : SaveGameActors)
	{
		CachedActors.AddUnique(Actor);
		FOrionActorSaveData ActorData;
		ActorData.ActorName = Actor->GetFName();

		FMemoryWriter MyMemoryWriter(ActorData.ByteData);

		FObjectAndNameAsStringProxyArchive Ar(MyMemoryWriter, true);

		Ar.ArIsSaveGame = true;
		Ar.ArNoDelta = true;
		Actor->Serialize(Ar);

		SavedActors.Add(ActorData);
	}


}

void UOrionSaveGame::HandlePostSave(bool bSuccess)
{
	bIsSaving = false;
	if (bSuccess)
	{
		for (auto& Actor : CachedActors)
		{
			if (Actor.IsValid())
			{
				IOrionSaveGameInterface::Execute_OnSaved(Actor.Get());
			}
		}
	}
}

void UOrionSaveGame::HandleLoadGame(UWorld* World, bool bFromLoadGame)
{

	check(World);
	UE_LOG(LogTemp, Log, TEXT("Handle load game (bFromLoadGame: %d)"), bFromLoadGame);

	APlayerController* PC = World->GetFirstPlayerController<APlayerController>();
	check(PC);

	UOrionSetting* OrionSetting = GetMutableDefault<UOrionSetting>();

	// Data that we need to restore if load game only, not on every level transition)
	if (bFromLoadGame)
	{
		UGameInstance* GI = World->GetGameInstance<UGameInstance>();
		check(GI);

		// PLAYER
		{
			APawn* PlayerChar = PC->GetPawn<APawn>();
			check(PlayerChar);

			FString MapName = World->GetMapName();
			MapName.RemoveFromStart(World->StreamingLevelsPrefix);

			FString MapPath;
			const FString PathName = World->GetPathName();
			int32 TokenIndex = INDEX_NONE;
			if (PathName.FindLastChar(TEXT('/'), TokenIndex))
			{
				MapPath = PathName.LeftChop(PathName.Len() - TokenIndex);
			}
			FString CurrentMapURL = FString::Printf(TEXT("%s/%s"), *MapPath, *MapName);

			// Compare with saved URL
			UE_LOG(LogTemp, Log, TEXT("CurrentMapURL: %s, PlayInfoData.MapURL: %s"), *CurrentMapURL, *PlayInfoData.MapURL);
			// Compare with saved URL
			bool bIsSameMap = CurrentMapURL.Equals(PlayInfoData.MapURL, ESearchCase::IgnoreCase);

			if (bIsSameMap)
			{
				FHitResult HitResult;
				PlayerChar->SetActorTransform(FTransform(PlayerData.Rotation, PlayerData.Location), false, &HitResult, ETeleportType::TeleportPhysics);
			}
			
		}

		

		// DATA LAYERS
		{
			if (OrionSetting->bSaveDataLayer)
			{
				UDataLayerManager* DataLayerManager = UDataLayerManager::GetDataLayerManager(World);

				for (const FName& DataLayerName : PlayInfoData.DataLayerNames)
				{
					const UDataLayerInstance* DataLayerInstance = DataLayerManager->GetDataLayerInstanceFromName(DataLayerName);
					check(DataLayerInstance);
					DataLayerManager->SetDataLayerInstanceRuntimeState(DataLayerInstance, EDataLayerRuntimeState::Activated);
				}
			}
		}

		//ACTOR DATA
		{
			TArray<AActor*> SaveGameActors;
			UGameplayStatics::GetAllActorsWithInterface(World, UOrionSaveGameInterface::StaticClass(), SaveGameActors);

			// Build TMap<FName, AActor*> for fast lookup
			TMap<FName, AActor*> ActorMap;
			for (AActor* Actor : SaveGameActors)
			{
				if (Actor)
				{
					ActorMap.Add(Actor->GetFName(), Actor);
				}
			}

            for (int32 i = 0; i < SavedActors.Num(); ++i)
            {
				const FOrionActorSaveData& ActorData = SavedActors[i];
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

		//INVENTORY
		{
			UOrionInventoryComponent* InvComp = UOrionInventoryFunctionLibrary::GetInventoryComponentFromTarget(PC);
			if (InvComp)
			{
				InvComp->InitializeFromSaveGame(InventorySaveData);
			}
		}

		// QUEST
		{
			UQuestComponent* QuestComp = UQuestBuilderFunctionLibrary::GetQuestComponentFromTarget(PC);
			if (QuestComp)
			{
				QuestComp->InitializeFromSaveGame(QuestData);
			}
		}

	}


	
}

void UOrionSaveGame::SaveActor(AActor* Actor)
{
	if (Actor && Actor->Implements<UOrionSaveGameInterface>())
	{
		CachedActors.AddUnique(Actor);
		FOrionActorSaveData ActorData;
		ActorData.ActorName = Actor->GetFName();

		FMemoryWriter MyMemoryWriter(ActorData.ByteData);

		FObjectAndNameAsStringProxyArchive Ar(MyMemoryWriter, true);

		Ar.ArIsSaveGame = true;
		Ar.ArNoDelta = true;
		Actor->Serialize(Ar);

		SavedActors.Add(ActorData);
	}
}
