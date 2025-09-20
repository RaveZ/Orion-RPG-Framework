// Copyright 2025 Ivan Chandra. All Rights Reserved.


#include "OrionSaveGame.h"
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
		APawn* PlayerChar = PC->GetPawn<APawn>();
		check(PlayerChar);
		PlayerData.Location = PlayerChar->GetActorLocation();
		PlayerData.Rotation = PlayerChar->GetActorRotation();
	}

	//Quest Data
	{
		UQuestComponent* QuestComp = UQuestBuilderFunctionLibrary::GetQuestComponent(PC);
		check(QuestComp)
		QuestData = QuestComp->GetSaveGameData();
	}

	SavedActors.Empty();

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
		for (AActor* Actor : CachedActors)
		{
			IOrionSaveGameInterface::Execute_OnSaved(Actor);
		}
	}
}

void UOrionSaveGame::HandleLoadGame(UWorld* World, bool bFromLoadGame)
{
	check(World);
	UE_LOG(LogTemp, Log, TEXT("Handle post load level (bFromLoadGame: %d)"), bFromLoadGame);

	APlayerController* PC = World->GetFirstPlayerController<APlayerController>();
	check(PC);

	UOrionSetting* OrionSetting = GetMutableDefault<UOrionSetting>();

	// Data that we need to restore if load game only, not on every level transition)
	if (bFromLoadGame)
	{
		UGameInstance* GI = World->GetGameInstance<UGameInstance>();
		check(GI);

		// Player
		{
			APawn* PlayerChar = PC->GetPawn<APawn>();
			check(PlayerChar);

			if (PlayerData.Location != FVector::ZeroVector)
			{
				FHitResult HitResult;
				PlayerChar->SetActorTransform(FTransform(PlayerData.Rotation, PlayerData.Location), false, &HitResult, ETeleportType::TeleportPhysics);
			}
		}

		// Quest
		{
			UQuestComponent* QuestComp = UQuestBuilderFunctionLibrary::GetQuestComponent(PC);
			check(QuestComp)
			QuestComp->InitializeFromSaveGame(QuestData);
		}

		// Datalayers
		{
			if(OrionSetting->bSaveDataLayer)
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
	}


	
}