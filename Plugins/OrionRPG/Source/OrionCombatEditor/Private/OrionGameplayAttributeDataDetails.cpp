// Copyright 2025 Ivan Chandra. All Rights Reserved.

#include "OrionGameplayAttributeDataDetails.h"
#include "AttributeSet.h"
#include "DetailLayoutBuilder.h"  
#include "DetailWidgetRow.h"
#include "IDetailChildrenBuilder.h"
#include "IPropertyUtilities.h"
#include "PropertyHandle.h"
#include "ScopedTransaction.h"
#include "Widgets/Input/SNumericEntryBox.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/Layout/SGridPanel.h" 
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

	const FSlateFontInfo DisplayFont = FCoreStyle::GetDefaultFontStyle("Regular", 10);
	// Use SHorizontalBox with proper spacing instead of SVerticalBox
	HeaderRow
		.NameContent()
		[
			StructPropertyHandle->CreatePropertyNameWidget()
		]
		.ValueContent()
		.MinDesiredWidth(400.f)
		[
			SNew(SBox)
			[
				SNew(SVerticalBox)  // Actually SVerticalBox does exist, but if it doesn't, use SGridPanel
				
				// Display row for Base and Current values as text
				+ SVerticalBox::Slot()
				.AutoHeight()
				.Padding(0.f, 0.f, 0.f, 4.f)
				[
					SNew(SHorizontalBox)
					+ SHorizontalBox::Slot()
					.FillWidth(0.5f)
					[
						SNew(SHorizontalBox)
						+ SHorizontalBox::Slot()
						.AutoWidth()
						.Padding(0.f, 0.f, 4.f, 0.f)
						[
							SNew(STextBlock)
							.Text(LOCTEXT("BaseValueDisplayLabel", "Base:"))
							.Font(DisplayFont) 
						]
						+ SHorizontalBox::Slot()
						.AutoWidth()
						[
							SNew(STextBlock)
							.Text(this, &FOrionGameplayAttributeDataDetails::GetBaseValueText)
							.Font(DisplayFont)
						]
					]
					+ SHorizontalBox::Slot()
					.FillWidth(0.5f)
					[
						SNew(SHorizontalBox)
						+ SHorizontalBox::Slot()
						.AutoWidth()
						.Padding(0.f, 0.f, 4.f, 0.f)
						[
							SNew(STextBlock)
							.Text(LOCTEXT("CurrentValueDisplayLabel", "Current:"))
							.Font(DisplayFont)
						]
						+ SHorizontalBox::Slot()
						.AutoWidth()
						[
							SNew(STextBlock)
							.Text(this, &FOrionGameplayAttributeDataDetails::GetCurrentValueText)
							.Font(DisplayFont)
						]
					]
				]
				
				// Editable row for Base value only
				+ SVerticalBox::Slot()
				.AutoHeight()
				[
					SNew(SNumericEntryBox<float>)
					.LabelVAlign(VAlign_Center)
					.Label()
					[
						SNew(STextBlock)
						.Text(LOCTEXT("EditBaseLabel", "Base Value"))
					]
					.Value(this, &FOrionGameplayAttributeDataDetails::GetBaseValue)
					.OnValueCommitted_Lambda([this](float NewValue, ETextCommit::Type)
					{
						SetBaseValue(NewValue);
					})
				]
			]
		];
}

void FOrionGameplayAttributeDataDetails::CustomizeChildren(TSharedRef<IPropertyHandle> StructPropertyHandle, IDetailChildrenBuilder& StructBuilder, IPropertyTypeCustomizationUtils& StructCustomizationUtils)
{
	// Hide children (BaseValue and CurrentValue) since we're customizing them in the header
}

TOptional<float> FOrionGameplayAttributeDataDetails::GetBaseValue() const
{
	if (FGameplayAttributeData* AttributeData = GetAttributeData())
	{
		return AttributeData->GetBaseValue();
	}
	return TOptional<float>();
}

TOptional<float> FOrionGameplayAttributeDataDetails::GetCurrentValue() const
{
	if (FGameplayAttributeData* AttributeData = GetAttributeData())
	{
		return AttributeData->GetCurrentValue();
	}
	return TOptional<float>();
}

FText FOrionGameplayAttributeDataDetails::GetBaseValueText() const
{
	if (FGameplayAttributeData* AttributeData = GetAttributeData())
	{
		return FText::AsNumber(AttributeData->GetBaseValue());
	}
	return FText::GetEmpty();
}

FText FOrionGameplayAttributeDataDetails::GetCurrentValueText() const
{
	if (FGameplayAttributeData* AttributeData = GetAttributeData())
	{
		return FText::AsNumber(AttributeData->GetCurrentValue());
	}
	return FText::GetEmpty();
}

void FOrionGameplayAttributeDataDetails::SetBaseValue(float NewValue)
{
	if (!AttributeDataHandle.IsValid())
	{
		return;
	}

	const FScopedTransaction Transaction(LOCTEXT("SetBaseValueTransaction", "Set Attribute Base Value"));

	AttributeDataHandle->NotifyPreChange();

	TArray<void*> RawData;
	AttributeDataHandle->AccessRawData(RawData);
	if (!RawData.IsEmpty())
	{
		for (void* RawValue : RawData)
		{
			if (FGameplayAttributeData* AttributeData = static_cast<FGameplayAttributeData*>(RawValue))
			{
				AttributeData->SetBaseValue(NewValue);
				AttributeData->SetCurrentValue(NewValue); // Force Current to match Base
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

FGameplayAttributeData* FOrionGameplayAttributeDataDetails::GetAttributeData() const
{
	if (!AttributeDataHandle.IsValid())
	{
		return nullptr;
	}

	TArray<void*> RawData;
	AttributeDataHandle->AccessRawData(RawData);
	if (RawData.Num() == 1 && RawData[0])
	{
		return static_cast<FGameplayAttributeData*>(RawData[0]);
	}

	return nullptr;
}

#undef LOCTEXT_NAMESPACE