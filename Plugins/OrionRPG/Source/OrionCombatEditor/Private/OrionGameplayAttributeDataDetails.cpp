// Copyright 2025 Ivan Chandra. All Rights Reserved.

#include "OrionGameplayAttributeDataDetails.h"

#include "AttributeSet.h"
#include "DetailWidgetRow.h"
#include "IDetailChildrenBuilder.h"
#include "IPropertyUtilities.h"
#include "PropertyHandle.h"
#include "ScopedTransaction.h"
#include "Widgets/Input/SNumericEntryBox.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/Text/STextBlock.h"

#define LOCTEXT_NAMESPACE "OrionGameplayAttributeDataDetails"

TSharedRef<IPropertyTypeCustomization> FOrionGameplayAttributeDataDetails::MakeInstance()
{
	return MakeShared<FOrionGameplayAttributeDataDetails>();
}

void FOrionGameplayAttributeDataDetails::CustomizeHeader(TSharedRef<IPropertyHandle> StructPropertyHandle, FDetailWidgetRow& HeaderRow, IPropertyTypeCustomizationUtils& StructCustomizationUtils)
{
	AttributeDataHandle = StructPropertyHandle;
	PropertyUtilities = StructCustomizationUtils.GetPropertyUtilities();

	HeaderRow
		.NameContent()
		[
			StructPropertyHandle->CreatePropertyNameWidget()
		]
		.ValueContent()
		.MinDesiredWidth(360.f)
		[
			SNew(SHorizontalBox)
			+ SHorizontalBox::Slot()
			.FillWidth(0.5f)
			.Padding(0.f, 0.f, 4.f, 0.f)
			[
				SNew(SBox)
				.ToolTipText(LOCTEXT("BaseValueToolTip", "The permanent base value used when the AttributeSet default object is initialized."))
				[
					SNew(SNumericEntryBox<float>)
					.LabelVAlign(VAlign_Center)
					.Label()
					[
						SNew(STextBlock)
						.Text(LOCTEXT("BaseValueLabel", "Base"))
					]
					.Value(this, &FOrionGameplayAttributeDataDetails::GetBaseValue)
					.OnValueCommitted_Lambda([this](float NewValue, ETextCommit::Type)
					{
						SetBaseValue(NewValue);
					})
				]
			]
			+ SHorizontalBox::Slot()
			.FillWidth(0.5f)
			[
				SNew(SBox)
				.ToolTipText(LOCTEXT("CurrentValueToolTip", "The starting current value. Usually this should match Base unless you intentionally want a different initial current value."))
				[
					SNew(SNumericEntryBox<float>)
					.LabelVAlign(VAlign_Center)
					.Label()
					[
						SNew(STextBlock)
						.Text(LOCTEXT("CurrentValueLabel", "Current"))
					]
					.Value(this, &FOrionGameplayAttributeDataDetails::GetCurrentValue)
					.OnValueCommitted_Lambda([this](float NewValue, ETextCommit::Type)
					{
						SetCurrentValue(NewValue);
					})
				]
			]
		];
}

void FOrionGameplayAttributeDataDetails::CustomizeChildren(TSharedRef<IPropertyHandle> StructPropertyHandle, IDetailChildrenBuilder& StructBuilder, IPropertyTypeCustomizationUtils& StructCustomizationUtils)
{
}

TOptional<float> FOrionGameplayAttributeDataDetails::GetBaseValue() const
{
	TArray<void*> RawData;
	if (AttributeDataHandle.IsValid())
	{
		AttributeDataHandle->AccessRawData(RawData);
		if (RawData.Num() == 1 && RawData[0])
		{
			return static_cast<FGameplayAttributeData*>(RawData[0])->GetBaseValue();
		}
	}

	return TOptional<float>();
}

TOptional<float> FOrionGameplayAttributeDataDetails::GetCurrentValue() const
{
	TArray<void*> RawData;
	if (AttributeDataHandle.IsValid())
	{
		AttributeDataHandle->AccessRawData(RawData);
		if (RawData.Num() == 1 && RawData[0])
		{
			return static_cast<FGameplayAttributeData*>(RawData[0])->GetCurrentValue();
		}
	}

	return TOptional<float>();
}

void FOrionGameplayAttributeDataDetails::SetBaseValue(float NewValue)
{
	SetAttributeValue(NewValue, true);
}

void FOrionGameplayAttributeDataDetails::SetCurrentValue(float NewValue)
{
	SetAttributeValue(NewValue, false);
}

void FOrionGameplayAttributeDataDetails::SetAttributeValue(float NewValue, bool bSetBaseValue)
{
	if (!AttributeDataHandle.IsValid())
	{
		return;
	}

	const FScopedTransaction Transaction(bSetBaseValue
		? LOCTEXT("SetBaseValueTransaction", "Set Attribute Base Value")
		: LOCTEXT("SetCurrentValueTransaction", "Set Attribute Current Value"));

	AttributeDataHandle->NotifyPreChange();

	TArray<void*> RawData;
	AttributeDataHandle->AccessRawData(RawData);
	if (!RawData.IsEmpty())
	{
		for (void* RawValue : RawData)
		{
			if (FGameplayAttributeData* AttributeData = static_cast<FGameplayAttributeData*>(RawValue))
			{
				if (bSetBaseValue)
				{
					AttributeData->SetBaseValue(NewValue);
				}
				else
				{
					AttributeData->SetCurrentValue(NewValue);
				}
			}
		}
	}

	AttributeDataHandle->NotifyPostChange(EPropertyChangeType::ValueSet);
	AttributeDataHandle->NotifyFinishedChangingProperties();

	if (const TSharedPtr<IPropertyUtilities> Utilities = PropertyUtilities.Pin())
	{
		Utilities->ForceRefresh();
	}
}

#undef LOCTEXT_NAMESPACE
