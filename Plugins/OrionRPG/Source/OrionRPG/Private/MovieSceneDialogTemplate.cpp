// Copyright 2025 Ivan Chandra. All Rights Reserved.

#include "MovieSceneDialogTemplate.h"
#include "MovieSceneDialogSection.h"
#include "OrionDialogInterface.h"

#include "Engine/Engine.h"
#include "DialogBuilderSetting.h"
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
			if (DialogWidget)
			{
				DialogWidget->RemoveFromParent();
			}

			StopDialogAudio();

			DialogWidget = nullptr;
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
					if (!DialogWidget)
					{

						DialogWidget = CreateWidget<UUserWidget>(World, DialogWidgetClass);
						if (DialogWidget)
						{
							DialogWidget->GetRootWidget()->SetVisibility(ESlateVisibility::HitTestInvisible);

							if (UObject* PlaybackContextObject = Player.GetPlaybackContext())
							{
								if (IDialogContext* DialogContext = Cast<IDialogContext>(PlaybackContextObject))
								{
									DialogContext->AddDialogToViewport(DialogWidget);
								}
							}
						}

						const TWeakObjectPtr<UUserWidget> DialogWidgetWeak = DialogWidget;
						FWorldDelegates::OnWorldCleanup.AddLambda(
							[this, DialogWidgetWeak](UWorld*, bool, bool)
							{
								/*if (DialogWidgetWeak.IsValid())
								{
									DialogWidgetWeak->RemoveFromParent();
								}
								DialogWidget = nullptr;*/
							});
					}

					NotifyDialogInterface(DialogWidget);
				}
				else
				{
					NotifyDialogInterface(DialogWidget);

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

		UPROPERTY()
		TObjectPtr<UUserWidget> DialogWidget = nullptr;

		TWeakObjectPtr<UAudioComponent> DialogAudioComponent;

		FFrameTime LastAudioEvalTime = FFrameTime(TNumericLimits<int32>::Lowest());
		EPlayDirection LastAudioEvalDirection = EPlayDirection::Forwards;
		bool bHasLastAudioEvalTime = false;
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