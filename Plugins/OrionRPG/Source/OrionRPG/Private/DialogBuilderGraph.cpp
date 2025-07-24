// Copyright 2025 Ivan Chandra. All Rights Reserved.


#include "DialogBuilderGraph.h"
#include "DialogComponent.h"
#include "DialogBuilderNode.h"
#include "DialogBuilderEdge.h"
#include "DialogCameraShot.h"
#include "DialogBuilderNode_Root.h"
#include "DialogBuilderSetting.h"
#include "LevelSequencePlayer.h"
#include "LevelSequenceActor.h"
#include "DialogBuilderNode_RerouteNode.h"
#include "DialogBuilderNode_DialogLine.h"
#include "EngineUtils.h"
#include "GameFramework/Character.h"
#include <CineCameraActor.h>
#include <DefaultLevelSequenceInstanceData.h>
#include <CineCameraComponent.h>
#include "Sound/SoundBase.h"
#include "Components/AudioComponent.h"
#include "Kismet/GameplayStatics.h"

#define LOCTEXT_NAMESPACE "DialogBuilderGraph"


UDialogBuilderGraph::UDialogBuilderGraph()
{
	bAutoRotateParticipant = false;
	DefaultBoneToTrack = FName("head");
	bAutoAdvanceDialogLine = true;
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
		CachedParticipants.Empty();
		NodeMap.Empty();
		for (auto& Node : AllNodes)
		{
			if (Node)
			{
				Node->DialogGraph = this;
				Node->OwningController = InitializingComp->GetOwningController();
				Node->DialogComponent = InitializingComp;
				NodeMap.Emplace(Node->ID, Node);
			}
		}

		TArray<FParticipantInfo> ParticipantList;
		ParticipantInfoMap.GenerateValueArray(ParticipantList);
		for (auto& Participant : ParticipantList)
		{
			if(AActor* ParticipantActor = RetrieveParticipant(Participant))
			{
				CachedParticipants.Emplace(Participant.ParticipantTag, ParticipantActor);

				// Log the cached participant
				UE_LOG(LogTemp, Log, TEXT("Cached participant: %s -> %s"), *Participant.ParticipantName.ToString(), *GetNameSafe(ParticipantActor));
			}
			else
			{
				UE_LOG(LogTemp, Warning, TEXT("Failed to retrieve participant: %s for Dialog : %s"), *Participant.ParticipantName.ToString(), *ID.ToString());
			}
		}


		if (OwningController && OwningController->IsLocalPlayerController())
		{
			if (DialogCameraShake)
			{
				OwningController->ClientStartCameraShake(DialogCameraShake);
			}
		}

	}
	return false;
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

	if (GetWorld() && bAutoAdvanceDialogLine)
	{
		FString LineString = InDialogLine->DialogLineData.Line.ToString();

		// Split the string into words (splits on whitespace)
		TArray<FString> Words;
		LineString.ParseIntoArrayWS(Words);
		float DialogLineDuration = Words.Num() / GetDefault<UDialogBuilderSetting>()->DialogLineWordsPerSecond;
		DialogLineDuration = FMath::Max(GetDefault<UDialogBuilderSetting>()->MinDialogLineDuration, DialogLineDuration);
		GetWorld()->GetTimerManager().SetTimer(DialogLineTimer, this, &ThisClass::AdvanceDialogLine, DialogLineDuration, false);
	}

	UDialogCameraShot* DialogShot = nullptr;
	AActor* Speaker = CachedParticipants.FindRef(InDialogLine->ParticipantInfo.ParticipantTag);
	AActor* Listener = CachedParticipants.FindRef(InDialogLine->ListenerTag);

	ApplyRotationSetting(InDialogLine, Speaker, Listener);

	//Determine the shot
	switch (InDialogLine->DialogLineData.DialogCameraMode)
	{
		case EDialogCameraMode::E_GeneratedCameraShot:
			DialogShot = InDialogLine->ParticipantInfo.DefaultShot;
			if (!DialogShot || InDialogLine->DialogLineData.ShotOverride)
			{
				//if default shot not valid, try use override shot data instead
				DialogShot = InDialogLine->DialogLineData.ShotOverride;
			}

			PlayDialogShot(DialogShot, Speaker);
			
			break;
		case EDialogCameraMode::E_Sequence:
			
			PlayDialogSequence(InDialogLine->DialogLineData);
			break;
	}

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

	//Dialog Sound
	if (InDialogLine->DialogLineData.DialogSound)
	{
		if (Speaker)
		{
			DialogAudio = UGameplayStatics::SpawnSoundAttached(InDialogLine->DialogLineData.DialogSound, Speaker->GetRootComponent(), NAME_None, FVector::ZeroVector, EAttachLocation::SnapToTarget, false, 1.f, 1.f, 0.f, DialogSoundAttenuation);
		}
		else 
		{
			DialogAudio = UGameplayStatics::SpawnSound2D(DialogComponent, InDialogLine->DialogLineData.DialogSound);
		}

		
	}

	
	CachedLastSpeaker = Speaker;
	CachedLastShot = DialogShot;
	
}

void UDialogBuilderGraph::ApplyRotationSetting(UDialogBuilderNode_DialogLine* InDialogLine, AActor* Speaker, AActor* Listener)
{
	//Rotation Setting
	if (Speaker && bAutoRotateParticipant)
	{
		for (auto& Participant : CachedParticipants)
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

void UDialogBuilderGraph::BeginChoiceSelection(UDialogBuilderNode_DialogLine* InDialogLine)
{
	if (InDialogLine == nullptr)
		return;

	UDialogCameraShot* DialogShot = DefaultSelectingChoiceShot;
	AActor* Speaker = CachedParticipants.FindRef(TAG_Dialog_Participant_Player);
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

void UDialogBuilderGraph::PlayDialogShot(UDialogCameraShot* InDialogShot, AActor* InSpeaker)
{
	if(!InDialogShot)
	{
		UE_LOG(LogTemp, Warning, TEXT("PlayDialogShot: Invalid shot or speaker for Dialog ID: %s"), *ID.ToString());
		return;
	}
	//Shot method changed, make sure sequencer is destroyed
	if (DialogSequencer)
	{
		if (ULevelSequencePlayer* SP = DialogSequencer->GetSequencePlayer())
		{
			SP->Stop();
		}
		DialogSequencer->Destroy();
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

void UDialogBuilderGraph::PlayDialogSequence(const FDialogLineData& InDialogLineData)
{
	//Shot method changed, make sure cinecam is destroyed
	if (Cinecam.IsValid())
	{
		Cinecam->Destroy();
		Cinecam = nullptr;
	}

	if (!DialogSequencer)
	{
		ALevelSequenceActor* OutActor = nullptr;
		ULevelSequencePlayer::CreateLevelSequencePlayer(GetWorld(), InDialogLineData.SequenceToPlay, InDialogLineData.PlaybackSettings, OutActor);
		DialogSequencer = OutActor;
		if (DialogSequencer)
		{
			DialogSequencer->SetSequence(nullptr);
		}
	}
	if (DialogSequencer)
	{
		if (ULevelSequencePlayer* SP = DialogSequencer->GetSequencePlayer())
		{
			SP->OnFinished.RemoveAll(this);
			if (SP->IsPlaying())
			{
				SP->Stop();
			}
			if (InDialogLineData.SequenceToPlay)
			{
				DialogSequencer->PlaybackSettings = InDialogLineData.PlaybackSettings;
				DialogSequencer->SetSequence(InDialogLineData.SequenceToPlay);
			}
			SP->Play();


		}
	}
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
	GetDialogComponent()->OnEndDialog.Broadcast(this);

	if (DialogSequencer)
	{
		if (ULevelSequencePlayer* SP = DialogSequencer->GetSequencePlayer())
		{
			SP->Stop();
		}

		DialogSequencer->Destroy();
	}

	if (Cinecam.IsValid())
	{
		Cinecam->Destroy();
		Cinecam = nullptr;
	}

	if (DialogCameraShake && OwningController)
	{
		OwningController->ClientStopCameraShake(DialogCameraShake);
	}

}

FVector UDialogBuilderGraph::GetSpeakerBoneLocation(AActor* Actor) const
{
	if (!Actor)
	{
		return FVector::ZeroVector;
	}

	FVector EyesLoc;
	FRotator EyesRot;

	const bool bIsChar = Actor->IsA<ACharacter>();

	for (auto& ActorSkelMesh : Actor->GetComponents())
	{
		if (USkeletalMeshComponent* SkelMesh = Cast<USkeletalMeshComponent>(ActorSkelMesh))
		{
			EyesLoc = SkelMesh->GetBoneLocation(DefaultBoneToTrack);
			break;
		}
	}

	if (EyesLoc == FVector::ZeroVector)
	{
		Actor->GetActorEyesViewPoint(EyesLoc, EyesRot);
	}

	return EyesLoc;
}

AActor* UDialogBuilderGraph::RetrieveParticipant(const FParticipantInfo& Info)
{
	APawn* PlayerPawn = GetOwningPawn();
	
	//retrieve player
	if (Info.ParticipantTag == TAG_Dialog_Participant_Player)
	{
		return PlayerPawn;
	}

	//Retrieve other participant
	if(!Info.ParticipantTag.GetTagName().IsNone())
	{
		if (!CachedParticipants.Contains(Info.ParticipantTag) && IsValid(Info.ParticipantClassOverride))
		{
			FActorSpawnParameters SpawnParams;
			SpawnParams.bNoFail = true;
			SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
			SpawnParams.Owner = OwningController;

			FTransform SpawnTransform;

			// If we have a valid player pawn, spawn in front of it within a cone
			
			if (PlayerPawn)
			{
				const FVector PawnLocation = PlayerPawn->GetActorLocation();
				const FRotator PawnRotation = PlayerPawn->GetActorRotation();
				const FVector ForwardVector = PawnRotation.Vector();

				// Parameters for cone
				const float Distance = 200.f; // Distance in front of pawn
				const float ConeHalfAngleDegrees = 30.f; // Half angle of cone
				const float RandomAngle = FMath::FRandRange(-ConeHalfAngleDegrees, ConeHalfAngleDegrees);

				// Calculate random direction within cone
				const FRotator OffsetRot = FRotator(0.f, RandomAngle, 0.f);
				const FVector ConeDirection = OffsetRot.RotateVector(ForwardVector);

				const FVector SpawnLocation = PawnLocation + ConeDirection * Distance;
				const FRotator SpawnRotation = PawnRotation;

				SpawnTransform.SetLocation(SpawnLocation);
				SpawnTransform.SetRotation(SpawnRotation.Quaternion());
			}

			if (UWorld* World = GetWorld())
			{
				return World->SpawnActor(Info.ParticipantClassOverride, &SpawnTransform, SpawnParams);
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

#if WITH_EDITORONLY_DATA

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
#endif // WITH_EDITORONLY_DATA

#undef LOCTEXT_NAMESPACE