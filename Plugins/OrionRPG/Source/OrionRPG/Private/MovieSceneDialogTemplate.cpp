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

		void SetInfo(bool bInShow, const FOrionDialogLine& InDialogLine)
		{
			bShow = bInShow;
			DialogLine = InDialogLine;
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
							DialogWidgetSlateWidget = DialogWidget->TakeWidget();

							if (UObject* PlaybackContextObject = Player.GetPlaybackContext())
							{
								if (IDialogContext* DialogContext = Cast<IDialogContext>(PlaybackContextObject))
								{
									DialogContext->AddDialogToViewport(DialogWidgetSlateWidget.ToSharedRef());
								}
							}
						}

						const TWeakObjectPtr<UUserWidget> DialogWidgetWeak = DialogWidget;
						FWorldDelegates::OnWorldCleanup.AddLambda(
							[this, DialogWidgetWeak](UWorld*, bool, bool)
							{
								if (DialogWidgetWeak.IsValid())
								{
									DialogWidgetWeak->RemoveFromParent();
								}

								DialogWidgetSlateWidget.Reset();
								DialogWidget = nullptr;
							});
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

			UDialogComponent* DialogComponent = UDialogBuilderFunctionLibrary::GetDialogComponent(World);

			if (!DialogComponent) return;
			if (bShow)
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

		TSharedPtr<SWidget> DialogWidgetSlateWidget;

#if WITH_EDITOR
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
	TrackData.SetInfo(false, DialogSection->DialogLine);
	TrackData.Apply(Player);
}