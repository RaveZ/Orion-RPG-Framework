// Copyright 2025 Ivan Chandra. All Rights Reserved.

#include "MovieSceneDialogTemplate.h"
#include "MovieSceneDialogSection.h"
#include "UI/OrionDialogWidget.h"

#include "Engine/Engine.h"
#include "Engine/GameViewportClient.h"
#include "Engine/World.h"
#include "Evaluation/MovieSceneExecutionTokens.h"
#include "IMovieScenePlayer.h"
#include "Widgets/SWidget.h"
#include "Widgets/SViewport.h"

#if WITH_EDITOR
#include "LevelEditor.h"
//#include "ILevelViewport.h"
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

		void Apply(IMovieScenePlayer& Player)
		{
			bNeedExecute = false;

			UWorld* World = Player.GetPlaybackContext() ? Player.GetPlaybackContext()->GetWorld() : nullptr;
			if (!World)
			{
				return;
			}

#if WITH_EDITOR
			// Editor non-PIE path
			if (GIsEditor && !World->IsGameWorld())
			{
				if (bShow)
				{
					if (!DialogWidget)
					{
						DialogWidget = CreateWidget<UOrionDialogWidget>(World, UOrionDialogWidget::StaticClass());
						if (DialogWidget)
						{
							DialogWidgetSlateWidget = DialogWidget->TakeWidget();

							if (FSceneViewport* SceneViewport = static_cast<FSceneViewport*>(GEditor ? GEditor->GetActiveViewport() : nullptr))
							{
								if (SceneViewport->GetViewportWidget().IsValid())
								{
									EditorViewportContainer = SceneViewport->GetViewportWidget().Pin()->GetContent();
									if (TSharedPtr<SOverlay> Overlay = StaticCastSharedPtr<SOverlay>(EditorViewportContainer.Pin()))
									{
										Overlay->AddSlot()
										[
											DialogWidgetSlateWidget.ToSharedRef()
										];
									}
								}
							}
						}
					}

					if (DialogWidget)
					{
						DialogWidget->SetDialogData(
							FText::FromString(SpeakerTag.ToString()),
							DialogLineData.Line,
							SpeakerTag,
							ListenerTag);
					}
				}
				else
				{
					if (DialogWidgetSlateWidget.IsValid())
					{
						if (TSharedPtr<SOverlay> Overlay = StaticCastSharedPtr<SOverlay>(EditorViewportContainer.Pin()))
						{
							Overlay->RemoveSlot(DialogWidgetSlateWidget.ToSharedRef());
						}
					}

					EditorViewportContainer.Reset();
					DialogWidgetSlateWidget.Reset();
					DialogWidget = nullptr;
				}

				return;
			}
#endif

			// Runtime / PIE path
			if (!GEngine || !GEngine->GameViewport)
			{
				return;
			}

			UGameViewportClient* GameViewport = GEngine->GameViewport;

			if (bShow)
			{
				if (!DialogWidget)
				{
					DialogWidget = CreateWidget<UOrionDialogWidget>(World, UOrionDialogWidget::StaticClass());
					if (DialogWidget)
					{
						DialogWidgetSlateWidget = DialogWidget->TakeWidget();
						GameViewport->AddViewportWidgetContent(DialogWidgetSlateWidget.ToSharedRef());
					}
				}

				if (DialogWidget)
				{
					DialogWidget->SetDialogData(
						FText::FromString(SpeakerTag.ToString()),
						DialogLineData.Line,
						SpeakerTag,
						ListenerTag);
				}
			}
			else
			{
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
		TObjectPtr<UOrionDialogWidget> DialogWidget = nullptr;

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