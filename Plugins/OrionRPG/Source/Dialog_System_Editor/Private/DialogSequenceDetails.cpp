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
	ActorTagPropertyHandle.Reset();

	const TArray<TWeakObjectPtr<UObject>>& SelectedObjects = DetailBuilder.GetSelectedObjects();
	for (const TWeakObjectPtr<UObject>& CurrentObject : SelectedObjects)
	{
		if (CurrentObject.IsValid())
		{
			if (UDialogSequenceSlot* SlotPtr = Cast<UDialogSequenceSlot>(CurrentObject.Get()))
			{
				if (!DialogSequenceSlot.IsValid())
				{
					DialogSequenceSlot = SlotPtr;
				}
			}
		}
	}

	if (!DialogSequenceSlot.IsValid() || SelectedObjects.Num() != 1)
	{
		return;
	}

	IDetailCategoryBuilder& DetailCategory = DetailBuilder.EditCategory("Detail", LOCTEXT("DetailCategoryTitle", "Detail"));

	ActorTagPropertyHandle = DetailBuilder.GetProperty(GET_MEMBER_NAME_CHECKED(UDialogSequenceSlot, ActorTag));
	if (ActorTagPropertyHandle.IsValid())
	{
		ActorTagPropertyHandle->MarkHiddenByCustomization();
	}

	UDialogSequenceSlot* DialogSequenceSlotInstance = DialogSequenceSlot.Get();

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
			GetWidgetForParticipantPicker(
				TAttribute<FText>::Create(TAttribute<FText>::FGetter::CreateLambda([DialogSequenceSlotInstance]()
				{
					if (!DialogSequenceSlotInstance)
					{
						return LOCTEXT("NoSlotSelected", "None");
					}

					const UDialogBuilderGraph* Graph = DialogSequenceSlotInstance->OwningDialogGraph;
					if (Graph)
					{
						for (const TObjectPtr<UDialogParticipant>& ParticipantDefinition : Graph->ParticipantDefinitions)
						{
							if (ParticipantDefinition && ParticipantDefinition->Tag == DialogSequenceSlotInstance->ActorTag)
							{
								if (!ParticipantDefinition->DisplayName.IsEmpty())
								{
									return ParticipantDefinition->DisplayName;
								}

								return FText::FromString(ParticipantDefinition->Tag.ToString());
							}
						}
					}

					return DialogSequenceSlotInstance->ActorTag.IsValid()
						? FText::FromString(DialogSequenceSlotInstance->ActorTag.ToString())
						: LOCTEXT("NoParticipantSelected", "None");
				})),
				FOnGetContent::CreateSP(this, &FDialogSequenceDetails::OnGetParticipantListMenu))
		];

	DetailCategory.AddProperty(GET_MEMBER_NAME_CHECKED(UDialogSequenceSlot, DialogDefinition)).DisplayName(LOCTEXT("DialogDefinitionLabel", "Dialog Definition"));
	DetailCategory.AddProperty(GET_MEMBER_NAME_CHECKED(UDialogSequenceSlot, SlotPlacement)).DisplayName(LOCTEXT("SlotPlacementLabel", "Slot Placement"));
}

TSharedRef<SWidget> FDialogSequenceDetails::GetWidgetForParticipantPicker(const TAttribute<FText>& InParticipantNameText, FOnGetContent GetContentMenu)
{
	return
		SNew(SHorizontalBox)
		+ SHorizontalBox::Slot()
		.AutoWidth()
		[
			SNew(SComboButton)
				.ContentPadding(FMargin(4.0f, 2.0f))
				.ToolTipText(LOCTEXT("ParticipantPickerTooltip", "Participant definition to bind to this slot"))
				.OnGetMenuContent(GetContentMenu)
				.ButtonContent()
				[
					SNew(STextBlock)
						.Text_Lambda([InParticipantNameText]() { return InParticipantNameText.Get(); })
						.Font(IDetailLayoutBuilder::GetDetailFont())
				]
		];
}

TSharedRef<SWidget> FDialogSequenceDetails::OnGetParticipantListMenu()
{
	FMenuBuilder MenuBuilder(true, nullptr);

	MenuBuilder.BeginSection("DialogSequenceSlotParticipants", LOCTEXT("ParticipantDefinitionsSectionLabel", "Participant Definitions"));

	if (UDialogSequenceSlot* Slot = DialogSequenceSlot.Get())
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

				FUIAction Action(FExecuteAction::CreateSP(this, &FDialogSequenceDetails::UseParticipantDefinition, ParticipantDefinition.Get()));
				MenuBuilder.AddMenuEntry(EntryText, EntryToolTip, FSlateIcon(), Action);
			}
		}
	}

	MenuBuilder.EndSection();
	return MenuBuilder.MakeWidget();
}

void FDialogSequenceDetails::UseParticipantDefinition(UDialogParticipant* InParticipantDefinition)
{
	if (!InParticipantDefinition || !ActorTagPropertyHandle.IsValid())
	{
		return;
	}

	ActorTagPropertyHandle->SetValueFromFormattedString(InParticipantDefinition->Tag.ToString());
}

#undef LOCTEXT_NAMESPACE