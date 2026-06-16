// Copyright 2025 Ivan Chandra. All Rights Reserved.

#include "Task/OrionAsyncAction_SetDataLayerRuntimeState.h"

#include "Engine/World.h"
#include "TimerManager.h"
#include "WorldPartition/DataLayer/DataLayerManager.h"
#include "WorldPartition/WorldPartitionSubsystem.h"

UOrionAsyncAction_SetDataLayerRuntimeState* UOrionAsyncAction_SetDataLayerRuntimeState::SetDataLayerRuntimeStateAsync(UObject* WorldContextObject, const UDataLayerAsset* DataLayerAsset, EDataLayerRuntimeState InState, bool bInIsRecursive)
{
    UOrionAsyncAction_SetDataLayerRuntimeState* Node = NewObject<UOrionAsyncAction_SetDataLayerRuntimeState>();
    Node->WorldContextObjectPtr = WorldContextObject;
    Node->DataLayerAssetPtr = DataLayerAsset;
    Node->DesiredState = InState;
    Node->bRecursive = bInIsRecursive;
    return Node;
}

void UOrionAsyncAction_SetDataLayerRuntimeState::Activate()
{
    UObject* WC = const_cast<UObject*>(WorldContextObjectPtr.Get());
    if (!WC)
    {
        OnCompleted.Broadcast(false);
        return;
    }

    UWorld* World = WC->GetWorld();
    if (!World)
    {
        OnCompleted.Broadcast(false);
        return;
    }

    if (UDataLayerManager* DataLayerManager = UDataLayerManager::GetDataLayerManager(World))
    {
        if (DataLayerAssetPtr && DataLayerManager->GetDataLayerInstanceFromAsset(DataLayerAssetPtr))
        {
            DataLayerManager->SetDataLayerRuntimeState(DataLayerAssetPtr, DesiredState, bRecursive);
        }
        else
        {
            UE_LOG(LogTemp, Warning, TEXT("SetDataLayerRuntimeStateAsync: unknown DataLayer or not found"));
            OnCompleted.Broadcast(false);
            return;
        }
    }
    else
    {
        OnCompleted.Broadcast(false);
        return;
    }
    
    World->GetTimerManager().SetTimer(PollTimerHandle, this, &UOrionAsyncAction_SetDataLayerRuntimeState::PollStreaming, 0.2f, true);
}

void UOrionAsyncAction_SetDataLayerRuntimeState::PollStreaming()
{
    UObject* WC = const_cast<UObject*>(WorldContextObjectPtr.Get());
    if (!WC)
    {
        OnCompleted.Broadcast(false);
        return;
    }

    UWorld* World = WC->GetWorld();
    if (!World)
    {
        OnCompleted.Broadcast(false);
        return;
    }

    if (UWorldPartitionSubsystem* Subsystem = World->GetSubsystem<UWorldPartitionSubsystem>())
    {
        if (Subsystem->IsStreamingCompleted())
        {
            World->GetTimerManager().ClearTimer(PollTimerHandle);
            OnCompleted.Broadcast(true);
        }
        // else keep polling
    }
    else
    {
        World->GetTimerManager().ClearTimer(PollTimerHandle);
        OnCompleted.Broadcast(false);
    }
}