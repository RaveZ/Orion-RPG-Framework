// Copyright 2025 Ivan Chandra. All Rights Reserved.

#include "DialogParticipantDetails.h"

#include "DialogBuilderEdNode_DialogLine.h"
#include "DialogBuilderNode_DialogLine.h"
#include "Containers/Array.h"
#include "Containers/EnumAsByte.h"
#include "Containers/Map.h"
#include "Delegates/Delegate.h"
#include "DetailCategoryBuilder.h"
#include "DetailLayoutBuilder.h"
#include "DetailWidgetRow.h"
#include "EdGraph/EdGraph.h"
#include "EdGraph/EdGraphPin.h"
#include "Engine/Blueprint.h"
#include "Fonts/SlateFontInfo.h"
#include "Framework/Application/MenuStack.h"
#include "Framework/Application/SlateApplication.h"
#include "Framework/Commands/UIAction.h"
#include "Framework/MultiBox/MultiBoxBuilder.h"
#include "HAL/PlatformCrt.h"
#include "HAL/PlatformMath.h"
#include "IDetailPropertyRow.h"
#include "Internationalization/Internationalization.h"
#include "Internationalization/Text.h"
#include "Kismet2/BlueprintEditorUtils.h"
#include "Kismet2/KismetEditorUtilities.h"
#include "Layout/Margin.h"
#include "Layout/WidgetPath.h"
#include "Math/Color.h"
#include "Misc/AssertionMacros.h"
#include "PropertyEditorModule.h"
#include "PropertyHandle.h"
#include "SKismetLinearExpression.h"
#include "SlateOptMacros.h"
#include "SlotBase.h"
#include "Styling/AppStyle.h"
#include "Templates/Casts.h"
#include "Templates/SubclassOf.h"
#include "Textures/SlateIcon.h"
#include "UObject/NameTypes.h"
#include "UObject/Object.h"
#include "UObject/ObjectPtr.h"
#include "Widgets/DeclarativeSyntaxSupport.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Input/SComboButton.h"
#include "Widgets/Input/STextEntryPopup.h"
#include "Widgets/SBoxPanel.h"
#include "Widgets/SWindow.h"
#include "Widgets/Text/STextBlock.h"

#define LOCTEXT_NAMESPACE "DetailsCustomization"

/////////////////////////////////////////////////////////////////////////

TSharedRef<IDetailCustomization> FDialogParticipantDetails::MakeInstance()
{
	return MakeShareable(new FDialogParticipantDetails);
}



BEGIN_SLATE_FUNCTION_BUILD_OPTIMIZATION
void FDialogParticipantDetails::CustomizeDetails(IDetailLayoutBuilder& DetailBuilder)
{
	// Get a handle to the node we're viewing
	const TArray< TWeakObjectPtr<UObject> >& SelectedObjects = DetailBuilder.GetSelectedObjects();
	for (int32 ObjectIndex = 0; ObjectIndex < SelectedObjects.Num(); ++ObjectIndex)
	{
		const TWeakObjectPtr<UObject>& CurrentObject = SelectedObjects[ObjectIndex];
		if (CurrentObject.IsValid())
		{
			if (UDialogBuilderNode_DialogLine* DialogLineNodePtr = Cast<UDialogBuilderNode_DialogLine>(CurrentObject.Get()))
			{
				if (!DialogLineNode.IsValid())
				{
					DialogLineNode = DialogLineNodePtr;
				}

			}
		}
	}


	IDetailCategoryBuilder& DialogParticipantCategory = DetailBuilder.EditCategory("Participant", LOCTEXT("ParticipantCategoryTitle", "Participant Settings"));
	if (DialogLineNode != NULL && SelectedObjects.Num() == 1)
	{
		UDialogBuilderNode_DialogLine* DialogLineNodeInstance = DialogLineNode.Get();
		// The sharing option for the crossfade settings
		DialogParticipantCategory.AddCustomRow(LOCTEXT("ParticipantSettingsSharingLabel", "Participant Sharing"))
			.NameContent()
			[
				SNew(STextBlock)
					.Text(LOCTEXT("ParticipantSharingLabel", "Participant Sharing"))
					.Font(IDetailLayoutBuilder::GetDetailFont())
					.ToolTipText(LOCTEXT("UseSharedParticipant_ToolTip", "Participant Sharing let you access and share other participant data.\n Convenient when you need to input the same participant multiple times."))
			]
			.ValueContent()
			.MaxDesiredWidth(300.0f)
			[
				GetWidgetForInlineShareMenu(
					TAttribute<FText>::Create(TAttribute<FText>::FGetter::CreateLambda([DialogLineNodeInstance]() { return FText::FromString(DialogLineNodeInstance->SharedParticipantName); })),
					TAttribute<bool>::Create(TAttribute<bool>::FGetter::CreateLambda([DialogLineNodeInstance]() { return DialogLineNodeInstance->bSharedParticipant; })),
					FOnClicked::CreateSP(this, &FDialogParticipantDetails::OnPromoteToSharedClick),
					FOnClicked::CreateSP(this, &FDialogParticipantDetails::OnUnshareClick),
					FOnGetContent::CreateSP(this, &FDialogParticipantDetails::OnGetShareableNodesMenu))
			]; 
	}

	DialogParticipantCategory.AddProperty(GET_MEMBER_NAME_CHECKED(UDialogBuilderNode_DialogLine, ParticipantName)).DisplayName(LOCTEXT("ParticipantNameLabel", "Participant Name"));
	DialogParticipantCategory.AddProperty(GET_MEMBER_NAME_CHECKED(UDialogBuilderNode_DialogLine, ParticipantTag)).DisplayName(LOCTEXT("ParticipantTagLabel", "Participant Tag"));

}
END_SLATE_FUNCTION_BUILD_OPTIMIZATION

TSharedRef<SWidget> FDialogParticipantDetails::GetWidgetForInlineShareMenu(const TAttribute<FText>& InSharedNameText, const TAttribute<bool>& bInIsCurrentlyShared, FOnClicked PromoteClick, FOnClicked DemoteClick, FOnGetContent GetContentMenu)
{
	return
		SNew(SHorizontalBox)
		+ SHorizontalBox::Slot()
		.AutoWidth()
		[
			SNew(SComboButton)
				.ContentPadding(FMargin(4.0f, 2.0f))
				.ToolTipText(LOCTEXT("UseSharedParticipant_ToolTip", "Use Shared Participant"))
				.OnGetMenuContent(GetContentMenu)
				.ButtonContent()
				[
					SNew(STextBlock)
						.Text_Lambda([bInIsCurrentlyShared, InSharedNameText]() { return bInIsCurrentlyShared.Get() ? InSharedNameText.Get() : LOCTEXT("SharedTransition", "Use Shared"); })
						.Font(IDetailLayoutBuilder::GetDetailFont())
				]
		]

		+ SHorizontalBox::Slot()
		.AutoWidth()
		.Padding(3.0f, 0.0f, 0.0f, 0.0f)
		[
			SNew(SButton)
				.ContentPadding(FMargin(4.0f, 2.0f))
				.HAlign(HAlign_Center)
				.VAlign(VAlign_Center)
				.OnClicked_Lambda([bInIsCurrentlyShared, DemoteClick, PromoteClick]() { return bInIsCurrentlyShared.Get() ? DemoteClick.Execute() : PromoteClick.Execute(); })
				.Text_Lambda([bInIsCurrentlyShared]() { return bInIsCurrentlyShared.Get() ? LOCTEXT("UnshareLabel", "Unshare") : LOCTEXT("ShareLabel", "Promote To Shared"); })
				.TextStyle(&FAppStyle::Get(), TEXT("TinyText"))
		];
}

/** RuleShare = true if we are sharing the rules of this participant (else we are implied to be sharing the participant settings) */
FReply FDialogParticipantDetails::OnPromoteToSharedClick()
{
	TSharedPtr< SWindow > Parent = FSlateApplication::Get().GetActiveTopLevelWindow();
	if (Parent.IsValid())
	{
		// Show dialog to enter new track name
		TSharedRef<STextEntryPopup> TextEntry =
			SNew(STextEntryPopup)
			.Label(LOCTEXT("PromoteDialogLineNodeToSharedLabel", "Shared Participant Name"))
			.OnTextCommitted(this, &FDialogParticipantDetails::PromoteToShared);

		// Show dialog to enter new event name
		FSlateApplication::Get().PushMenu(
			Parent.ToSharedRef(),
			FWidgetPath(),
			TextEntry,
			FSlateApplication::Get().GetCursorPos(),
			FPopupTransitionEffect(FPopupTransitionEffect::TypeInPopup)
		);
		TextEntryWidget = TextEntry;
	}

	return FReply::Handled();
}

void FDialogParticipantDetails::PromoteToShared(const FText& NewTransitionName, ETextCommit::Type CommitInfo)
{
	if (CommitInfo == ETextCommit::OnEnter)
	{
		if (DialogLineNode.Get())
		{
			DialogLineNode.Get()->MakeParticipantShareable(NewTransitionName.ToString());
		}
	}

	FSlateApplication::Get().DismissAllMenus();
}

FReply FDialogParticipantDetails::OnUnshareClick()
{
	if (DialogLineNode.Get())
	{
		DialogLineNode.Get()->UnshareParticipant();
	}

	return FReply::Handled();
}

TSharedRef<SWidget> FDialogParticipantDetails::OnGetShareableNodesMenu()
{
	FMenuBuilder MenuBuilder(true, NULL);

	FText SectionText;

	SectionText = LOCTEXT("PickSharedParticipantSettings", "Shared Settings");
	

	MenuBuilder.BeginSection("ParticipantSharableNodes", SectionText);

	if (UDialogBuilderNode_DialogLine* DialogLineNodeInstance = DialogLineNode.Get())
	{
		const UDialogBuilderGraph* CurrentDialogGraph = DialogLineNode.Get()->GetOwningDialogGraph();

		// Collect all unique shared participant and group them by their name.
		TMultiMap<FString, UDialogBuilderNode_DialogLine*> SharedParticipants;

		for (auto& Node : CurrentDialogGraph->AllNodes)
		{
			UDialogBuilderNode_DialogLine* DialogLineNodeInst = Node ? Cast<UDialogBuilderNode_DialogLine>(Node) : nullptr;
			if (DialogLineNodeInst)
			{
				if (!DialogLineNodeInst->SharedParticipantName.IsEmpty())
				{
					SharedParticipants.Add(DialogLineNodeInst->SharedParticipantName, DialogLineNodeInst);
				}
			}
		}
		

		// Get the unique shared participant names
		TSet<FString> SharedParticipantKeys;
		SharedParticipants.GetKeys(SharedParticipantKeys);

		//Iterate through the unique shared participant names and list all the places where they are referenced in the tooltip.
		TArray<UDialogBuilderNode_DialogLine*> UsedIn;
		for (const FString& Key : SharedParticipantKeys)
		{
			UsedIn.Reset();
			SharedParticipants.MultiFind(Key, UsedIn, true);
			if (UsedIn.IsEmpty())
			{
				continue;
			}

			FTextBuilder ToolTipBuilder;
			ToolTipBuilder.AppendLine(LOCTEXT("DialogLineUsedBy", "Used by:"));
			for (const UDialogBuilderNode_DialogLine* UsedInTransitionNode : UsedIn)
			{
				ToolTipBuilder.AppendLine(UsedInTransitionNode->GetOwningDialogGraph()->GetName());
			}

			FUIAction Action = FUIAction(FExecuteAction::CreateSP(this, &FDialogParticipantDetails::BecomeSharedWith, UsedIn[0]));
			MenuBuilder.AddMenuEntry(FText::FromString(Key), ToolTipBuilder.ToText(), FSlateIcon(), Action);
		}
	}
	MenuBuilder.EndSection();

	return MenuBuilder.MakeWidget();
}

void FDialogParticipantDetails::BecomeSharedWith(UDialogBuilderNode_DialogLine* NewNode)
{
	if (DialogLineNode.Get())
	{
		DialogLineNode.Get()->UseSharedParticipant(NewNode);
		
	}
}


















#undef LOCTEXT_NAMESPACE

