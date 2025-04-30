// Copyright Epic Games, Inc. All Rights Reserved.

#include "AttributeDetails.h"
#include "UObject/UnrealType.h"
#include "SlateOptMacros.h"
#include "Widgets/Text/STextBlock.h"
#include "Widgets/Input/SComboButton.h"
#include "Widgets/Input/SSlider.h"
#include "Engine/CurveTable.h"
#include "PropertyHandle.h"
#include "IPropertyUtilities.h"
#include "DetailLayoutBuilder.h"
#include "DetailWidgetRow.h"
#include "DetailCategoryBuilder.h"
#include "Widgets/Input/STextComboBox.h"
#include "PropertyCustomizationHelpers.h"
#include "Editor.h"
#include "IDetailChildrenBuilder.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Text/STextBlock.h"
#include "QuestData.h"

TSharedRef<IPropertyTypeCustomization> FQuestPropertyDetails::MakeInstance()
{
	return MakeShareable(new FQuestPropertyDetails);
}

void FQuestPropertyDetails::CustomizeHeader(TSharedRef<class IPropertyHandle> StructPropertyHandle, FDetailWidgetRow& HeaderRow, IPropertyTypeCustomizationUtils& StructCustomizationUtils)
{
    bSourceRefreshQueued = false;
    PropertyUtilities = StructCustomizationUtils.GetPropertyUtilities();

    QuestIdProperty = StructPropertyHandle->GetChildHandle(GET_MEMBER_NAME_CHECKED(FQuestDataHandle, QuestID));
    NodeIdProperty = StructPropertyHandle->GetChildHandle(GET_MEMBER_NAME_CHECKED(FQuestDataHandle, NodeID));

    if (QuestIdProperty.IsValid() && NodeIdProperty.IsValid())
    {
		QuestIdProperty->SetOnPropertyValueChanged(FSimpleDelegate::CreateSP(this, &FQuestPropertyDetails::OnQuestIdChanged));
		QuestIdProperty->SetOnPropertyValuePreChange(FSimpleDelegate::CreateSP(this, &FQuestPropertyDetails::OnQuestIdPreChange));
		NodeIdProperty->SetOnPropertyValueChanged(FSimpleDelegate::CreateSP(this, &FQuestPropertyDetails::OnNodeIdChanged));
		NodeIdProperty->SetOnPropertyValuePreChange(FSimpleDelegate::CreateSP(this, &FQuestPropertyDetails::OnNodeIdPreChange));
    }
}

void FQuestPropertyDetails::CustomizeChildren(TSharedRef<class IPropertyHandle> StructPropertyHandle, IDetailChildrenBuilder& StructBuilder, IPropertyTypeCustomizationUtils& StructCustomizationUtils)
{
	if (!StructPropertyHandle->IsValidHandle())
	{
		return;
	}
}

void FQuestPropertyDetails::OnQuestIdChanged()
{

}

void FQuestPropertyDetails::OnQuestIdPreChange()
{
}

void FQuestPropertyDetails::OnNodeIdChanged()
{
}

void FQuestPropertyDetails::OnNodeIdPreChange()
{
}

FText FQuestPropertyDetails::GetQuestIDComboBoxContentText() const
{
	return FText();
}
void FQuestPropertyDetails::GetQuestIDNames(TArray<FName>& OutRows) const
{
}


