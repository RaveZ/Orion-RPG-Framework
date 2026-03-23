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

	FFrameNumber StartTime = MovieScene->GetPlaybackRange().GetLowerBoundValue();
	FFrameNumber LastSectionEndTime = StartTime;

	for (UMovieSceneSection* Section : DialogTrack->GetAllSections())
	{
		TRange<FFrameNumber> SectionRange = Section->GetRange();
		LastSectionEndTime = SectionRange.GetUpperBoundValue() > LastSectionEndTime ? SectionRange.GetUpperBoundValue() : LastSectionEndTime;
	}

	UMovieSceneDialogSection* DialogSection = CastChecked<UMovieSceneDialogSection>(DialogTrack->CreateNewSection());

	StartTime = LastSectionEndTime;
	const FFrameRate TickResolution = MovieScene->GetTickResolution();
	const FFrameNumber SectionLength = (0.5 * TickResolution).FloorToFrame();

	FFrameNumber EndTime = StartTime + SectionLength;
	if (EndTime < StartTime)
	{
		const FFrameNumber Temp = StartTime;
		StartTime = EndTime;
		EndTime = Temp;
	}

	DialogSection->SetRange(TRange<FFrameNumber>(StartTime, TRangeBound<FFrameNumber>::Exclusive(EndTime)));
	DialogSection->SetRowIndex(0);

	DialogTrack->AddSection(*DialogSection);
}

#undef LOCTEXT_NAMESPACE