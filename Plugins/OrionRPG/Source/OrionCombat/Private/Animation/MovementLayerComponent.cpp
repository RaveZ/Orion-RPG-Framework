// MovementLayerComponent.cpp
// Copyright 2025 Ivan Chandra. All Rights Reserved.

#include "Animation/MovementLayerComponent.h"
#include "GameFramework/Character.h"
#include "Components/SkeletalMeshComponent.h"

UMovementLayerComponent::UMovementLayerComponent()
{
    PrimaryComponentTick.bCanEverTick = true;
}

void UMovementLayerComponent::SetOverlay(FGameplayTag Tag)
{
    if (!Tag.IsValid())
    {
        UE_LOG(LogTemp, Warning, TEXT("MovementLayerComponent: Invalid tag provided to SetOverlay"));
        return;
    }

    FMovementLayer FoundLayer;
    bool bFound = false;
    
    for (const FMovementLayer& Layer : OverlayData)
    {
        if (Layer.Tag == Tag)
        {
            FoundLayer = Layer;
            bFound = true;
            break;
        }
    }

    if (!bFound)
    {
        UE_LOG(LogTemp, Warning, TEXT("MovementLayerComponent: No overlay found for tag '%s'"), *Tag.ToString());
        return;
    }

    if (!FoundLayer.AnimInstance)
    {
        UE_LOG(LogTemp, Warning, TEXT("MovementLayerComponent: AnimInstance is null for tag '%s'"), *Tag.ToString());
        return;
    }

    ACharacter* Character = Cast<ACharacter>(GetOwner());
    if (!Character)
    {
        UE_LOG(LogTemp, Warning, TEXT("MovementLayerComponent: Owner is not a character"));
        return;
    }

    USkeletalMeshComponent* SkeletalMesh = Character->GetMesh();
    if (!SkeletalMesh)
    {
        UE_LOG(LogTemp, Warning, TEXT("MovementLayerComponent: Skeletal mesh component not found"));
        return;
    }

    LinkOverlayLayer(FoundLayer.AnimInstance);

    CurrentOverlay = Tag;
    CurrentAnimInstanceClass = FoundLayer.AnimInstance;

    UE_LOG(LogTemp, Log, TEXT("MovementLayerComponent: Set overlay to '%s'"), *Tag.ToString());
}

void UMovementLayerComponent::ClearOverlay()
{
    UnlinkOverlayLayer();
    CurrentOverlay = FGameplayTag();
    CurrentAnimInstanceClass = nullptr;
    
    UE_LOG(LogTemp, Log, TEXT("MovementLayerComponent: Cleared overlay"));
}

FMovementLayer UMovementLayerComponent::GetOverlayData(FGameplayTag Tag) const
{
    for (const FMovementLayer& Layer : OverlayData)
    {
        if (Layer.Tag == Tag)
        {
            return Layer;
        }
    }
    
    return FMovementLayer();
}

void UMovementLayerComponent::LinkOverlayLayer(TSubclassOf<UAnimInstance> AnimInstanceClass)
{
    if (!AnimInstanceClass)
    {
        return;
    }

    ACharacter* Character = Cast<ACharacter>(GetOwner());
    if (!Character)
    {
        return;
    }

    USkeletalMeshComponent* SkeletalMesh = Character->GetMesh();
    if (!SkeletalMesh)
    {
        return;
    }

    SkeletalMesh->LinkAnimClassLayers(AnimInstanceClass);
}

void UMovementLayerComponent::UnlinkOverlayLayer()
{
    if (!CurrentAnimInstanceClass)
    {
        return;
    }

    ACharacter* Character = Cast<ACharacter>(GetOwner());
    if (!Character)
    {
        return;
    }

    USkeletalMeshComponent* SkeletalMesh = Character->GetMesh();
    if (!SkeletalMesh)
    {
        return;
    }

    SkeletalMesh->UnlinkAnimClassLayers(CurrentAnimInstanceClass);
}

void UMovementLayerComponent::BeginPlay()
{
    Super::BeginPlay();

}

void UMovementLayerComponent::TickComponent(float DeltaTime, ELevelTick TickType,
                                            FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
}