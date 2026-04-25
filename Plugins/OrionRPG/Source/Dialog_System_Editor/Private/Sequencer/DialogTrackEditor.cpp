// Copyright 2025 Ivan Chandra. All Rights Reserved.

#include "Sequencer/DialogTrackEditor.h"
#include "Sequencer/DialogSection.h"
#include "MovieSceneDialogSection.h"
#include "MovieSceneDialogTrack.h"
#include "DialogSequence.h"

#include "Framework/MultiBox/MultiBoxBuilder.h"
#include "Modules/ModuleManager.h"
#include "PropertyEditorModule.h"
#include "IDetailsView.h"
#include "IDetailCustomization.h"
#include "DetailLayoutBuilder.h"
#include "SequencerUtilities.h"
#include "ScopedTransaction.h"

#define LOCTEXT_NAMESPACE "DialogTrackEditor"

TSharedRef<ISequencerTrackEditor> FDialogTrackEditor::CreateTrackEditor(TSharedRef<ISequencer> InSequencer)
{
	return MakeShared<FDialogTrackEditor>(InSequencer);
}

FDialogTrackEditor::FDialogTrackEditor(TSharedRef<ISequencer> InSequencer)
	: FMovieSceneTrackEditor(InSequencer)
{
}

bool FDialogTrackEditor::SupportsType(TSubclassOf<UMovieSceneTrack> Type) const
{
	return Type == UMovieSceneDialogTrack::StaticClass();
}

bool FDialogTrackEditor::SupportsSequence(UMovieSceneSequence* InSequence) const
{
	return Cast<UDialogSequence>(InSequence) != nullptr;
}

TSharedRef<ISequencerSection> FDialogTrackEditor::MakeSectionInterface(UMovieSceneSection& SectionObject, UMovieSceneTrack& Track, FGuid ObjectBinding)
{
	UMovieSceneDialogSection* DialogSection = Cast<UMovieSceneDialogSection>(&SectionObject);
	check(SupportsType(SectionObject.GetOuter()->GetClass()) && DialogSection != nullptr);

	return MakeShared<FDialogSection>(*DialogSection);
}

void FDialogTrackEditor::BuildAddTrackMenu(FMenuBuilder& MenuBuilder)
{
	/*UMovieSceneSequence* RootSequence = GetSequencer()->GetRootMovieSceneSequence();
	if (!Cast<UDialogSequence>(RootSequence))
	{
		return;
	}

	MenuBuilder.AddMenuEntry(
		LOCTEXT("AddDialogTrack", "Dialog"),
		LOCTEXT("AddDialogTrackTooltip", "Add a dialog track."),
		FSlateIcon(),
		FUIAction(FExecuteAction::CreateSP(this, &FDialogTrackEditor::OnAddTrack)));*/
}

TSharedPtr<SWidget> FDialogTrackEditor::BuildOutlinerEditWidget(const FGuid& ObjectBinding, UMovieSceneTrack* Track, const FBuildEditWidgetParams& Params)
{
	if (!Track || !SupportsType(Track->GetClass()))
	{
		return nullptr;
	}

	return FSequencerUtilities::MakeAddButton(
		LOCTEXT("AddDialogSectionButton", "Add Dialog"),
		FOnGetContent::CreateSP(this, &FDialogTrackEditor::BuildAddDialogSectionMenu, Track),
		Params.NodeIsHovered,
		GetSequencer());
}

void FDialogTrackEditor::BuildTrackContextMenu(FMenuBuilder& MenuBuilder, UMovieSceneTrack* Track)
{
	UMovieSceneDialogTrack* DialogTrack = Cast<UMovieSceneDialogTrack>(Track);
	if (!DialogTrack)
	{
		return;
	}

	class FDialogTrackCustomization : public IDetailCustomization
	{
	public:
		virtual void CustomizeDetails(IDetailLayoutBuilder& DetailBuilder) override
		{
			DetailBuilder.HideCategory("Track");
			DetailBuilder.HideCategory("General");
		}
	};

	auto SubMenuDelegate = [DialogTrack](FMenuBuilder& InMenuBuilder)
	{
		FDetailsViewArgs DetailsViewArgs;
		DetailsViewArgs.bAllowSearch = false;
		DetailsViewArgs.bCustomFilterAreaLocation = true;
		DetailsViewArgs.bCustomNameAreaLocation = true;
		DetailsViewArgs.bHideSelectionTip = true;
		DetailsViewArgs.bLockable = false;
		DetailsViewArgs.bSearchInitialKeyFocus = true;
		DetailsViewArgs.bUpdatesFromSelection = false;
		DetailsViewArgs.bShowOptions = false;
		DetailsViewArgs.bShowModifiedPropertiesOption = false;

		TArray<TWeakObjectPtr<UObject>> Objects;
		Objects.Add(DialogTrack);

		TSharedRef<IDetailsView> DetailsView = FModuleManager::GetModuleChecked<FPropertyEditorModule>("PropertyEditor").CreateDetailView(DetailsViewArgs);
		const FOnGetDetailCustomizationInstance CreateInstance = FOnGetDetailCustomizationInstance::CreateLambda([] { return MakeShared<FDialogTrackCustomization>(); });
		DetailsView->RegisterInstancedCustomPropertyLayout(UMovieSceneDialogTrack::StaticClass(), CreateInstance);
		DetailsView->SetObjects(Objects);

		InMenuBuilder.AddWidget(DetailsView, FText::GetEmpty(), true);
	};

	MenuBuilder.AddSubMenu(
		LOCTEXT("DialogTrackProperties", "Properties"),
		LOCTEXT("DialogTrackPropertiesTooltip", "Dialog track properties."),
		FNewMenuDelegate::CreateLambda(SubMenuDelegate),
		false);
}

void FDialogTrackEditor::OnAddTrack()
{
	UMovieScene* FocusedMovieScene = GetFocusedMovieScene();
	if (!FocusedMovieScene)
	{
		return;
	}

	const FScopedTransaction Transaction(LOCTEXT("AddDialogTrack_Transaction", "Add Dialog Track"));
	FocusedMovieScene->Modify();

	UMovieSceneDialogTrack* NewTrack = FocusedMovieScene->AddTrack<UMovieSceneDialogTrack>();
	checkf(NewTrack != nullptr, TEXT("Failed to create new dialog track."));

	GetSequencer()->NotifyMovieSceneDataChanged(EMovieSceneDataChangeType::MovieSceneStructureItemAdded);
}

TSharedRef<SWidget> FDialogTrackEditor::BuildAddDialogSectionMenu(UMovieSceneTrack* DialogTrack)
{
	FMenuBuilder MenuBuilder(true, nullptr);

	MenuBuilder.AddMenuEntry(
		LOCTEXT("AddDialogSection", "Dialog"),
		LOCTEXT("AddDialogSectionTooltip", "Add a dialog section."),
		FSlateIcon(),
		FUIAction(FExecuteAction::CreateSP(this, &FDialogTrackEditor::OnAddNewSection, DialogTrack)));

	return MenuBuilder.MakeWidget();
}

void FDialogTrackEditor::OnAddNewSection(UMovieSceneTrack* DialogTrack)
{
	UMovieScene* FocusedMovieScene = GetFocusedMovieScene();
	if (!FocusedMovieScene || !DialogTrack)
	{
		return;
	}

	AddNewSection(FocusedMovieScene, DialogTrack);
	GetSequencer()->NotifyMovieSceneDataChanged(EMovieSceneDataChangeType::MovieSceneStructureItemAdded);
}

void FDialogTrackEditor::AddNewSection(UMovieScene* MovieScene, UMovieSceneTrack* DialogTrack)
{
	if (!MovieScene || !DialogTrack)
	{
		return;
	}

	const FScopedTransaction Transaction(LOCTEXT("AddDialogSection_Transaction", "Add Dialog Section"));
	DialogTrack->Modify();

	const FFrameRate TickResolution = MovieScene->GetTickResolution();
	const FFrameNumber SectionLength = TickResolution.AsFrameNumber(3.0);

	FFrameNumber LastSectionEndTime = MovieScene->GetPlaybackRange().GetLowerBoundValue();
	for (UMovieSceneSection* Section : DialogTrack->GetAllSections())
	{
		if (!Section)
		{
			continue;
		}

		const TRange<FFrameNumber> SectionRange = Section->GetRange();
		if (SectionRange.HasUpperBound())
		{
			LastSectionEndTime = SectionRange.GetUpperBoundValue() > LastSectionEndTime
				? SectionRange.GetUpperBoundValue()
				: LastSectionEndTime;
		}
	}

	const FFrameNumber CurrentFrame = GetSequencer().IsValid()
		? GetSequencer()->GetLocalTime().Time.FloorToFrame()
		: LastSectionEndTime;

	FFrameNumber StartTime = CurrentFrame;
	FFrameNumber EndTime = StartTime + SectionLength;

	// Try to keep the section at the current frame, but shorten it if it overlaps.
	for (UMovieSceneSection* Section : DialogTrack->GetAllSections())
	{
		if (!Section)
		{
			continue;
		}

		const TRange<FFrameNumber> SectionRange = Section->GetRange();
		const TRange<FFrameNumber> CandidateRange(StartTime, TRangeBound<FFrameNumber>::Exclusive(EndTime));

		if (SectionRange.Overlaps(CandidateRange))
		{
			if (SectionRange.HasLowerBound())
			{
				EndTime = FMath::Min(EndTime, SectionRange.GetLowerBoundValue());
			}
		}
	}

	// If the current frame is already too close/inside another section, fall back to the end.
	if (EndTime <= StartTime)
	{
		StartTime = LastSectionEndTime;
		EndTime = StartTime + SectionLength;
	}

	UMovieSceneDialogSection* DialogSection = CastChecked<UMovieSceneDialogSection>(DialogTrack->CreateNewSection());
	DialogSection->SetRange(TRange<FFrameNumber>(StartTime, TRangeBound<FFrameNumber>::Exclusive(EndTime)));
	DialogSection->SetRowIndex(0);

	DialogTrack->AddSection(*DialogSection);
}

#undef LOCTEXT_NAMESPACE