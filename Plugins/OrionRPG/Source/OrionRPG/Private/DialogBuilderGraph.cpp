// Copyright 2025 Ivan Chandra. All Rights Reserved.


#include "DialogBuilderGraph.h"
#include "DialogComponent.h"
#include "DialogBuilderNode.h"
#include "DialogBuilderEdge.h"
#include "DialogCameraShot.h"
#include "DialogSequence.h"
#include "DialogStage.h"
#include "DialogBuilderNode_Root.h"
#include "DialogBuilderSetting.h"
#include "LevelSequencePlayer.h"
#include "LevelSequenceActor.h"
#include "DialogBuilderNode_RerouteNode.h"
#include "DialogBuilderNode_DialogLine.h"
#include "DialogBuilderNode_DialogSequence.h"
#include "EngineUtils.h"
#include "GameFramework/Character.h"
#include <CineCameraActor.h>
#include <DefaultLevelSequenceInstanceData.h>
#include <CineCameraComponent.h>
#include "Sound/SoundBase.h"
#include "Components/AudioComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Components/SkeletalMeshComponent.h"
#include "Camera/PlayerCameraManager.h"
#include "Animation/AnimMontage.h"
#include "TimerManager.h"
#include "Animation/AnimInstance.h"
#include "Camera/CameraShakeBase.h"

#define LOCTEXT_NAMESPACE "DialogBuilderGraph"


UDialogBuilderGraph::UDialogBuilderGraph()
{
	bAutoRotateParticipant = false;
	bRestoreState = false;
	DefaultBoneToTrack = FName("head");
	bAutoAdvanceDialogLine = false;
	bLockPlayerMovement = true;
	bCanInterruptDialog = true;
	bFadeCameraOnDialogBegin = true;

	BlendTime = 1.25f;
	BlendFunc = VTBlend_Cubic;
	BlendExp = 2.0f;
}

UDialogBuilderGraph::~UDialogBuilderGraph()
{
	
}

bool UDialogBuilderGraph::Initialize(UDialogComponent* InitializingComp)
{
	if (!HasAnyFlags(RF_ClassDefaultObject))
	{
		//We need a valid Quest component to make a quest for 
		if (!InitializingComp)
		{
			return false;
		}
		OwningController = InitializingComp->GetOwningController();
		DialogComponent = InitializingComp;
		VisitedNodeIDs.Empty();
		CachedActorMap.Empty();
		CachedActorOriginalTransforms.Empty();
		NodeMap.Empty();
		for (auto& Node : AllNodes)
		{
			if (Node)
			{
				Node->DialogGraph = this;
				Node->OwningController = InitializingComp->GetOwningController();
				Node->DialogComponent = InitializingComp;
				for (auto& Event : Node->Events)
				{
					if (Event)
					{
						Event->BeginSetup(DialogComponent->GetOwningController(), GetOwningPawn());
					}
				}
				NodeMap.Emplace(Node->ID, Node);
			}
		}

		InitializeDialogActors();
		

		if (OwningController && OwningController->IsLocalPlayerController())
		{
			if (DialogCameraShake)
			{
				OwningController->ClientStartCameraShake(DialogCameraShake);
			}
		}
		bAutoAdvanceDialogLine = bLockPlayerMovement ? bAutoAdvanceDialogLine : true; // if dialog is free movement, force auto advance dialog line -> true
		bFadeCameraOnDialogBegin = bLockPlayerMovement ? bFadeCameraOnDialogBegin : false;
		
	}
	return false;
}

void UDialogBuilderGraph::InitializeDialogActors()
{
	UWorld* World = GetWorld();
	check(World);

	//retrieve partcipant
	for (TPair<FGameplayTag, FParticipantInfo>& Pair : ParticipantInfoMap)
	{
		FGameplayTag ParticipantTag = Pair.Key;
		FParticipantInfo& ParticipantInfo = Pair.Value;
		if (AActor* ParticipantActor = RetrieveParticipant(ParticipantInfo))
		{
			const FName ParticipantId = ParticipantInfo.GetID();
			CachedActorMap.Emplace(ParticipantId, ParticipantActor);
			CacheOriginalActorTransform(ParticipantId, ParticipantActor);
		}
	}


	if (!SequencePivot)
	{
		FActorSpawnParameters SpawnParams;
		SpawnParams.bNoFail = true;
		SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

		AActor* PivotActor = World->SpawnActor<AActor>(AActor::StaticClass(), FTransform::Identity, SpawnParams);
		
		if (PivotActor)
		{
			USceneComponent* PivotRoot = NewObject<USceneComponent>(PivotActor, TEXT("PivotRoot"));
			PivotActor->SetRootComponent(PivotRoot);
			PivotRoot->RegisterComponent();

			PivotActor->SetActorHiddenInGame(true);
			PivotActor->SetActorEnableCollision(false);

			SequencePivot = PivotActor;
			SpawnedDialogActors.Add(PivotActor);
		}
	}

	
}

void UDialogBuilderGraph::SetDialogStage(UDialogStage* InDialogStage)
{
	if (!CurrentDialogSequence) return;
	if(!InDialogStage || InDialogStage == CurrentDialogStage)
		return;

	SequencePivot->SetActorLocation(InDialogStage->Location);
	SequencePivot->SetActorRotation(InDialogStage->Rotation);
	UDialogStage* LastDialogStage = CurrentDialogStage;
	CurrentDialogStage = InDialogStage;
	
	for (auto& Slot : CurrentDialogStage->Slots)
	{
		if (AActor* DialogActor = RetrieveActorFromSlot(Slot))
		{
			const FName SlotId = Slot->GetActorTag().GetTagName();
			CachedActorMap.Emplace(SlotId, DialogActor);
			CacheOriginalActorTransform(SlotId, DialogActor);
		}
	}
		

	for (auto& Slot : CurrentDialogStage->CameraSlots)
	{
		if (AActor* DialogActor = RetrieveActorFromSlot(Slot))
		{
			const FName SlotId = Slot->GetID();
			CachedActorMap.Emplace(SlotId, DialogActor);
			CacheOriginalActorTransform(SlotId, DialogActor);
		}
	}
	
	UMovieScene* MovieScene = CurrentDialogSequence ? CurrentDialogSequence->GetMovieScene() : nullptr;
	if (!MovieScene) return;
	TArray<UDialogSequenceSlot*> DialogSlots;

	DialogSlots.Append(CurrentDialogStage->Slots);
	DialogSlots.Append(CurrentDialogStage->CameraSlots);

	for (int32 i = 0; i < DialogSlots.Num(); i++)
	{
		UDialogSequenceSlot* DialogSlot = DialogSlots[i];
		AActor* DialogActor = CachedActorMap.FindRef(DialogSlot->GetID());

		if (DialogSlot && DialogActor)
		{
			FGuid ObjectBinding = DialogSlot->ID;

			CurrentDialogSequence->BindPossessableObject(ObjectBinding, *DialogActor, DialogActor->GetClass());
		}
	}
}



void UDialogBuilderGraph::Deinitialize()
{
	VisitedNodeIDs.Empty();
	CurrentNode = nullptr;
	
	for (auto& Node : AllNodes)
	{
		Node->Deinitialize();
	}
}

UWorld* UDialogBuilderGraph::GetWorld() const
{
	if (DialogComponent)
	{
		return DialogComponent->GetWorld();
	}

	return nullptr;
}

void UDialogBuilderGraph::StartFromRoot()
{
	if (RootNodes.IsValidIndex(0))
	{
		if (bFadeCameraOnDialogBegin)
		{
			FTimerDelegate BeginNodeDelegate = FTimerDelegate::CreateLambda([this]()
				{
					BeginNode(RootNodes[0]);
				});
			if (GetWorld())
			{
				FTimerHandle UnfadeHandle;
				GetWorld()->GetTimerManager().SetTimer(UnfadeHandle, BeginNodeDelegate, .75f, false);
			}
		}
		else 
		{
			BeginNode(RootNodes[0]);
		}
		
	}
}

void UDialogBuilderGraph::BeginNode(UDialogBuilderNode* InNode, bool bLaunchEventOnLoad)
{
	if (InNode)
	{
		CurrentNode = InNode;
	}
	if (InNode == nullptr)  
	{  
		UE_LOG(LogTemp, Warning, TEXT("BeginNode: Node is null for Dialog ID: %s"), *ID.ToString());  
		return;  
	}
	
	if (UDialogBuilderNode_Root* RootNode = Cast<UDialogBuilderNode_Root>(InNode))
	{
		RootNode->Initialize();
		return;
	}

	//add visited node
	VisitedNodeIDs.Add(InNode->ID);
	
	//InitializeNode
	InNode->Initialize(bLaunchEventOnLoad);
}

void UDialogBuilderGraph::BeginDialogLine(UDialogBuilderNode_DialogLine* InDialogLine)
{
	if (InDialogLine == nullptr)
		return;
	CurrentLine = InDialogLine;

	AActor* Speaker = CachedActorMap.FindRef(InDialogLine->ParticipantInfo.GetID());
	AActor* Listener = CachedActorMap.FindRef(InDialogLine->ListenerTag.GetTagName());

	DetermineAdvanceDialogRule(InDialogLine);

	//Dialog Sound
	if (InDialogLine->DialogLineData.DialogSound)
	{
		if (Speaker && bLockPlayerMovement)
		{
			DialogAudio = UGameplayStatics::SpawnSoundAttached(InDialogLine->DialogLineData.DialogSound, Speaker->GetRootComponent(), NAME_None, FVector::ZeroVector, EAttachLocation::SnapToTarget, false, 1.f, 1.f, 0.f, DialogSoundAttenuation);
		}
		else
		{
			DialogAudio = UGameplayStatics::SpawnSound2D(this, InDialogLine->DialogLineData.DialogSound);
		}
	}

	if (!bLockPlayerMovement && InDialogLine->ParticipantInfo.ParticipantTag == TAG_Dialog_Participant_Player)
		return;

	DetermineDialogShot(InDialogLine, Speaker, Listener);

	//Dialog Animation 
	if (Speaker && InDialogLine->DialogLineData.DialogMontage)
	{
		UActorComponent* Mesh = Speaker->GetComponentByClass(USkeletalMeshComponent::StaticClass());
		if (USkeletalMeshComponent* BodyMesh = Cast<USkeletalMeshComponent>(Mesh))
		{
			if (BodyMesh->GetAnimInstance())
			{
				BodyMesh->GetAnimInstance()->Montage_Play(InDialogLine->DialogLineData.DialogMontage);
			}
		}
	}

	
	
	CachedLastSpeaker = Speaker;
	
}

void UDialogBuilderGraph::ApplyRotationSetting(UDialogBuilderNode_DialogLine* InDialogLine, AActor* Speaker, AActor* Listener)
{
	//Rotation Setting
	if (Speaker && bAutoRotateParticipant)
	{
		for (auto& Participant : CachedActorMap)
		{
			if (Participant.Value && Participant.Value->IsValidLowLevel())
			{
				if (Participant.Value == Speaker)
					continue;
				FVector ToSpeaker = Speaker->GetActorLocation() - Participant.Value->GetActorLocation();
				ToSpeaker.Z = 0; // Ignore pitch for yaw-only rotation
				if (!ToSpeaker.IsNearlyZero())
				{
					FRotator LookAtRotation = ToSpeaker.Rotation();
					if (GetOwningController() && GetOwningController()->GetPawn() == Participant.Value)
					{
						GetOwningController()->SetControlRotation(LookAtRotation);
					}

					Participant.Value->SetActorRotation(LookAtRotation);
					
				}
			}
		}
	}

	if (Speaker && Listener && InDialogLine->bRotateToListener)
	{
		FVector ToListener = Listener->GetActorLocation() - Speaker->GetActorLocation();
		ToListener.Z = 0; // Ignore pitch for yaw-only rotation
		if (!ToListener.IsNearlyZero())
		{
			FRotator LookAtRotation = ToListener.Rotation();
			Speaker->SetActorRotation(LookAtRotation);
		}

		FVector ToSpeaker = Speaker->GetActorLocation() - Listener->GetActorLocation();
		ToSpeaker.Z = 0; // Ignore pitch for yaw-only rotation
		if (!ToSpeaker.IsNearlyZero())
		{
			FRotator LookAtRotation = ToSpeaker.Rotation();
			Listener->SetActorRotation(LookAtRotation);
		}
	}
}

void UDialogBuilderGraph::DetermineDialogShot(UDialogBuilderNode_DialogLine* InDialogLine, AActor* Speaker, AActor* Listener)
{
	if (!bLockPlayerMovement)
		return;

	ApplyRotationSetting(InDialogLine, Speaker, Listener);

	UDialogCameraShot* DialogShot = nullptr;

	FDialogLineData& DialogLineData = InDialogLine->DialogLineData;
	FParticipantInfo& ParticipantInfo = InDialogLine->ParticipantInfo;

	//Determine the shot
	switch (DialogLineData.DialogCameraMode)
	{
	case EDialogCameraMode::E_GeneratedCameraShot:
		DialogShot = ParticipantInfo.DefaultShot;
		if (!DialogShot ||DialogLineData.ShotOverride)
		{
			//if default shot not valid, try use override shot data instead
			DialogShot = DialogLineData.ShotOverride;
		}

		PlayDialogShot(DialogShot, Speaker);

		break;
	case EDialogCameraMode::E_Sequence:
		PlaySequence(DialogLineData.SequenceToPlay, DialogLineData.PlaybackSettings);
		break;
	}
	CachedLastShot = DialogShot;
}

void UDialogBuilderGraph::BeginChoiceSelection(UDialogBuilderNode_DialogLine* InDialogLine)
{
	if (!bLockPlayerMovement)
	{
		EndDialog();
		return;
	}
	if (InDialogLine == nullptr)
		return;

	UDialogCameraShot* DialogShot = DefaultSelectingChoiceShot;
	AActor* Speaker = CachedActorMap.FindRef(TAG_Dialog_Participant_Player.GetTag().GetTagName());
	Speaker = Speaker ? Speaker : GetOwningPawn();

	if (!DialogShot || InDialogLine->SelectingChoiceShotOverride)
	{
		//if default shot not valid, try use override shot data instead
		DialogShot = InDialogLine->SelectingChoiceShotOverride;
	}

	PlayDialogShot(DialogShot, Speaker);

	CachedLastSpeaker = Speaker;
	CachedLastShot = DialogShot;
}

void UDialogBuilderGraph::BeginDialogSequence(UDialogBuilderNode_DialogSequence* InDialogSequence)
{
	if (!InDialogSequence)
		return;
	if(!InDialogSequence->DialogSequence)
	{
		UE_LOG(LogTemp, Warning, TEXT("BeginDialogSequence: DialogSequence is null for Node ID: %s"), *InDialogSequence->ID.ToString());
		return;
	}
	CurrentDialogSequence = InDialogSequence->DialogSequence;
	InDialogSequence->UpdateDialogStageData();

	SetDialogStage(InDialogSequence->DialogStage);


	DetermineAdvanceDialogRule(InDialogSequence);

	PlaySequence(Cast<ULevelSequence>(InDialogSequence->DialogSequence.Get()), InDialogSequence->PlaybackSettings);

}

void UDialogBuilderGraph::PlayDialogShot(UDialogCameraShot* InDialogShot, AActor* InSpeaker)
{
	if(!InDialogShot || !InSpeaker)
	{
		return;
	}
	//Shot method changed, make sure sequencer is destroyed
	if (DialogSequenceActor)
	{
		if (ULevelSequencePlayer* SP = DialogSequenceActor->GetSequencePlayer())
		{
			SP->Stop();
		}
		DialogSequenceActor->Destroy();
	}

	// Spawn cinecam if it does not exist
	if (!Cinecam.IsValid())
	{
		FActorSpawnParameters SpawnParams;
		SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
		ACineCameraActor* NewCinecam = GetWorld()->SpawnActor<ACineCameraActor>(ACineCameraActor::StaticClass(), SpawnParams);
		Cinecam = NewCinecam;
	}

	InDialogShot->Play(this, InSpeaker);
}

void UDialogBuilderGraph::PlaySequence(class ULevelSequence* InSequence, FMovieSceneSequencePlaybackSettings InPlaybackSetting)
{
	//Shot method changed, make sure cinecam is destroyed
	if (Cinecam.IsValid())
	{
		Cinecam->Destroy();
		Cinecam = nullptr;
	}

	if (!DialogSequenceActor)
	{
		ALevelSequenceActor* OutActor = nullptr;
		ULevelSequencePlayer::CreateLevelSequencePlayer(GetWorld(), InSequence, InPlaybackSetting, OutActor);
		DialogSequenceActor = OutActor;
		if (DialogSequenceActor)
		{
			DialogSequenceActor->SetSequence(nullptr);
		}
	}
	if (DialogSequenceActor)
	{
		if (ULevelSequencePlayer* SP = DialogSequenceActor->GetSequencePlayer())
		{
			SP->OnFinished.RemoveAll(this);
			if (SP->IsPlaying())
			{
				SP->Stop();
			}
			if (InSequence)
			{
				DialogSequenceActor->PlaybackSettings = InPlaybackSetting;
				DialogSequenceActor->SetSequence(InSequence);
			}
			SP->Play();
		}
	}
}

FText UDialogBuilderGraph::GetParticipantName(FGameplayTag ParticipantTag) const
{
	if(!ParticipantTag.IsValid())
	{
		return FText();
	}
	for (auto& Participant : ParticipantDefinitions)
	{
		if (Participant->Tag == ParticipantTag)
		{
			return Participant->DisplayName;
		}
	}

	FParticipantInfo ParticipantInfo;
	if (const FParticipantInfo* FoundInfo = ParticipantInfoMap.Find(ParticipantTag))
	{
		ParticipantInfo = *FoundInfo;
	}
	else
	{
		return FText();
	}
	return ParticipantInfo.ParticipantName.IsEmpty() ? FText() : ParticipantInfo.ParticipantName;
}

int UDialogBuilderGraph::GetLevelNum() const
{
	int Level = 0;
	TArray<UDialogBuilderNode*> CurrLevelNodes = RootNodes;
	TArray<UDialogBuilderNode*> NextLevelNodes;

	while (CurrLevelNodes.Num() != 0)
	{
		for (int i = 0; i < CurrLevelNodes.Num(); ++i)
		{
			UDialogBuilderNode* Node = CurrLevelNodes[i];
			check(Node != nullptr);

			for (int j = 0; j < Node->ChildrenNodes.Num(); ++j)
			{
				NextLevelNodes.Add(Node->ChildrenNodes[j]);
			}
		}

		CurrLevelNodes = NextLevelNodes;
		NextLevelNodes.Reset();
		++Level;
	}

	return Level;
}

void UDialogBuilderGraph::GetNodesByLevel(int Level, TArray<UDialogBuilderNode*>& Nodes)
{
	int CurrLEvel = 0;
	TArray<UDialogBuilderNode*> NextLevelNodes;

	Nodes = RootNodes;

	while (Nodes.Num() != 0)
	{
		if (CurrLEvel == Level)
			break;

		for (int i = 0; i < Nodes.Num(); ++i)
		{
			UDialogBuilderNode* Node = Nodes[i];
			check(Node != nullptr);

			for (int j = 0; j < Node->ChildrenNodes.Num(); ++j)
			{
				NextLevelNodes.Add(Node->ChildrenNodes[j]);
			}
		}

		Nodes = NextLevelNodes;
		NextLevelNodes.Reset();
		++CurrLEvel;
	}
}

void UDialogBuilderGraph::ClearGraph()
{
	for (int i = 0; i < AllNodes.Num(); ++i)
	{
		UDialogBuilderNode* Node = AllNodes[i];
		if (Node)
		{
			Node->ParentNodes.Empty();
			Node->ChildrenNodes.Empty();
			Node->Edges.Empty();
		}
	}

	ParticipantInfoMap.Empty();
	VisitedNodeIDs.Empty();
	AllNodes.Empty();
	RootNodes.Empty();
	NodeMap.Empty();
}

APlayerController* UDialogBuilderGraph::GetOwningController()
{
	return OwningController;
}

APawn* UDialogBuilderGraph::GetOwningPawn()
{
	if (DialogComponent)
	{
		return DialogComponent->GetOwningPawn();
	}
	return nullptr;
}

UDialogComponent* UDialogBuilderGraph::GetDialogComponent()
{
	return DialogComponent;
}

void UDialogBuilderGraph::AdvanceDialogLine()
{
	if (CurrentLine && !CurrentLine->bCanSkipDialogLine 
		&& DialogLineTimer.IsValid() 
		&& GetWorld() && GetWorld()->GetTimerManager().GetTimerRemaining(DialogLineTimer) > .0f)
	{
		return;
	}


	if (GetWorld())
	{
		GetWorld()->GetTimerManager().ClearTimer(DialogLineTimer);
	}
	StopCurrentDialogLine();
	if (CurrentNode)
	{
		CurrentNode->EvaluateNextNode();
	}
}

void UDialogBuilderGraph::StopCurrentDialogLine()
{
	if (DialogAudio)
	{
		//Remove any previously added bindings
		DialogAudio->OnAudioFinished.RemoveAll(this);
		DialogAudio->Stop();
		DialogAudio->DestroyComponent();
		DialogAudio = nullptr;
	}

	UDialogBuilderNode_DialogLine* CurrentDialogLine = Cast<UDialogBuilderNode_DialogLine>(CurrentNode);
	if (CurrentDialogLine && CurrentDialogLine->DialogLineData.DialogMontage && CachedLastSpeaker.IsValid())
	{
		UActorComponent* Mesh = CachedLastSpeaker->GetComponentByClass(USkeletalMeshComponent::StaticClass());
		if (USkeletalMeshComponent* BodyMesh = Cast<USkeletalMeshComponent>(Mesh))
		{
			if (BodyMesh->GetAnimInstance())
			{
				const float BlendOutTime = CurrentDialogLine->DialogLineData.DialogMontage->BlendOut.GetBlendTime();
				BodyMesh->GetAnimInstance()->Montage_Stop(BlendOutTime, CurrentDialogLine->DialogLineData.DialogMontage);
			}
		}

	}
}

void UDialogBuilderGraph::EndDialog()
{
	//destroy spawned participant
	for (auto& SpawnedParticipant : SpawnedDialogActors)
	{
		SpawnedParticipant->Destroy();
	}

	if (bLockPlayerMovement)
	{
		if (bRestoreState)
		{
			RestoreCachedActorTransforms();
		}
        
	}
	
	
	//clear shot and sequence
	CurrentDialogSequence = nullptr;
	SequencePivot = nullptr;
	if (DialogSequenceActor)
	{
		if (ULevelSequencePlayer* SP = DialogSequenceActor->GetSequencePlayer())
		{
			SP->Stop();
		}

		DialogSequenceActor->Destroy();
	}

	if (Cinecam.IsValid())
	{
		if (bBlendCameraOnDialogEnd)
		{
			OwningController->SetViewTargetWithBlend(
				GetOwningPawn(),
				BlendTime,
				BlendFunc,
				BlendExp,
				true // bLockOutgoing
			);
			Cinecam->SetLifeSpan(BlendTime);
		}
		else
		{
			Cinecam->Destroy();
		}
		Cinecam = nullptr;
	}

	if (DialogCameraShake && OwningController)
	{
		OwningController->ClientStopCameraShake(DialogCameraShake);
	}

	

	StopCurrentDialogLine();
	GetDialogComponent()->OnEndDialog.Broadcast(this);
	GetWorld()->GetTimerManager().ClearTimer(DialogLineTimer);
}

UDialogDefinition* UDialogBuilderGraph::GetDialogDefinitionByTag(FGameplayTag InTag)
{
	// Search ParticipantDefinitions
	for (auto&  Participant : ParticipantDefinitions)
	{
		if (!Participant) continue;
		// Matches tag?
		if (Participant->Tag == InTag)
		{
			return Participant;
		}
	}

	for (auto& Prop : PropDefinitions)
	{
		if (!Prop) continue;
		// Matches tag?
		if (Prop->Tag == InTag)
		{
			return Prop;
		}
	}

	return nullptr;
}

void UDialogBuilderGraph::DetermineAdvanceDialogRule(UDialogBuilderNode* InDialogNode)
{
	if(!GetWorld())
		return;
	//Determine each dialog time before advance to the next line
	if (bAutoAdvanceDialogLine)
	{
		if (UDialogBuilderNode_DialogLine* InDialogLine = Cast<UDialogBuilderNode_DialogLine>(InDialogNode))
		{
			GetWorld()->GetTimerManager().SetTimer(DialogLineTimer, this, &ThisClass::AdvanceDialogLine, InDialogLine->GetLineDuration(), false);
		}
		
	}
	if (UDialogBuilderNode_DialogSequence* InDialogSequence = Cast< UDialogBuilderNode_DialogSequence>(InDialogNode))
	{
		GetWorld()->GetTimerManager().SetTimer(DialogLineTimer, this, &ThisClass::AdvanceDialogLine, InDialogSequence->GetSequenceDuration(), false);
	}

}

FVector UDialogBuilderGraph::GetSpeakerBoneLocation(AActor* Actor, bool bOverrideTrackedBone, FName BoneOverride) const
{
	if (!Actor)
	{
		return FVector::ZeroVector;
	}

	FVector BoneLoc;
	FRotator BoneRot;


	for (auto& ActorSkelMesh : Actor->GetComponents())
	{
		if (USkeletalMeshComponent* SkelMesh = Cast<USkeletalMeshComponent>(ActorSkelMesh))
		{
			BoneLoc = SkelMesh->GetBoneLocation(DefaultBoneToTrack);
			break;
		}
	}

	if (bOverrideTrackedBone)
	{
		for (auto& ActorSkelMesh : Actor->GetComponents())
		{
			if (USkeletalMeshComponent* SkelMesh = Cast<USkeletalMeshComponent>(ActorSkelMesh))
			{
				FVector FoundedLocation;
				FoundedLocation = SkelMesh->GetBoneLocation(BoneOverride);
				if(!FoundedLocation.IsNearlyZero())
				{
					BoneLoc = FoundedLocation;
				}
				break;
			}
		}
	}
	

	if (BoneLoc == FVector::ZeroVector)
	{
		Actor->GetActorEyesViewPoint(BoneLoc, BoneRot);
	}

	return BoneLoc;
}

AActor* UDialogBuilderGraph::RetrieveParticipant(FParticipantInfo& Info)
{
	APawn* PlayerPawn = GetOwningPawn();
	
	//retrieve player
	if (PlayerPawn && Info.ParticipantTag == TAG_Dialog_Participant_Player)
	{
		if (bOverrideTransform)
		{
			PlayerPawn->SetActorTransform(PlayerTransformOverride);
		}
		return PlayerPawn;
	}

	//Retrieve other participant
	if(!Info.ParticipantTag.GetTagName().IsNone())
	{
		if (!CachedActorMap.Contains(Info.GetID()) && 
			IsValid(Info.ParticipantToSpawn) &&
			Info.ParticipantSetup == EParticipantSetup::E_SpawnParticipant)
		{
			FActorSpawnParameters SpawnParams;
			SpawnParams.bNoFail = true;
			SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
			SpawnParams.Owner = OwningController;

			
			if (UWorld* World = GetWorld())
			{
				AActor* SpawnedParticipant = World->SpawnActor(Info.ParticipantToSpawn, &Info.ParticipantTransform, SpawnParams);
				SpawnedDialogActors.AddUnique(SpawnedParticipant);
				return SpawnedParticipant;
			}
		}
		else
		{
			TArray<AActor*> FoundActors;

			for (FActorIterator It(GetWorld()); It; ++It)
			{
				AActor* Actor = *It;

				if (Actor && Actor->ActorHasTag(Info.ParticipantTag.GetTagName()))
				{
					FoundActors.Add(Actor);
				}
			}

			if (FoundActors.Num() > 1 && GetOwningPawn())
			{
				float Dist;

				return UGameplayStatics::FindNearestActor(GetOwningPawn()->GetActorLocation(), FoundActors, Dist);
			}
			else if (FoundActors.IsValidIndex(0))
			{
				return FoundActors[0];
			}
		}
	}
	

	return nullptr;
}


AActor* UDialogBuilderGraph::RetrieveActorFromSlot(UDialogSequenceSlot* InSlot)
{
	if (!InSlot)
	{
		UE_LOG(LogTemp, Warning, TEXT("RetrieveParticipant: Slot is null for Dialog ID: %s"), *ID.ToString());
		return nullptr;
	}

	UWorld* World = GetWorld();
	if (!World)
	{
		return nullptr;
	}

	UDialogDefinition* InDialogDefinition = InSlot->DialogDefinition;
	if (!InDialogDefinition)
	{
		return nullptr;
	}

	const TSubclassOf<AActor> ActorClass = InDialogDefinition->GetActorClass();

	// Look up by the slot's ID (FName) which is the map's key type
	if (TObjectPtr<AActor> ExistingActor = CachedActorMap.FindRef(InSlot->GetActorTag().GetTagName()))
	{
		ExistingActor->AttachToActor(SequencePivot, FAttachmentTransformRules::KeepWorldTransform);
		ExistingActor->SetActorRelativeLocation(InSlot->SlotLocation);
		ExistingActor->SetActorRelativeRotation(InSlot->SlotRotation);
		return ExistingActor;
	}


	FActorSpawnParameters SpawnParams;
	SpawnParams.bNoFail = true;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	SpawnParams.Owner = OwningController;

	AActor* NewActor = World->SpawnActor<AActor>(ActorClass, FTransform::Identity, SpawnParams);
	if (NewActor)
	{
		NewActor->AttachToActor(SequencePivot, FAttachmentTransformRules::KeepWorldTransform);
		NewActor->SetActorRelativeLocation(InSlot->SlotLocation);
		NewActor->SetActorRelativeRotation(InSlot->SlotRotation);

		SpawnedDialogActors.AddUnique(NewActor);
	}

	return NewActor;
}

void UDialogBuilderGraph::CacheOriginalActorTransform(const FName& ActorId, AActor* Actor)
{
	if (!Actor || ActorId.IsNone())
	{
		return;
	}

	if (CachedActorOriginalTransforms.Contains(ActorId))
	{
		return;
	}

	if (SpawnedDialogActors.Contains(Actor))
	{
		return;
	}

	CachedActorOriginalTransforms.Emplace(ActorId, Actor->GetActorTransform());
}

void UDialogBuilderGraph::RestoreCachedActorTransforms()
{
	for (const TPair<FName, TObjectPtr<AActor>>& Pair : CachedActorMap)
	{
		AActor* Actor = Pair.Value;
		if (!Actor)
		{
			continue;
		}

		if (SpawnedDialogActors.Contains(Actor))
		{
			continue;
		}

		const FTransform* OriginalTransform = CachedActorOriginalTransforms.Find(Pair.Key);
		if (!OriginalTransform)
		{
			continue; 
		}

		Actor->DetachFromActor(FDetachmentTransformRules::KeepWorldTransform);
		Actor->SetActorTransform(*OriginalTransform, false, nullptr, ETeleportType::TeleportPhysics);
	}

	CachedActorOriginalTransforms.Empty();
}

#if WITH_EDITOR

void UDialogBuilderGraph::GetAllGraphs(TArray<UEdGraph*>& Graphs) const
{
	

	for (int32 i = 0; i < DialogGraphPages.Num(); ++i)
	{
		UEdGraph* Graph = DialogGraphPages[i];
		if (Graph)
		{
			Graphs.Add(Graph);
			Graph->GetAllChildrenGraphs(Graphs);
		}
	}
	
}
void UDialogBuilderGraph::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);
}
#endif // WITH_EDITORONLY_DATA

#undef LOCTEXT_NAMESPACE

void UDialogBuilderGraph::GetNodesOfType(TSubclassOf<UDialogBuilderNode> NodeType, TArray<UDialogBuilderNode*>& Nodes) const
{
	Nodes.Reset();

	if (!*NodeType)
	{
		return;
	}

	for (UDialogBuilderNode* Node : AllNodes)
	{
		if (Node && Node->IsA(NodeType))
		{
			Nodes.Add(Node);
		}
	}
}