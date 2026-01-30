// Copyright 2025 Ivan Chandra. All Rights Reserved.


#include "Inventory/OrionEquipmentInstance.h"
#include "Components/SkeletalMeshComponent.h"
#include "GameFramework/Character.h"
#include "Engine/StreamableManager.h"
#include "Engine/AssetManager.h"
#include "Net/UnrealNetwork.h"

#if UE_WITH_IRIS
#include "Iris/ReplicationSystem/ReplicationFragmentUtil.h"
#endif // UE_WITH_IRIS

#include UE_INLINE_GENERATED_CPP_BY_NAME(OrionEquipmentInstance)

class FLifetimeProperty;
class UClass;
class USceneComponent;

UOrionEquipmentInstance::UOrionEquipmentInstance(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

UWorld* UOrionEquipmentInstance::GetWorld() const
{
	if (APawn* OwningPawn = GetPawn())
	{
		return OwningPawn->GetWorld();
	}
	else
	{
		return nullptr;
	}
}

void UOrionEquipmentInstance::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ThisClass, OwningItemInstance);
	DOREPLIFETIME(ThisClass, Instigator);
	DOREPLIFETIME(ThisClass, SpawnedActors);
	DOREPLIFETIME(ThisClass, EquipmentDef);
}

#if UE_WITH_IRIS
void UOrionEquipmentInstance::RegisterReplicationFragments(UE::Net::FFragmentRegistrationContext& Context, UE::Net::EFragmentRegistrationFlags RegistrationFlags)
{
	using namespace UE::Net;

	// Build descriptors and allocate PropertyReplicationFragments for this object
	FReplicationFragmentUtil::CreateAndRegisterFragmentsForObject(this, Context, RegistrationFlags);
}
#endif // UE_WITH_IRIS

APawn* UOrionEquipmentInstance::GetPawn() const
{
	return Cast<APawn>(GetOuter());
}

UOrionInventoryItemInstance* UOrionEquipmentInstance::GetOwningItemInstance() const
{
	return OwningItemInstance;
}

APawn* UOrionEquipmentInstance::GetTypedPawn(TSubclassOf<APawn> PawnType) const
{
	APawn* Result = nullptr;
	if (UClass* ActualPawnType = PawnType)
	{
		if (GetOuter()->IsA(ActualPawnType))
		{
			Result = Cast<APawn>(GetOuter());
		}
	}
	return Result;
}

void UOrionEquipmentInstance::SpawnEquipmentActors(const TArray<FOrionEquipmentActorData>& ActorsToSpawn, APawn* InPawn)
{
	USceneComponent* AttachTarget = InPawn->GetRootComponent();
	if (ACharacter* Char = Cast<ACharacter>(InPawn))
	{
		AttachTarget = Char->GetMesh();
	}

	for (const FOrionEquipmentActorData& SpawnInfo : ActorsToSpawn)
	{
		const FSoftObjectPath SoftPath = SpawnInfo.ActorToSpawn.ToSoftObjectPath();
		if (!SoftPath.IsValid())
		{
			continue;
		}

#if WITH_EDITOR || UE_BUILD_SHIPPING || UE_BUILD_TEST
		FOrionEquipmentActorData SpawnInfoCopy = SpawnInfo;
		FStreamableDelegate Delegate = FStreamableDelegate::CreateLambda([this, SoftPath, InPawn, AttachTarget, SpawnInfoCopy]()
			{
				UObject* LoadedObj = SoftPath.ResolveObject();
				if (!LoadedObj)
				{
					LoadedObj = SoftPath.TryLoad();
				}

				UClass* LoadedClass = Cast<UClass>(LoadedObj);
				if (!LoadedClass)
				{
					return;
				}

				if (!GetWorld() || !InPawn)
				{
					return;
				}

				AActor* NewActor = GetWorld()->SpawnActorDeferred<AActor>(LoadedClass, FTransform::Identity, InPawn);
				if (NewActor)
				{
					NewActor->FinishSpawning(FTransform::Identity, /*bIsDefaultTransform=*/ true);
					NewActor->SetActorRelativeTransform(SpawnInfoCopy.AttachTransform);
					NewActor->AttachToComponent(AttachTarget, FAttachmentTransformRules::KeepRelativeTransform, SpawnInfoCopy.AttachSocket);

					if (SpawnInfoCopy.bSetLeaderComponent)
					{
						USkeletalMeshComponent* LeaderSkelMeshComp = Cast<USkeletalMeshComponent>(AttachTarget);
						USkeletalMeshComponent* SkelMeshComp = NewActor->FindComponentByClass<USkeletalMeshComponent>();
						if (LeaderSkelMeshComp && SkelMeshComp)
						{
							SkelMeshComp->SetLeaderPoseComponent(Cast<USkeletalMeshComponent>(AttachTarget));
						}
					}
					
					OnEquipmentActorSpawned.Broadcast(NewActor);

					SpawnedActors.Add(NewActor);
				}
			});

		// Request async load via AssetManager's StreamableManager
		UAssetManager::GetStreamableManager().RequestAsyncLoad(SoftPath, Delegate);
#endif
	}
	
}

void UOrionEquipmentInstance::DestroyEquipmentActors()
{
	for (AActor* Actor : SpawnedActors)
	{
		if (Actor)
		{
			Actor->Destroy();
		}
	}
}

void UOrionEquipmentInstance::OnEquipped()
{
	K2_OnEquipped();
	OnEquippedDelegate.Broadcast();
}

void UOrionEquipmentInstance::OnUnequipped()
{
	K2_OnUnequipped();
	OnUnequippedDelegate.Broadcast();
}

void UOrionEquipmentInstance::SetEquipmentDef(FOrionEquipmentDefinition& InDef)
{
	EquipmentDef = InDef;	
}

void UOrionEquipmentInstance::OnRep_Instigator()
{
}




#undef LOCTEXT_NAMESPACE

