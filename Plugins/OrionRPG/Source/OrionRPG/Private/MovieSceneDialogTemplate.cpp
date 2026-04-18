// Copyright 2025 Ivan Chandra. All Rights Reserved.

#include "MovieSceneDialogTemplate.h"
#include "MovieSceneDialogSection.h"
#include "OrionDialogInterface.h"

#include "Engine/Engine.h"
#include "DialogBuilderSetting.h"
#include "DialogDefinition.h"
#include "DialogComponent.h"
#include "DialogBuilderFunctionLibrary.h"
#include "Engine/GameViewportClient.h"
#include "Engine/World.h"
#include "Evaluation/MovieSceneExecutionTokens.h"
#include "IMovieScenePlayer.h"
#include "Widgets/SWidget.h"
#include "Widgets/SViewport.h"
#include "CommonActivatableWidget.h"
#include "Blueprint/UserWidget.h"
#include "Components/AudioComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Animation/AnimSequence.h"
#include "Animation/Skeleton.h"
#include "Engine/SkeletalMesh.h"
#include "Kismet/GameplayStatics.h"
#include "Sound/SoundBase.h"

#if WITH_EDITOR
#include "LevelEditor.h"
#include "Editor.h"
#include "Slate/SceneViewport.h"
#include "EditorStyleSet.h"
#endif

namespace OrionDialogTemplate
{
	static FSharedPersistentDataKey GetSharedDataKey()
	{
		static FMovieSceneSharedDataId DataId(FMovieSceneSharedDataId::Allocate());
		return FSharedPersistentDataKey(DataId, FMovieSceneEvaluationOperand());
	}

	struct FDialogSharedTrackData : IPersistentEvaluationData
	{
		FDialogSharedTrackData()
			: bNeedExecute(false)
			, bShow(false)
		{
		}

		~FDialogSharedTrackData()
		{
			if (DialogWidget.IsValid())
			{
				DialogWidget->RemoveFromParent();
			}

			StopDialogAudio();
			StopDialogFacialAnimation();

			DialogWidget.Reset();
		}

		void StopDialogAudio()
		{
			if (DialogAudioComponent.IsValid())
			{
				DialogAudioComponent->Stop();
				DialogAudioComponent->DestroyComponent();
				DialogAudioComponent.Reset();
			}

			bHasLastAudioEvalTime = false;
			LastAudioEvalTime = FFrameTime(TNumericLimits<int32>::Lowest());
			LastAudioEvalDirection = EPlayDirection::Forwards;
		}

		void StopDialogFacialAnimation()
		{
			if (FacialMeshComponent.IsValid())
			{
				FacialMeshComponent->Stop();
				FacialMeshComponent.Reset();
			}

			SpeakerActor.Reset();
			FacialAnimationAsset.Reset();
			bHasLastFacialEvalTime = false;
			LastFacialEvalTime = FFrameTime(TNumericLimits<int32>::Lowest());
			LastFacialEvalDirection = EPlayDirection::Forwards;
		}

		void UpdateDialogAudio(const FMovieSceneContext& Context, IMovieScenePlayer& Player, const UMovieSceneDialogSection* InDialogSection, const FOrionDialogLine& InDialogLine)
		{
			if (!InDialogSection || !InDialogLine.DialogSound || !InDialogSection->HasStartFrame() || !InDialogSection->HasEndFrame())
			{
				StopDialogAudio();
				return;
			}

			const EMovieScenePlayerStatus::Type Status = Context.GetStatus();
			const bool bIsPlayingLike =
				Status == EMovieScenePlayerStatus::Playing ||
				Status == EMovieScenePlayerStatus::Scrubbing ||
				Status == EMovieScenePlayerStatus::Stepping;

			if (Context.IsSilent() || !bIsPlayingLike)
			{
				StopDialogAudio();
				return;
			}

			const FFrameNumber CurrentFrame = Context.GetTime().FloorToFrame();
			if (CurrentFrame < InDialogSection->GetInclusiveStartFrame() || CurrentFrame >= InDialogSection->GetExclusiveEndFrame())
			{
				StopDialogAudio();
				return;
			}

			UWorld* World = Player.GetPlaybackContext() ? Player.GetPlaybackContext()->GetWorld() : nullptr;
			if (!World)
			{
				StopDialogAudio();
				return;
			}

			const FFrameTime RelativeFrameTime = Context.GetTime() - InDialogSection->GetInclusiveStartFrame();

			float StartTimeSeconds = FMath::Max(0.0f, Context.GetFrameRate().AsSeconds(RelativeFrameTime));
			const float SoundDuration = InDialogLine.DialogSound->GetDuration();
			if (SoundDuration > 0.f && SoundDuration != INDEFINITELY_LOOPING_DURATION)
			{
				StartTimeSeconds = FMath::Min(StartTimeSeconds, FMath::Max(0.0f, SoundDuration - KINDA_SMALL_NUMBER));
			}

			const bool bIsBackwards = Context.GetDirection() == EPlayDirection::Backwards;
			const bool bIsScrubbing =
				Context.GetStatus() == EMovieScenePlayerStatus::Scrubbing ||
				Context.GetStatus() == EMovieScenePlayerStatus::Stepping;

			const bool bDirectionChanged = !bHasLastAudioEvalTime || (LastAudioEvalDirection != Context.GetDirection());
			const bool bTimeChanged = !bHasLastAudioEvalTime || (LastAudioEvalTime != Context.GetTime());

			const bool bForceResyncForScrub = bIsScrubbing && bTimeChanged;
			const bool bForceResyncForReverse = bIsBackwards && bTimeChanged;

			UAudioComponent* AudioComponent = DialogAudioComponent.Get();
			const bool bNeedsRestart =
				!AudioComponent ||
				AudioComponent->GetSound() != InDialogLine.DialogSound ||
				!AudioComponent->IsPlaying() ||
				Context.HasJumped() ||
				Context.HasLooped() ||
				bDirectionChanged ||
				bForceResyncForScrub ||
				bForceResyncForReverse;

			if (bNeedsRestart)
			{
				StopDialogAudio();
				DialogAudioComponent = UGameplayStatics::SpawnSound2D(
					World,
					InDialogLine.DialogSound,
					1.0f,
					1.0f,
					StartTimeSeconds,
					nullptr,
					false,
					false);
			}

			bHasLastAudioEvalTime = true;
			LastAudioEvalTime = Context.GetTime();
			LastAudioEvalDirection = Context.GetDirection();
		}

		void UpdateDialogFacialAnimation(const FMovieSceneContext& Context, IMovieScenePlayer& Player, const UMovieSceneDialogSection* InDialogSection, const FOrionDialogLine& InDialogLine)
		{
			if (bShow)
			{
				if (!InDialogSection || !InDialogSection->SpeakerParticipantDefinition || !InDialogLine.FacialAnimation || !InDialogSection->HasStartFrame() || !InDialogSection->HasEndFrame())
				{
					StopDialogFacialAnimation();
					return;
				}

				const EMovieScenePlayerStatus::Type Status = Context.GetStatus();
				const bool bIsPlayingLike =
					Status == EMovieScenePlayerStatus::Playing ||
					Status == EMovieScenePlayerStatus::Scrubbing ||
					Status == EMovieScenePlayerStatus::Stepping;

				if (Context.IsSilent() || !bIsPlayingLike)
				{
					StopDialogFacialAnimation();
					return;
				}

				UWorld* World = Player.GetPlaybackContext() ? Player.GetPlaybackContext()->GetWorld() : nullptr;
				if (!World)
				{
					StopDialogFacialAnimation();
					return;
				}

				const FFrameNumber CurrentFrame = Context.GetTime().FloorToFrame();
				if (CurrentFrame < InDialogSection->GetInclusiveStartFrame() || CurrentFrame >= InDialogSection->GetExclusiveEndFrame())
				{
					StopDialogFacialAnimation();
					return;
				}

				

#if WITH_EDITOR
				if (!GEditor || !GEditor->PlayWorld || GEditor->IsSimulateInEditorInProgress())
				{
					UObject* PlaybackContextObject = Player.GetPlaybackContext();
					IDialogContext* DialogContext = PlaybackContextObject ? Cast<IDialogContext>(PlaybackContextObject) : nullptr;
					if (!DialogContext)
					{
						StopDialogFacialAnimation();
						return;
					}
					SpeakerActor = SpeakerActor.IsValid() ? SpeakerActor : DialogContext->GetDialogDefinitionActor(InDialogSection->SpeakerParticipantDefinition);
				}
#endif

				UDialogComponent* DialogComponent = UDialogBuilderFunctionLibrary::GetDialogComponent(World);
				if(DialogComponent)
				{
					SpeakerActor = DialogComponent->GetDialogDefinitionActor(InDialogSection->SpeakerParticipantDefinition);
				}

				if (!SpeakerActor.IsValid())
				{
					StopDialogFacialAnimation();
					return;
				}

				TArray<USkeletalMeshComponent*> SkeletalMeshComponents;
				SpeakerActor->GetComponents(SkeletalMeshComponents);

				USkeletalMeshComponent* TargetMesh = nullptr;
				const USkeleton* TargetSkeleton = InDialogLine.FacialAnimation->GetSkeleton();

				for (USkeletalMeshComponent* SkeletalMeshComponent : SkeletalMeshComponents)
				{
					if (!SkeletalMeshComponent)
					{
						continue;
					}

					if (!TargetMesh)
					{
						TargetMesh = SkeletalMeshComponent;
					}

					const USkeletalMesh* SkeletalMesh = SkeletalMeshComponent->GetSkeletalMeshAsset();
					if (SkeletalMesh && TargetSkeleton && SkeletalMesh->GetSkeleton() == TargetSkeleton)
					{
						TargetMesh = SkeletalMeshComponent;
						break;
					}
				}

				if (!TargetMesh)
				{
					StopDialogFacialAnimation();
					return;
				}

				const FFrameTime RelativeFrameTime = Context.GetTime() - InDialogSection->GetInclusiveStartFrame();

				float StartTimeSeconds = FMath::Max(0.0f, Context.GetFrameRate().AsSeconds(RelativeFrameTime));
				const float AnimationLength = InDialogLine.FacialAnimation->GetPlayLength();
				if (AnimationLength > 0.f)
				{
					StartTimeSeconds = FMath::Min(StartTimeSeconds, FMath::Max(0.0f, AnimationLength - KINDA_SMALL_NUMBER));
				}

				const bool bIsBackwards = Context.GetDirection() == EPlayDirection::Backwards;
				const bool bIsScrubbing =
					Context.GetStatus() == EMovieScenePlayerStatus::Scrubbing ||
					Context.GetStatus() == EMovieScenePlayerStatus::Stepping;

				const bool bDirectionChanged = !bHasLastFacialEvalTime || (LastFacialEvalDirection != Context.GetDirection());

				const bool bNeedsRestart =
					!FacialMeshComponent.IsValid() ||
					FacialMeshComponent.Get() != TargetMesh ||
					!FacialAnimationAsset.IsValid() ||
					FacialAnimationAsset.Get() != InDialogLine.FacialAnimation ||
					Context.HasJumped() ||
					Context.HasLooped() ||
					bDirectionChanged;

				if (bNeedsRestart)
				{
					if (FacialMeshComponent.IsValid() && FacialMeshComponent.Get() != TargetMesh)
					{
						FacialMeshComponent->Stop();
					}

					FacialMeshComponent = TargetMesh;
					FacialAnimationAsset = InDialogLine.FacialAnimation;
					TargetMesh->PlayAnimation(InDialogLine.FacialAnimation, false);
				}

				if (FacialMeshComponent.IsValid())
				{
					FacialMeshComponent->SetPosition(StartTimeSeconds, false);

					const float PlayRate = bIsScrubbing ? 0.0f : (bIsBackwards ? -1.0f : 1.0f);
					FacialMeshComponent->SetPlayRate(PlayRate);

					if (!FacialMeshComponent->IsPlaying())
					{
						FacialMeshComponent->Play(false);
					}
				}

				bHasLastFacialEvalTime = true;
				LastFacialEvalTime = Context.GetTime();
				LastFacialEvalDirection = Context.GetDirection();
			}
			else
			{
				StopDialogFacialAnimation();
			}
		}

		bool HasAnythingToDo() const
		{
			return bNeedExecute;
		}

		void SetInfo(bool bInShow, const FOrionDialogLine& InDialogLine)
		{
			bShow = bInShow;
			DialogLine = InDialogLine;
			bNeedExecute = true;
		}

		void NotifyDialogInterface(UObject* TargetObject) const
		{
			if (!IsValid(TargetObject))
			{
				return;
			}

			UClass* TargetClass = TargetObject->GetClass();
			if (!TargetClass || !TargetClass->ImplementsInterface(UOrionDialogInterface::StaticClass()))
			{
				return;
			}

			if (bShow)
			{
				IOrionDialogInterface::Execute_OnDialogLineStarted(TargetObject, DialogLine);
			}
			else
			{
				IOrionDialogInterface::Execute_OnDialogLineEnded(TargetObject, DialogLine);
			}
		}

		void Apply(IMovieScenePlayer& Player)
		{
			bNeedExecute = false;

			UWorld* World = Player.GetPlaybackContext() ? Player.GetPlaybackContext()->GetWorld() : nullptr;
			if (!World)
			{
				return;
			}

			UDialogBuilderSetting* DialogBuilderSetting = GetMutableDefault<UDialogBuilderSetting>();
			TSubclassOf<UUserWidget> DialogWidgetClass = DialogBuilderSetting->DefaultDialogWidget.Get();


#if WITH_EDITOR
			if (GIsEditor && !World->IsGameWorld())
			{
				if (bShow)
				{
					if (DialogWidget.IsValid() && !DialogWidget->GetCachedWidget().IsValid())
					{
						DialogWidget->RemoveFromParent();
						
						DialogWidget.Reset();
					}
					if (!DialogWidget.IsValid())
					{
						DialogWidget = CreateWidget<UUserWidget>(World, DialogWidgetClass);
						if (DialogWidget.IsValid())
						{
							DialogWidget->GetRootWidget()->SetVisibility(ESlateVisibility::HitTestInvisible);

							if (UObject* PlaybackContextObject = Player.GetPlaybackContext())
							{
								if (IDialogContext* DialogContext = Cast<IDialogContext>(PlaybackContextObject))
								{
									DialogContext->AddDialogToViewport(DialogWidget.Get());
								}
							}
						}

						FWorldDelegates::OnWorldCleanup.AddLambda(
							[this](UWorld*, bool, bool)
							{
								if (DialogWidget.IsValid())
								{
									DialogWidget->RemoveFromParent();
								}
								DialogWidget.Reset();
								
							});
					}

					NotifyDialogInterface(DialogWidget.Get());
				}
				else
				{
					NotifyDialogInterface(DialogWidget.Get());
				}

				return;
			}
#endif

			if (!GEngine || !GEngine->GameViewport)
			{
				return;
			}

			UGameViewportClient* GameViewport = GEngine->GameViewport;

			UDialogComponent* DialogComponent = UDialogBuilderFunctionLibrary::GetDialogComponent(World);

			if (!DialogComponent) return;
			if (bShow && !DialogLine.Line.IsEmpty())
			{
				DialogComponent->OnDialogLineUpdated.Broadcast(DialogLine);
			}
			else
			{
				
			}
		}

		bool bNeedExecute;
		bool bShow;

		FOrionDialogLine DialogLine;
		FGameplayTag SpeakerTag;
		FGameplayTag ListenerTag;

		TWeakObjectPtr<UUserWidget> DialogWidget;
		TWeakObjectPtr<AActor> SpeakerActor;


		TWeakObjectPtr<UAudioComponent> DialogAudioComponent;

		TWeakObjectPtr<USkeletalMeshComponent> FacialMeshComponent;
		TWeakObjectPtr<UAnimSequence> FacialAnimationAsset;

		FFrameTime LastAudioEvalTime = FFrameTime(TNumericLimits<int32>::Lowest());
		EPlayDirection LastAudioEvalDirection = EPlayDirection::Forwards;
		bool bHasLastAudioEvalTime = false;

		FFrameTime LastFacialEvalTime = FFrameTime(TNumericLimits<int32>::Lowest());
		EPlayDirection LastFacialEvalDirection = EPlayDirection::Forwards;
		bool bHasLastFacialEvalTime = false;
	};

	struct FDialogExecutionToken : IMovieSceneSharedExecutionToken
	{
		virtual void Execute(FPersistentEvaluationData& PersistentData, IMovieScenePlayer& Player) override
		{
			FDialogSharedTrackData* TrackData = PersistentData.Find<FDialogSharedTrackData>(GetSharedDataKey());
			if (TrackData)
			{
				TrackData->Apply(Player);
			}
		}
	};

	struct FDialogAudioExecutionToken : IMovieSceneExecutionToken
	{
		FDialogAudioExecutionToken(UMovieSceneDialogSection* InDialogSection, const FOrionDialogLine& InDialogLine)
			: DialogSection(InDialogSection)
			, DialogLine(InDialogLine)
		{
		}

		virtual void Execute(const FMovieSceneContext& Context, const FMovieSceneEvaluationOperand&, FPersistentEvaluationData& PersistentData, IMovieScenePlayer& Player) override
		{
			FDialogSharedTrackData& TrackData = PersistentData.GetOrAdd<FDialogSharedTrackData>(GetSharedDataKey());
			TrackData.UpdateDialogAudio(Context, Player, DialogSection, DialogLine);
		}

		UMovieSceneDialogSection* DialogSection;
		FOrionDialogLine DialogLine;
	};

	struct FDialogFacialAnimationExecutionToken : IMovieSceneExecutionToken
	{
		FDialogFacialAnimationExecutionToken(UMovieSceneDialogSection* InDialogSection, const FOrionDialogLine& InDialogLine)
			: DialogSection(InDialogSection)
			, DialogLine(InDialogLine)
		{
		}

		virtual void Execute(const FMovieSceneContext& Context, const FMovieSceneEvaluationOperand&, FPersistentEvaluationData& PersistentData, IMovieScenePlayer& Player) override
		{
			FDialogSharedTrackData& TrackData = PersistentData.GetOrAdd<FDialogSharedTrackData>(GetSharedDataKey());
			TrackData.UpdateDialogFacialAnimation(Context, Player, DialogSection, DialogLine);
		}

		UMovieSceneDialogSection* DialogSection;
		FOrionDialogLine DialogLine;
	};
}

FMovieSceneDialogSectionTemplate::FMovieSceneDialogSectionTemplate(const UMovieSceneDialogSection& InSection)
	: DialogLine(InSection.DialogLine)
{
	DialogSection = const_cast<UMovieSceneDialogSection*>(& InSection);
}

void FMovieSceneDialogSectionTemplate::Evaluate(
	const FMovieSceneEvaluationOperand& Operand,
	const FMovieSceneContext& Context,
	const FPersistentEvaluationData& PersistentData,
	FMovieSceneExecutionTokens& ExecutionTokens) const
{
	using namespace OrionDialogTemplate;

	DialogSection->RefreshDialogContent();

	ExecutionTokens.Add(FDialogAudioExecutionToken(DialogSection, DialogSection->DialogLine));
	ExecutionTokens.Add(FDialogFacialAnimationExecutionToken(DialogSection, DialogSection->DialogLine));

	const FDialogSharedTrackData* TrackData = PersistentData.Find<FDialogSharedTrackData>(GetSharedDataKey());
	if (TrackData)
	{
		const FMovieSceneSharedDataId SharedId = GetSharedDataKey().UniqueId;
		if (!ExecutionTokens.FindShared(SharedId))
		{
			ExecutionTokens.AddShared(SharedId, FDialogExecutionToken());
		}
	}
}

void FMovieSceneDialogSectionTemplate::Setup(FPersistentEvaluationData& PersistentData, IMovieScenePlayer& Player) const
{
	using namespace OrionDialogTemplate;

	FDialogSharedTrackData& TrackData = PersistentData.GetOrAdd<FDialogSharedTrackData>(GetSharedDataKey());
	TrackData.SetInfo(true, DialogSection->DialogLine);
}

void FMovieSceneDialogSectionTemplate::TearDown(FPersistentEvaluationData& PersistentData, IMovieScenePlayer& Player) const
{
	using namespace OrionDialogTemplate;

	FDialogSharedTrackData& TrackData = PersistentData.GetOrAdd<FDialogSharedTrackData>(GetSharedDataKey());
	TrackData.StopDialogAudio();
	TrackData.SetInfo(false, DialogSection->DialogLine);
	TrackData.Apply(Player);
}