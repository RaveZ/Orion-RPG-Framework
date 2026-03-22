// Copyright 2025 Ivan Chandra. All Rights Reserved.

#include "MovieSceneDialogTemplate.h"
#include "MovieSceneDialogSection.h"
#include "OrionDialogInterface.h"

#include "Engine/Engine.h"
#include "DialogBuilderSetting.h"
#include "Engine/GameViewportClient.h"
#include "Engine/World.h"
#include "Evaluation/MovieSceneExecutionTokens.h"
#include "IMovieScenePlayer.h"
#include "Widgets/SWidget.h"
#include "Widgets/SViewport.h"
#include "CommonActivatableWidget.h"
#include "Blueprint/UserWidget.h"

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

		bool HasAnythingToDo() const
		{
			return bNeedExecute;
		}

		void SetInfo(bool bInShow, const FDialogLineData& InDialogLineData, const FGameplayTag& InSpeakerTag, const FGameplayTag& InListenerTag)
		{
			bShow = bInShow;
			DialogLineData = InDialogLineData;
			SpeakerTag = InSpeakerTag;
			ListenerTag = InListenerTag;
			bNeedExecute = true;
		}

		void NotifyDialogInterface(UObject* TargetObject) const
		{
			if (!TargetObject || !TargetObject->GetClass()->ImplementsInterface(UOrionDialogInterface::StaticClass()))
			{
				return;
			}
			 
			if (bShow)
			{
				IOrionDialogInterface::Execute_OnDialogLineStarted(TargetObject, DialogLineData, SpeakerTag, ListenerTag);
			}
			else
			{
				IOrionDialogInterface::Execute_OnDialogLineEnded(TargetObject, DialogLineData, SpeakerTag, ListenerTag);
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
							DialogWidgetSlateWidget = DialogWidget->TakeWidget();

							if (UObject* PlaybackContextObject = Player.GetPlaybackContext())
							{
								if (IDialogContext* DialogContext = Cast<IDialogContext>(PlaybackContextObject))
								{
									DialogContext->AddDialogToViewport(DialogWidgetSlateWidget.ToSharedRef());
								}
							}
						}
					}

					NotifyDialogInterface(DialogWidget);
				}
				else
				{
					NotifyDialogInterface(DialogWidget);

					if (UObject* PlaybackContextObject = Player.GetPlaybackContext())
					{
						if (IDialogContext* DialogContext = Cast<IDialogContext>(PlaybackContextObject))
						{
							DialogContext->RemoveDialogFromViewport();
						}
					}

					EditorViewportContainer.Reset();
					DialogWidgetSlateWidget.Reset();
					DialogWidget = nullptr;
				}

				return;
			}
#endif

			if (!GEngine || !GEngine->GameViewport)
			{
				return;
			}

			UGameViewportClient* GameViewport = GEngine->GameViewport;

			if (bShow)
			{
				if (!DialogWidget)
				{
					DialogWidget = CreateWidget<UUserWidget>(World, UUserWidget::StaticClass());
					if (DialogWidget)
					{
						DialogWidgetSlateWidget = DialogWidget->TakeWidget();
						GameViewport->AddViewportWidgetContent(DialogWidgetSlateWidget.ToSharedRef());
					}
				}

				NotifyDialogInterface(DialogWidget);
			}
			else
			{
				NotifyDialogInterface(DialogWidget);

				if (DialogWidgetSlateWidget.IsValid())
				{
					GameViewport->RemoveViewportWidgetContent(DialogWidgetSlateWidget.ToSharedRef());
				}

				DialogWidgetSlateWidget.Reset();
				DialogWidget = nullptr;
			}
		}

		bool bNeedExecute;
		bool bShow;

		FDialogLineData DialogLineData;
		FGameplayTag SpeakerTag;
		FGameplayTag ListenerTag;

		UPROPERTY()
		TObjectPtr<UUserWidget> DialogWidget = nullptr;

		TSharedPtr<SWidget> DialogWidgetSlateWidget;

#if WITH_EDITOR
		TWeakPtr<SWidget> EditorViewportContainer;
#endif
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
}

FMovieSceneDialogSectionTemplate::FMovieSceneDialogSectionTemplate(const UMovieSceneDialogSection& InSection)
	: SpeakerParticipantTag(InSection.SpeakerParticipantTag)
	, ListenerParticipantTag(InSection.ListenerParticipantTag)
	, DialogLineData(InSection.DialogLineData)
{
}

void FMovieSceneDialogSectionTemplate::Evaluate(
	const FMovieSceneEvaluationOperand& Operand,
	const FMovieSceneContext& Context,
	const FPersistentEvaluationData& PersistentData,
	FMovieSceneExecutionTokens& ExecutionTokens) const
{
	using namespace OrionDialogTemplate;

	const FDialogSharedTrackData* TrackData = PersistentData.Find<FDialogSharedTrackData>(GetSharedDataKey());
	if (TrackData && TrackData->HasAnythingToDo())
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
	TrackData.SetInfo(true, DialogLineData, SpeakerParticipantTag, ListenerParticipantTag);
}

void FMovieSceneDialogSectionTemplate::TearDown(FPersistentEvaluationData& PersistentData, IMovieScenePlayer& Player) const
{
	using namespace OrionDialogTemplate;

	FDialogSharedTrackData& TrackData = PersistentData.GetOrAdd<FDialogSharedTrackData>(GetSharedDataKey());
	TrackData.SetInfo(false, DialogLineData, SpeakerParticipantTag, ListenerParticipantTag);
	TrackData.Apply(Player);
}