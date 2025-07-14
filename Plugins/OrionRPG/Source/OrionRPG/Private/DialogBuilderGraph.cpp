// Copyright 2025 Ivan Chandra. All Rights Reserved.


#include "DialogBuilderGraph.h"
#include "DialogComponent.h"
#include "DialogBuilderNode.h"
#include "DialogBuilderEdge.h"
#include "DialogBuilderNode_Root.h"
#include "LevelSequencePlayer.h"
#include "LevelSequenceActor.h"
#include "DialogBuilderNode_RerouteNode.h"
#include "DialogBuilderNode_DialogLine.h"
#include "EngineUtils.h"
#include "GameFramework/Character.h"
#include <CineCameraActor.h>
#include <DefaultLevelSequenceInstanceData.h>
#include <CineCameraComponent.h>
#include "Kismet/GameplayStatics.h"

#define LOCTEXT_NAMESPACE "DialogBuilderGraph"


static const FName NAME_CinecamTag("Cinecam");
static const FName NAME_BodyTag("Body");

UDialogBuilderGraph::UDialogBuilderGraph()
{
	DefaultHeadBoneName = FName("head");
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

	if (UDialogBuilderNode_RerouteNode* RerouteNode = Cast<UDialogBuilderNode_RerouteNode>(InNode))
	{
		RerouteNode->BeginNode();
		return;
	}
	//add visited node
	VisitedNodeIDs.AddUnique(InNode->ID);
	
	//InitializeNode
	InNode->Initialize(bLaunchEventOnLoad);
}

void UDialogBuilderGraph::PlayDialogueSequence(UDialogBuilderNode_DialogLine* InDialogLine)
{
		
	if (!DialogSequencer)
	{
		ALevelSequenceActor* OutActor = nullptr;
		ULevelSequencePlayer::CreateLevelSequencePlayer(GetWorld(), InDialogLine->SequenceAsset, InDialogLine->PlaybackSettings, OutActor);
		DialogSequencer = OutActor;
		//passing CreateLevelSequencePlayer null asset makes it fail so we need to set it afterwards
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
			if (InDialogLine->SequenceAsset)
			{
				DialogSequencer->PlaybackSettings = InDialogLine->PlaybackSettings;
				DialogSequencer->SetSequence(InDialogLine->SequenceAsset);
			}
			SP->Play();
			
			SP->OnFinished.AddDynamic(this, &ThisClass::AdvanceDialogLine);

			//Determine the LookAtActor
			LookAtActor = CachedParticipants.FindRef(InDialogLine->ParticipantInfo.ParticipantTag);
			

			//Go in, and tell the cinecam to focus/track the speaker 
			for (auto& BoundObject : SP->GetBoundObjects(DialogSequencer->FindNamedBinding(NAME_CinecamTag)))
			{
				Cinecam = Cast<ACineCameraActor>(BoundObject);

				if (Cinecam.IsValid())
				{
					if (UCineCameraComponent* CinecamComp = Cinecam->GetCineCameraComponent())
					{
						CinecamComp->CropSettings = InDialogLine->CropSettings;

						if (LookAtActor.IsValid())
						{
							const FVector Offset = LookAtActor->GetActorTransform().InverseTransformPosition(GetSpeakerHeadLocation(LookAtActor.Get()));
							//Focus the cinecam on the speaker 
							Cinecam->LookatTrackingSettings.bEnableLookAtTracking = true;
							Cinecam->LookatTrackingSettings.ActorToTrack = LookAtActor.Get();
							Cinecam->LookatTrackingSettings.RelativeOffset = Offset + FVector(10.f, 0.f, 0.f);// LookAtTrackingSettings.TrackBoneNudge;

							//Tracked shots should still offset themselves if using 180d rule
							/*if (bUse180DegreeRule)
							{
								Cinecam->LookatTrackingSettings.RelativeOffset += FVector(0.f, (Speaker.Get() == Dialogue->GetPlayerAvatar() ? UnitsY180DegreeRule : -UnitsY180DegreeRule), 0.f);
							}*/
						}
					}
				}
			}
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

AController* UDialogBuilderGraph::GetOwningController()
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
	if (CurrentNode)
	{
		CurrentNode->EvaluateNextNode();
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
}

FVector UDialogBuilderGraph::GetSpeakerHeadLocation(AActor* Actor) const
{
	if (!Actor)
	{
		return FVector::ZeroVector;
	}

	FVector EyesLoc;
	FRotator EyesRot;
	Actor->GetActorEyesViewPoint(EyesLoc, EyesRot);

	const bool bIsChar = Actor->IsA<ACharacter>();

	for (auto& ActorSkelMesh : Actor->GetComponents())
	{
		if (USkeletalMeshComponent* SkelMesh = Cast<USkeletalMeshComponent>(ActorSkelMesh))
		{
			//Dont use face mesh, body meshes seem more consistent in testing
			if (SkelMesh->ComponentHasTag(NAME_BodyTag))
			{
				EyesLoc = SkelMesh->GetBoneLocation(DefaultHeadBoneName);
				//EyesLoc.Z = SkelMesh->GetBoneLocation(DefaultHeadBoneName).Z;
				break;
			}
		}
	}

	return EyesLoc;
}

AActor* UDialogBuilderGraph::RetrieveParticipant(const FParticipantInfo& Info)
{
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
			APawn* PlayerPawn = GetOwningPawn();
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