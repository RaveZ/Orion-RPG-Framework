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

namespace
{
static bool IsParticipantInGraph(const UDialogBuilderGraph* Graph, const UDialogParticipant* Participant)
{
	if (!Graph || !Participant)
	{
		return false;
	}

	for (const TObjectPtr<UDialogParticipant>& Entry : Graph->ParticipantDefinitions)
	{
		if (Entry.Get() == Participant)
		{
			return true;
		}
	}

	return false;
}

static bool IsDialogDefinitionInGraph(const UDialogBuilderGraph* Graph, const UDialogDefinition* Definition)
{
	if (!Graph || !Definition)
	{
		return false;
	}

	for (const TObjectPtr<UDialogParticipant>& Entry : Graph->ParticipantDefinitions)
	{
		if (Entry.Get() == Definition)
		{
			return true;
		}
	}

	for (const TObjectPtr<UDialogProp>& Entry : Graph->PropDefinitions)
	{
		if (Entry.Get() == Definition)
		{
			return true;
		}
	}

	return false;
}

static FText GetDialogDefinitionText(const UDialogDefinition* Definition)
{
	if (!Definition)
	{
		return LOCTEXT("NoneText", "None");
	}

	return !Definition->DisplayName.IsEmpty()
		? Definition->DisplayName
		: FText::FromString(Definition->Tag.ToString());
}

static FText GetParticipantText(const UDialogParticipant* Participant)
{
	if (!Participant)
	{
		return LOCTEXT("NoneText", "None");
	}

	return !Participant->DisplayName.IsEmpty()
		? Participant->DisplayName
		: FText::FromString(Participant->Tag.ToString());
}
}

TSharedRef<IDetailCustomization> FDialogSequenceDetails::MakeInstance()
{
	return MakeShareable(new FDialogSequenceDetails);
}

void FDialogSequenceDetails::CustomizeDetails(IDetailLayoutBuilder& DetailBuilder)
{
	DialogSequenceSlot.Reset();
	MovieSceneDialogSection.Reset();

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

	// Slot customization
	if (UDialogSequenceSlot* Slot = DialogSequenceSlot.Get())
	{
		if (!IsParticipantInGraph(Slot->OwningDialogGraph, Cast<UDialogParticipant>(Slot->DialogDefinition)))
		{
			Slot->Modify();
			Slot->DialogDefinition = nullptr;
			Slot->ActorTag = FGameplayTag();
		}

		if (!IsDialogDefinitionInGraph(Slot->OwningDialogGraph, Slot->DialogDefinition))
		{
			Slot->Modify();
			Slot->DialogDefinition = nullptr;
			Slot->ActorTag = FGameplayTag();
		}

		IDetailCategoryBuilder& DetailCategory = DetailBuilder.EditCategory("Detail", LOCTEXT("DetailCategoryTitle", "Detail"));

		TSharedPtr<IPropertyHandle> ActorTagHandle = DetailBuilder.GetProperty(GET_MEMBER_NAME_CHECKED(UDialogSequenceSlot, ActorTag));
		if (ActorTagHandle.IsValid())
		{
			ActorTagHandle->MarkHiddenByCustomization();
		}

		TSharedPtr<IPropertyHandle> DialogDefinitionHandle = DetailBuilder.GetProperty(GET_MEMBER_NAME_CHECKED(UDialogSequenceSlot, DialogDefinition));
		if (DialogDefinitionHandle.IsValid())
		{
			DialogDefinitionHandle->MarkHiddenByCustomization();
		}

		DetailCategory.AddCustomRow(LOCTEXT("ParticipantToBindLabel", "Participant To Bind"))
			.NameContent()
			[
				SNew(STextBlock)
					.Text(LOCTEXT("ParticipantToBindText", "Participant To Bind"))
					.Font(IDetailLayoutBuilder::GetDetailFont())
			]
			.ValueContent()
			.MaxDesiredWidth(300.0f)
			[
				GetWidgetForSlotParticipantPicker(
					TAttribute<FText>::Create(TAttribute<FText>::FGetter::CreateSP(this, &FDialogSequenceDetails::GetSlotParticipantDisplayText)),
					FOnGetContent::CreateSP(this, &FDialogSequenceDetails::OnGetSlotParticipantListMenu))
			];

		DetailCategory.AddProperty(GET_MEMBER_NAME_CHECKED(UDialogSequenceSlot, SlotPlacement)).DisplayName(LOCTEXT("SlotPlacementLabel", "Slot Placement"));
		return;
	}

	// Section customization
	if (UMovieSceneDialogSection* Section = MovieSceneDialogSection.Get())
	{
		Section->Modify();
		Section->RefreshDialogContent();

		IDetailCategoryBuilder& SectionCategory = DetailBuilder.EditCategory("Section", LOCTEXT("SectionCategoryTitle", "Section"));

		TSharedPtr<IPropertyHandle> DialogLineHandle = DetailBuilder.GetProperty(GET_MEMBER_NAME_CHECKED(UMovieSceneDialogSection, DialogLine));
		if (DialogLineHandle.IsValid())
		{
			DialogLineHandle->MarkHiddenByCustomization();
		}

		TSharedPtr<IPropertyHandle> SpeakerRefHandle = DetailBuilder.GetProperty(GET_MEMBER_NAME_CHECKED(UMovieSceneDialogSection, SpeakerParticipantDefinition));
		if (SpeakerRefHandle.IsValid())
		{
			SpeakerRefHandle->MarkHiddenByCustomization();
		}

		TSharedPtr<IPropertyHandle> ListenerRefHandle = DetailBuilder.GetProperty(GET_MEMBER_NAME_CHECKED(UMovieSceneDialogSection, ListenerParticipantDefinition));
		if (ListenerRefHandle.IsValid())
		{
			ListenerRefHandle->MarkHiddenByCustomization();
		}

		SectionCategory.AddCustomRow(LOCTEXT("SpeakerParticipantRowLabel", "Speaker Participant"))
			.NameContent()
			[
				SNew(STextBlock)
					.Text(LOCTEXT("SpeakerParticipantText", "Speaker"))
					.Font(IDetailLayoutBuilder::GetDetailFont())
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
			]
			.ValueContent()
			.MaxDesiredWidth(300.0f)
			[
				GetWidgetForListenerParticipantPicker(
					TAttribute<FText>::Create(TAttribute<FText>::FGetter::CreateSP(this, &FDialogSequenceDetails::GetSectionListenerDisplayText)),
					FOnGetContent::CreateSP(this, &FDialogSequenceDetails::OnGetListenerParticipantListMenu))
			];

		if (DialogLineHandle.IsValid())
		{
			uint32 NumChildren = 0;
			DialogLineHandle->GetNumChildren(NumChildren);

			for (uint32 ChildIndex = 0; ChildIndex < NumChildren; ++ChildIndex)
			{
				TSharedPtr<IPropertyHandle> ChildHandle = DialogLineHandle->GetChildHandle(ChildIndex);
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
	if (!Slot || !Slot->OwningDialogGraph)
	{
		return LOCTEXT("SlotNone", "None");
	}

	const UDialogDefinition* Selected = Slot->DialogDefinition;
	return IsDialogDefinitionInGraph(Slot->OwningDialogGraph, Selected) ? GetDialogDefinitionText(Selected) : LOCTEXT("SlotNoneMissing", "None");
}

FText FDialogSequenceDetails::GetSectionSpeakerDisplayText() const
{
	const UMovieSceneDialogSection* Section = MovieSceneDialogSection.Get();
	const UDialogBuilderGraph* Graph = Section ? Section->GetOwningDialogGraph() : nullptr;
	return IsParticipantInGraph(Graph, Section ? Section->SpeakerParticipantDefinition.Get() : nullptr)
		? GetParticipantText(Section->SpeakerParticipantDefinition.Get())
		: LOCTEXT("SpeakerNone", "None");
}

FText FDialogSequenceDetails::GetSectionListenerDisplayText() const
{
	const UMovieSceneDialogSection* Section = MovieSceneDialogSection.Get();
	const UDialogBuilderGraph* Graph = Section ? Section->GetOwningDialogGraph() : nullptr;
	return IsParticipantInGraph(Graph, Section ? Section->ListenerParticipantDefinition.Get() : nullptr)
		? GetParticipantText(Section->ListenerParticipantDefinition.Get())
		: LOCTEXT("ListenerNone", "None");
}

// Slot
TSharedRef<SWidget> FDialogSequenceDetails::GetWidgetForSlotParticipantPicker(const TAttribute<FText>& InParticipantNameText, FOnGetContent GetContentMenu)
{
	return SNew(SHorizontalBox)
		+ SHorizontalBox::Slot().AutoWidth()
		[
			SNew(SComboButton)
				.ContentPadding(FMargin(4.0f, 2.0f))
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

	MenuBuilder.BeginSection("SlotParticipants", LOCTEXT("SlotParticipantsSection", "Participant Definitions"));
	MenuBuilder.AddMenuEntry(
		LOCTEXT("SlotParticipantNone", "None"),
		FText(),
		FSlateIcon(),
		FUIAction(FExecuteAction::CreateSP(this, &FDialogSequenceDetails::ClearSlotParticipantDefinition)));

	if (const UDialogSequenceSlot* Slot = DialogSequenceSlot.Get())
	{
		if (const UDialogBuilderGraph* Graph = Slot->OwningDialogGraph)
		{
			for (const TObjectPtr<UDialogParticipant>& Participant : Graph->ParticipantDefinitions)
			{
				if (!Participant)
				{
					continue;
				}

				MenuBuilder.AddMenuEntry(
					GetDialogDefinitionText(Participant.Get()),
					FText::FromString(Participant->Tag.ToString()),
					FSlateIcon(),
					FUIAction(FExecuteAction::CreateSP(this, &FDialogSequenceDetails::UseSlotParticipantDefinition, static_cast<UDialogDefinition*>(Participant.Get()))));
			}
		}
	}
	MenuBuilder.EndSection();

	MenuBuilder.BeginSection("SlotProps", LOCTEXT("SlotPropsSection", "Prop Definitions"));
	if (const UDialogSequenceSlot* Slot = DialogSequenceSlot.Get())
	{
		if (const UDialogBuilderGraph* Graph = Slot->OwningDialogGraph)
		{
			for (const TObjectPtr<UDialogProp>& Prop : Graph->PropDefinitions)
			{
				if (!Prop)
				{
					continue;
				}

				MenuBuilder.AddMenuEntry(
					GetDialogDefinitionText(Prop.Get()),
					FText::FromString(Prop->Tag.ToString()),
					FSlateIcon(),
					FUIAction(FExecuteAction::CreateSP(this, &FDialogSequenceDetails::UseSlotParticipantDefinition, static_cast<UDialogDefinition*>(Prop.Get()))));
			}
		}
	}
	MenuBuilder.EndSection();

	return MenuBuilder.MakeWidget();
}

void FDialogSequenceDetails::UseSlotParticipantDefinition(UDialogDefinition* InDialogDefinition)
{
	if (!InDialogDefinition || !DialogSequenceSlot.IsValid())
	{
		return;
	}

	UDialogSequenceSlot* Slot = DialogSequenceSlot.Get();
	if (!IsDialogDefinitionInGraph(Slot->OwningDialogGraph, InDialogDefinition))
	{
		ClearSlotParticipantDefinition();
		return;
	}

	Slot->Modify();
	Slot->DialogDefinition = InDialogDefinition;
	Slot->ActorTag = InDialogDefinition->Tag;
}

void FDialogSequenceDetails::ClearSlotParticipantDefinition()
{
	if (UDialogSequenceSlot* Slot = DialogSequenceSlot.Get())
	{
		Slot->Modify();
		Slot->DialogDefinition = nullptr;
		Slot->ActorTag = FGameplayTag();
	}
}

// Speaker
TSharedRef<SWidget> FDialogSequenceDetails::GetWidgetForSpeakerParticipantPicker(const TAttribute<FText>& InParticipantNameText, FOnGetContent GetContentMenu)
{
	return SNew(SHorizontalBox)
		+ SHorizontalBox::Slot().AutoWidth()
		[
			SNew(SComboButton)
				.ContentPadding(FMargin(4.0f, 2.0f))
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
	MenuBuilder.BeginSection("SpeakerParticipants", LOCTEXT("SpeakerParticipantsSection", "Participant Definitions"));

	MenuBuilder.AddMenuEntry(
		LOCTEXT("SpeakerParticipantNone", "None"),
		FText(),
		FSlateIcon(),
		FUIAction(FExecuteAction::CreateSP(this, &FDialogSequenceDetails::ClearSpeakerParticipantDefinition)));

	if (const UMovieSceneDialogSection* Section = MovieSceneDialogSection.Get())
	{
		if (const UDialogBuilderGraph* Graph = Section->GetOwningDialogGraph())
		{
			for (const TObjectPtr<UDialogParticipant>& Participant : Graph->ParticipantDefinitions)
			{
				if (!Participant)
				{
					continue;
				}

				MenuBuilder.AddMenuEntry(
					GetParticipantText(Participant.Get()),
					FText::FromString(Participant->Tag.ToString()),
					FSlateIcon(),
					FUIAction(FExecuteAction::CreateSP(this, &FDialogSequenceDetails::UseSpeakerParticipantDefinition, Participant.Get())));
			}
		}
	}

	MenuBuilder.EndSection();
	return MenuBuilder.MakeWidget();
}

void FDialogSequenceDetails::UseSpeakerParticipantDefinition(UDialogParticipant* InParticipantDefinition)
{
	if (!InParticipantDefinition || !MovieSceneDialogSection.IsValid())
	{
		return;
	}

	UMovieSceneDialogSection* Section = MovieSceneDialogSection.Get();
	if (!IsParticipantInGraph(Section->GetOwningDialogGraph(), InParticipantDefinition))
	{
		ClearSpeakerParticipantDefinition();
		return;
	}

	Section->Modify();
	Section->SpeakerParticipantDefinition = InParticipantDefinition;
	Section->RefreshDialogContent();
}

void FDialogSequenceDetails::ClearSpeakerParticipantDefinition()
{
	if (UMovieSceneDialogSection* Section = MovieSceneDialogSection.Get())
	{
		Section->Modify();
		Section->SpeakerParticipantDefinition = nullptr;
		Section->RefreshDialogContent();
	}
}

// Listener
TSharedRef<SWidget> FDialogSequenceDetails::GetWidgetForListenerParticipantPicker(const TAttribute<FText>& InParticipantNameText, FOnGetContent GetContentMenu)
{
	return SNew(SHorizontalBox)
		+ SHorizontalBox::Slot().AutoWidth()
		[
			SNew(SComboButton)
				.ContentPadding(FMargin(4.0f, 2.0f))
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
	MenuBuilder.BeginSection("ListenerParticipants", LOCTEXT("ListenerParticipantsSection", "Participant Definitions"));

	MenuBuilder.AddMenuEntry(
		LOCTEXT("ListenerParticipantNone", "None"),
		FText(),
		FSlateIcon(),
		FUIAction(FExecuteAction::CreateSP(this, &FDialogSequenceDetails::ClearListenerParticipantDefinition)));

	if (const UMovieSceneDialogSection* Section = MovieSceneDialogSection.Get())
	{
		if (const UDialogBuilderGraph* Graph = Section->GetOwningDialogGraph())
		{
			for (const TObjectPtr<UDialogParticipant>& Participant : Graph->ParticipantDefinitions)
			{
				if (!Participant)
				{
					continue;
				}

				MenuBuilder.AddMenuEntry(
					GetParticipantText(Participant.Get()),
					FText::FromString(Participant->Tag.ToString()),
					FSlateIcon(),
					FUIAction(FExecuteAction::CreateSP(this, &FDialogSequenceDetails::UseListenerParticipantDefinition, Participant.Get())));
			}
		}
	}

	MenuBuilder.EndSection();
	return MenuBuilder.MakeWidget();
}

void FDialogSequenceDetails::UseListenerParticipantDefinition(UDialogParticipant* InParticipantDefinition)
{
	if (!InParticipantDefinition || !MovieSceneDialogSection.IsValid())
	{
		return;
	}

	UMovieSceneDialogSection* Section = MovieSceneDialogSection.Get();
	if (!IsParticipantInGraph(Section->GetOwningDialogGraph(), InParticipantDefinition))
	{
		ClearListenerParticipantDefinition();
		return;
	}

	Section->Modify();
	Section->ListenerParticipantDefinition = InParticipantDefinition;
	Section->RefreshDialogContent();
}

void FDialogSequenceDetails::ClearListenerParticipantDefinition()
{
	if (UMovieSceneDialogSection* Section = MovieSceneDialogSection.Get())
	{
		Section->Modify();
		Section->ListenerParticipantDefinition = nullptr;
		Section->RefreshDialogContent();
	}
}

#undef LOCTEXT_NAMESPACE