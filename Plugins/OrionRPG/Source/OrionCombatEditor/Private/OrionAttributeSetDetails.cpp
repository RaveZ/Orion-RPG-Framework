// Copyright 2025 Ivan Chandra. All Rights Reserved.

#include "OrionAttributeSetDetails.h"

#include "AbilitySystem/Attributes/OrionAttributeSet.h"
#include "AttributeSet.h"
#include "DetailCategoryBuilder.h"
#include "DetailLayoutBuilder.h"
#include "DetailWidgetRow.h"
#include "EdGraphSchema_K2.h"
#include "IPropertyUtilities.h"
#include "Kismet2/BlueprintEditorUtils.h"
#include "ScopedTransaction.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Text/STextBlock.h"

#define LOCTEXT_NAMESPACE "OrionAttributeSetDetails"

TSharedRef<IDetailCustomization> FOrionAttributeSetDetails::MakeInstance()
{
	return MakeShared<FOrionAttributeSetDetails>();
}

void FOrionAttributeSetDetails::CustomizeDetails(IDetailLayoutBuilder& DetailBuilder)
{
	PropertyUtilities = DetailBuilder.GetPropertyUtilities();
	AttributeSetBlueprint.Reset();

	TArray<TWeakObjectPtr<UObject>> Objects;
	DetailBuilder.GetObjectsBeingCustomized(Objects);
	for (const TWeakObjectPtr<UObject>& ObjectPtr : Objects)
	{
		const UObject* Object = ObjectPtr.Get();
		const UClass* ObjectClass = Object ? Object->GetClass() : nullptr;
		if (!ObjectClass || !ObjectClass->IsChildOf(UOrionAttributeSet::StaticClass()))
		{
			continue;
		}

		if (UBlueprint* Blueprint = Cast<UBlueprint>(ObjectClass->ClassGeneratedBy))
		{
			AttributeSetBlueprint = Blueprint;
			break;
		}
	}

	IDetailCategoryBuilder& Category = DetailBuilder.EditCategory("Orion Attribute Set");
	Category.AddCustomRow(LOCTEXT("AddGameplayAttributeFilter", "Add Gameplay Attribute"))
		.WholeRowContent()
		[
			SNew(SButton)
			.Text(LOCTEXT("AddGameplayAttributeButton", "Add Gameplay Attribute"))
			.ToolTipText(this, &FOrionAttributeSetDetails::GetAddGameplayAttributeToolTip)
			.IsEnabled(this, &FOrionAttributeSetDetails::CanAddGameplayAttribute)
			.HAlign(HAlign_Center)
			.OnClicked_Lambda([this]()
			{
				AddGameplayAttribute();
				return FReply::Handled();
			})
		];
}

bool FOrionAttributeSetDetails::CanAddGameplayAttribute() const
{
	return AttributeSetBlueprint.IsValid();
}

FText FOrionAttributeSetDetails::GetAddGameplayAttributeToolTip() const
{
	return CanAddGameplayAttribute()
		? LOCTEXT("AddGameplayAttributeToolTip", "Add a Blueprint FGameplayAttributeData variable to this Orion Attribute Set.")
		: LOCTEXT("AddGameplayAttributeDisabledToolTip", "Open class defaults for a Blueprint subclass of UOrionAttributeSet to add attributes.");
}

void FOrionAttributeSetDetails::AddGameplayAttribute()
{
	UBlueprint* Blueprint = AttributeSetBlueprint.Get();
	if (!Blueprint)
	{
		return;
	}

	const FScopedTransaction Transaction(LOCTEXT("AddGameplayAttributeTransaction", "Add Gameplay Attribute"));
	Blueprint->Modify();

	const FName VariableName = FBlueprintEditorUtils::FindUniqueKismetName(Blueprint, TEXT("NewAttribute"));

	FEdGraphPinType AttributePinType;
	AttributePinType.PinCategory = UEdGraphSchema_K2::PC_Struct;
	AttributePinType.PinSubCategoryObject = FGameplayAttributeData::StaticStruct();

	if (FBlueprintEditorUtils::AddMemberVariable(Blueprint, VariableName, AttributePinType))
	{
		FBlueprintEditorUtils::SetBlueprintVariableCategory(Blueprint, VariableName, nullptr, LOCTEXT("AttributeVariableCategory", "Attributes"), true);
		FBlueprintEditorUtils::SetBlueprintOnlyEditableFlag(Blueprint, VariableName, false);
		FBlueprintEditorUtils::SetBlueprintVariableMetaData(Blueprint, VariableName, nullptr, TEXT("AllowPrivateAccess"), TEXT("true"));
		FBlueprintEditorUtils::MarkBlueprintAsStructurallyModified(Blueprint);

		if (const TSharedPtr<IPropertyUtilities> Utilities = PropertyUtilities.Pin())
		{
			Utilities->ForceRefresh();
		}
	}
}

#undef LOCTEXT_NAMESPACE
