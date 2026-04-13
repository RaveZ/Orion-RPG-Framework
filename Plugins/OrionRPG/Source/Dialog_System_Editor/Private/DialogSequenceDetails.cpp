// Copyright 2025 Ivan Chandra. All Rights Reserved.

#include "DialogSequenceDetails.h"

#include "DialogBuilderGraph.h"
#include "DialogDefinition.h"
#include "DialogStage.h"
#include "DetailCategoryBuilder.h"
#include "DetailLayoutBuilder.h"
#include "DetailWidgetRow.h"
#include "Framework/Commands/UIAction.h"
#include "Framework/MultiBox/MultiBoxBuilder.h"
#include "Internationalization/Text.h"
#include "Layout/Margin.h"
#include "MovieSceneDialogSection.h"
#include "PropertyHandle.h"
#include "Templates/Casts.h"
#include "Widgets/Input/SComboButton.h"
#include "Widgets/SBoxPanel.h"
#include "Widgets/Text/STextBlock.h"

#define LOCTEXT_NAMESPACE "DialogSequenceDetails"

TSharedRef<IDetailCustomization> FDialogSequenceDetails::MakeInstance()
{
	return MakeShareable(new FDialogSequenceDetails);
}

void FDialogSequenceDetails::CustomizeDetails(IDetailLayoutBuilder& DetailBuilder)
{
	DialogSequenceSlot.Reset();
	MovieSceneDialogSection.Reset();

	ActorTagPropertyHandle.Reset();
	DialogLinePropertyHandle.Reset();
	SpeakerTagPropertyHandle.Reset();
	ListenerTagPropertyHandle.Reset();

	const TArray<TWeakObjectPtr<UObject>>& SelectedObjects = DetailBuilder.GetSelectedObjects();
	for (const TWeakObjectPtr<UObject>& CurrentObject : SelectedObjects)
	{
		if (!CurrentObject.IsValid())
		{
			continue;
		}

		if (!DialogSequenceSlot.IsValid())
		{
			if (UDialogSequenceSlot* SlotPtr = Cast<UDialogSequenceSlot>(CurrentObject.Get()))
			{
				DialogSequenceSlot = SlotPtr;
			}
		}

		if (!MovieSceneDialogSection.IsValid())
		{
			if (UMovieSceneDialogSection* SectionPtr = Cast<UMovieSceneDialogSection>(CurrentObject.Get()))
			{
				MovieSceneDialogSection = SectionPtr;
			}
		}
	}

	if (SelectedObjects.Num() != 1)
	{
		return;
	}

	// DialogSequenceSlot customization
	if (DialogSequenceSlot.IsValid())
	{
		IDetailCategoryBuilder& DetailCategory = DetailBuilder.EditCategory("Detail", LOCTEXT("DetailCategoryTitle", "Detail"));

		ActorTagPropertyHandle = DetailBuilder.GetProperty(GET_MEMBER_NAME_CHECKED(UDialogSequenceSlot, ActorTag));
		if (ActorTagPropertyHandle.IsValid())
		{
			ActorTagPropertyHandle->MarkHiddenByCustomization();
		}

		DetailCategory.AddCustomRow(LOCTEXT("ParticipantToBindLabel", "Participant To Bind"))
			.NameContent()
			[
				SNew(STextBlock)
					.Text(LOCTEXT("ParticipantToBindText", "Participant To Bind"))
					.Font(IDetailLayoutBuilder::GetDetailFont())
					.ToolTipText(LOCTEXT("ParticipantToBindTooltip", "Pick a participant definition from the owning dialog graph. This sets ActorTag on the slot."))
			]
			.ValueContent()
			.MaxDesiredWidth(300.0f)
			[
				GetWidgetForSlotParticipantPicker(
					TAttribute<FText>::Create(TAttribute<FText>::FGetter::CreateSP(this, &FDialogSequenceDetails::GetSlotParticipantDisplayText)),
					FOnGetContent::CreateSP(this, &FDialogSequenceDetails::OnGetSlotParticipantListMenu))
			];

		DetailCategory.AddProperty(GET_MEMBER_NAME_CHECKED(UDialogSequenceSlot, DialogDefinition)).DisplayName(LOCTEXT("DialogDefinitionLabel", "Dialog Definition"));
		DetailCategory.AddProperty(GET_MEMBER_NAME_CHECKED(UDialogSequenceSlot, SlotPlacement)).DisplayName(LOCTEXT("SlotPlacementLabel", "Slot Placement"));
		return;
	}

	// MovieSceneDialogSection customization
	if (MovieSceneDialogSection.IsValid())
	{
		IDetailCategoryBuilder& SectionCategory = DetailBuilder.EditCategory("Section", LOCTEXT("SectionCategoryTitle", "Section"));

		DialogLinePropertyHandle = DetailBuilder.GetProperty(GET_MEMBER_NAME_CHECKED(UMovieSceneDialogSection, DialogLine));
		if (DialogLinePropertyHandle.IsValid())
		{
			DialogLinePropertyHandle->MarkHiddenByCustomization();

			SpeakerTagPropertyHandle = DialogLinePropertyHandle->GetChildHandle(GET_MEMBER_NAME_CHECKED(FOrionDialogLine, SpeakerTag));
			ListenerTagPropertyHandle = DialogLinePropertyHandle->GetChildHandle(GET_MEMBER_NAME_CHECKED(FOrionDialogLine, ListenerTag));
		}

		SectionCategory.AddCustomRow(LOCTEXT("SpeakerParticipantRowLabel", "Speaker Participant"))
			.NameContent()
			[
				SNew(STextBlock)
					.Text(LOCTEXT("SpeakerParticipantText", "Speaker"))
					.Font(IDetailLayoutBuilder::GetDetailFont())
					.ToolTipText(LOCTEXT("SpeakerParticipantTooltip", "Pick participant for SpeakerTag."))
			]
			.ValueContent()
			.MaxDesiredWidth(300.0f)
			[
				GetWidgetForSpeakerParticipantPicker(
					TAttribute<FText>::Create(TAttribute<FText>::FGetter::CreateSP(this, &FDialogSequenceDetails::GetSectionSpeakerDisplayText)),
					FOnGetContent::CreateSP(this, &FDialogSequenceDetails::OnGetSpeakerParticipantListMenu))
			];

		SectionCategory.AddCustomRow(LOCTEXT("ListenerParticipantRowLabel", "Listener Participant"))
			.NameContent()
			[
				SNew(STextBlock)
					.Text(LOCTEXT("ListenerParticipantText", "Listener"))
					.Font(IDetailLayoutBuilder::GetDetailFont())
					.ToolTipText(LOCTEXT("ListenerParticipantTooltip", "Pick participant for ListenerTag."))
			]
			.ValueContent()
			.MaxDesiredWidth(300.0f)
			[
				GetWidgetForListenerParticipantPicker(
					TAttribute<FText>::Create(TAttribute<FText>::FGetter::CreateSP(this, &FDialogSequenceDetails::GetSectionListenerDisplayText)),
					FOnGetContent::CreateSP(this, &FDialogSequenceDetails::OnGetListenerParticipantListMenu))
			];

		// Re-add DialogLine children except SpeakerTag and ListenerTag
		if (DialogLinePropertyHandle.IsValid())
		{
			uint32 NumChildren = 0;
			DialogLinePropertyHandle->GetNumChildren(NumChildren);

			for (uint32 ChildIndex = 0; ChildIndex < NumChildren; ++ChildIndex)
			{
				TSharedPtr<IPropertyHandle> ChildHandle = DialogLinePropertyHandle->GetChildHandle(ChildIndex);
				if (!ChildHandle.IsValid() || !ChildHandle->GetProperty())
				{
					continue;
				}

				const FName ChildPropertyName = ChildHandle->GetProperty()->GetFName();
				if (ChildPropertyName == GET_MEMBER_NAME_CHECKED(FOrionDialogLine, SpeakerTag) ||
					ChildPropertyName == GET_MEMBER_NAME_CHECKED(FOrionDialogLine, ListenerTag))
				{
					continue;
				}

				SectionCategory.AddProperty(ChildHandle.ToSharedRef());
			}
		}
	}
}

FText FDialogSequenceDetails::GetSlotParticipantDisplayText() const
{
	const UDialogSequenceSlot* Slot = DialogSequenceSlot.Get();
	if (!Slot)
	{
		return LOCTEXT("NoSlotSelected", "None");
	}

	const UDialogBuilderGraph* Graph = Slot->OwningDialogGraph;
	if (Graph)
	{
		for (const TObjectPtr<UDialogParticipant>& ParticipantDefinition : Graph->ParticipantDefinitions)
		{
			if (ParticipantDefinition && ParticipantDefinition->Tag == Slot->ActorTag)
			{
				return !ParticipantDefinition->DisplayName.IsEmpty()
					? ParticipantDefinition->DisplayName
					: FText::FromString(ParticipantDefinition->Tag.ToString());
			}
		}
	}

	return Slot->ActorTag.IsValid()
		? FText::FromString(Slot->ActorTag.ToString())
		: LOCTEXT("NoParticipantSelected", "None");
}

FText FDialogSequenceDetails::GetSectionSpeakerDisplayText() const
{
	const UMovieSceneDialogSection* Section = MovieSceneDialogSection.Get();
	if (!Section)
	{
		return LOCTEXT("NoSectionSpeakerSelected", "None");
	}

	const UDialogBuilderGraph* Graph = Section->GetOwningDialogGraph();
	if (Graph)
	{
		for (const TObjectPtr<UDialogParticipant>& ParticipantDefinition : Graph->ParticipantDefinitions)
		{
			if (ParticipantDefinition && ParticipantDefinition->Tag == Section->DialogLine.SpeakerTag)
			{
				return !ParticipantDefinition->DisplayName.IsEmpty()
					? ParticipantDefinition->DisplayName
					: FText::FromString(ParticipantDefinition->Tag.ToString());
			}
		}
	}

	return Section->DialogLine.SpeakerTag.IsValid()
		? FText::FromString(Section->DialogLine.SpeakerTag.ToString())
		: LOCTEXT("NoSpeakerParticipantSelected", "None");
}

FText FDialogSequenceDetails::GetSectionListenerDisplayText() const
{
	const UMovieSceneDialogSection* Section = MovieSceneDialogSection.Get();
	if (!Section)
	{
		return LOCTEXT("NoSectionListenerSelected", "None");
	}

	const UDialogBuilderGraph* Graph = Section->GetOwningDialogGraph();
	if (Graph)
	{
		for (const TObjectPtr<UDialogParticipant>& ParticipantDefinition : Graph->ParticipantDefinitions)
		{
			if (ParticipantDefinition && ParticipantDefinition->Tag == Section->DialogLine.ListenerTag)
			{
				return !ParticipantDefinition->DisplayName.IsEmpty()
					? ParticipantDefinition->DisplayName
					: FText::FromString(ParticipantDefinition->Tag.ToString());
			}
		}
	}

	return Section->DialogLine.ListenerTag.IsValid()
		? FText::FromString(Section->DialogLine.ListenerTag.ToString())
		: LOCTEXT("NoListenerParticipantSelected", "None");
}

// DialogSequenceSlot ----------------------------------------------------------

TSharedRef<SWidget> FDialogSequenceDetails::GetWidgetForSlotParticipantPicker(const TAttribute<FText>& InParticipantNameText, FOnGetContent GetContentMenu)
{
	return
		SNew(SHorizontalBox)
		+ SHorizontalBox::Slot()
		.AutoWidth()
		[
			SNew(SComboButton)
				.ContentPadding(FMargin(4.0f, 2.0f))
				.ToolTipText(LOCTEXT("SlotParticipantPickerTooltip", "Participant definition to bind to this slot"))
				.OnGetMenuContent(GetContentMenu)
				.ButtonContent()
				[
					SNew(STextBlock)
						.Text_Lambda([InParticipantNameText]() { return InParticipantNameText.Get(); })
						.Font(IDetailLayoutBuilder::GetDetailFont())
				]
		];
}

TSharedRef<SWidget> FDialogSequenceDetails::OnGetSlotParticipantListMenu()
{
	FMenuBuilder MenuBuilder(true, nullptr);
	MenuBuilder.BeginSection("DialogSequenceSlotParticipants", LOCTEXT("SlotParticipantDefinitionsSectionLabel", "Participant Definitions"));

	if (const UDialogSequenceSlot* Slot = DialogSequenceSlot.Get())
	{
		if (const UDialogBuilderGraph* Graph = Slot->OwningDialogGraph)
		{
			for (const TObjectPtr<UDialogParticipant>& ParticipantDefinition : Graph->ParticipantDefinitions)
			{
				if (!ParticipantDefinition)
				{
					continue;
				}

				const FText EntryText = !ParticipantDefinition->DisplayName.IsEmpty()
					? ParticipantDefinition->DisplayName
					: FText::FromString(ParticipantDefinition->Tag.ToString());

				const FText EntryToolTip = FText::FromString(ParticipantDefinition->Tag.ToString());

				FUIAction Action(FExecuteAction::CreateSP(this, &FDialogSequenceDetails::UseSlotParticipantDefinition, ParticipantDefinition.Get()));
				MenuBuilder.AddMenuEntry(EntryText, EntryToolTip, FSlateIcon(), Action);
			}
		}
	}

	MenuBuilder.EndSection();
	return MenuBuilder.MakeWidget();
}

void FDialogSequenceDetails::UseSlotParticipantDefinition(UDialogParticipant* InParticipantDefinition)
{
	if (!InParticipantDefinition || !ActorTagPropertyHandle.IsValid())
	{
		return;
	}

	ActorTagPropertyHandle->SetValueFromFormattedString(InParticipantDefinition->Tag.ToString());
}

// MovieSceneDialogSection - Speaker ------------------------------------------

TSharedRef<SWidget> FDialogSequenceDetails::GetWidgetForSpeakerParticipantPicker(const TAttribute<FText>& InParticipantNameText, FOnGetContent GetContentMenu)
{
	return
		SNew(SHorizontalBox)
		+ SHorizontalBox::Slot()
		.AutoWidth()
		[
			SNew(SComboButton)
				.ContentPadding(FMargin(4.0f, 2.0f))
				.ToolTipText(LOCTEXT("SpeakerParticipantPickerTooltip", "Participant definition for speaker"))
				.OnGetMenuContent(GetContentMenu)
				.ButtonContent()
				[
					SNew(STextBlock)
						.Text_Lambda([InParticipantNameText]() { return InParticipantNameText.Get(); })
						.Font(IDetailLayoutBuilder::GetDetailFont())
				]
		];
}

TSharedRef<SWidget> FDialogSequenceDetails::OnGetSpeakerParticipantListMenu()
{
	FMenuBuilder MenuBuilder(true, nullptr);
	MenuBuilder.BeginSection("DialogSectionSpeakerParticipants", LOCTEXT("SpeakerParticipantDefinitionsSectionLabel", "Participant Definitions"));

	if (const UMovieSceneDialogSection* Section = MovieSceneDialogSection.Get())
	{
		if (const UDialogBuilderGraph* Graph = Section->GetOwningDialogGraph())
		{
			for (const TObjectPtr<UDialogParticipant>& ParticipantDefinition : Graph->ParticipantDefinitions)
			{
				if (!ParticipantDefinition)
				{
					continue;
				}

				const FText EntryText = !ParticipantDefinition->DisplayName.IsEmpty()
					? ParticipantDefinition->DisplayName
					: FText::FromString(ParticipantDefinition->Tag.ToString());

				const FText EntryToolTip = FText::FromString(ParticipantDefinition->Tag.ToString());

				FUIAction Action(FExecuteAction::CreateSP(this, &FDialogSequenceDetails::UseSpeakerParticipantDefinition, ParticipantDefinition.Get()));
				MenuBuilder.AddMenuEntry(EntryText, EntryToolTip, FSlateIcon(), Action);
			}
		}
	}

	MenuBuilder.EndSection();
	return MenuBuilder.MakeWidget();
}

void FDialogSequenceDetails::UseSpeakerParticipantDefinition(UDialogParticipant* InParticipantDefinition)
{
	if (!InParticipantDefinition || !SpeakerTagPropertyHandle.IsValid())
	{
		return;
	}

	SpeakerTagPropertyHandle->SetValueFromFormattedString(InParticipantDefinition->Tag.ToString());

	if (UMovieSceneDialogSection* Section = MovieSceneDialogSection.Get())
	{
		Section->RefreshDialogContent();
	}
}

// MovieSceneDialogSection - Listener -----------------------------------------

TSharedRef<SWidget> FDialogSequenceDetails::GetWidgetForListenerParticipantPicker(const TAttribute<FText>& InParticipantNameText, FOnGetContent GetContentMenu)
{
	return
		SNew(SHorizontalBox)
		+ SHorizontalBox::Slot()
		.AutoWidth()
		[
			SNew(SComboButton)
				.ContentPadding(FMargin(4.0f, 2.0f))
				.ToolTipText(LOCTEXT("ListenerParticipantPickerTooltip", "Participant definition for listener"))
				.OnGetMenuContent(GetContentMenu)
				.ButtonContent()
				[
					SNew(STextBlock)
						.Text_Lambda([InParticipantNameText]() { return InParticipantNameText.Get(); })
						.Font(IDetailLayoutBuilder::GetDetailFont())
				]
		];
}

TSharedRef<SWidget> FDialogSequenceDetails::OnGetListenerParticipantListMenu()
{
	FMenuBuilder MenuBuilder(true, nullptr);
	MenuBuilder.BeginSection("DialogSectionListenerParticipants", LOCTEXT("ListenerParticipantDefinitionsSectionLabel", "Participant Definitions"));

	if (const UMovieSceneDialogSection* Section = MovieSceneDialogSection.Get())
	{
		if (const UDialogBuilderGraph* Graph = Section->GetOwningDialogGraph())
		{
			for (const TObjectPtr<UDialogParticipant>& ParticipantDefinition : Graph->ParticipantDefinitions)
			{
				if (!ParticipantDefinition)
				{
					continue;
				}

				const FText EntryText = !ParticipantDefinition->DisplayName.IsEmpty()
					? ParticipantDefinition->DisplayName
					: FText::FromString(ParticipantDefinition->Tag.ToString());

				const FText EntryToolTip = FText::FromString(ParticipantDefinition->Tag.ToString());

				FUIAction Action(FExecuteAction::CreateSP(this, &FDialogSequenceDetails::UseListenerParticipantDefinition, ParticipantDefinition.Get()));
				MenuBuilder.AddMenuEntry(EntryText, EntryToolTip, FSlateIcon(), Action);
			}
		}
	}

	MenuBuilder.EndSection();
	return MenuBuilder.MakeWidget();
}

void FDialogSequenceDetails::UseListenerParticipantDefinition(UDialogParticipant* InParticipantDefinition)
{
	if (!InParticipantDefinition || !ListenerTagPropertyHandle.IsValid())
	{
		return;
	}

	ListenerTagPropertyHandle->SetValueFromFormattedString(InParticipantDefinition->Tag.ToString());
}

#undef LOCTEXT_NAMESPACE