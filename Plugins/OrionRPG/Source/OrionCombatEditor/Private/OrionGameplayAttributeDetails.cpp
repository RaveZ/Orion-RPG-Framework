// Copyright 2025 Ivan Chandra. All Rights Reserved.

#include "OrionGameplayAttributeDetails.h"

#include "AbilitySystem/Attributes/OrionAttributeSet.h"
#include "AbilitySystemComponent.h"
#include "AssetRegistry/AssetData.h"
#include "AttributeSet.h"
#include "DetailWidgetRow.h"
#include "Engine/Blueprint.h"
#include "Engine/BlueprintGeneratedClass.h"
#include "IDetailChildrenBuilder.h"
#include "PropertyHandle.h"
#include "ScopedTransaction.h"
#include "Widgets/Input/SComboButton.h"
#include "Widgets/Input/SSearchBox.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/Text/STextBlock.h"
#include "Widgets/Views/SListView.h"
#include "Widgets/Views/STableRow.h"

#define LOCTEXT_NAMESPACE "OrionGameplayAttributeDetails"

struct FOrionGameplayAttributeOption
{
	FOrionGameplayAttributeOption(FProperty* InProperty, FString InDisplayName)
		: Property(InProperty)
		, DisplayName(MoveTemp(InDisplayName))
	{
	}

	FProperty* Property = nullptr;
	FString DisplayName;
};

TSharedRef<IPropertyTypeCustomization> FOrionGameplayAttributeDetails::MakeInstance()
{
	return MakeShared<FOrionGameplayAttributeDetails>();
}

void FOrionGameplayAttributeDetails::CustomizeHeader(TSharedRef<IPropertyHandle> StructPropertyHandle, FDetailWidgetRow& HeaderRow, IPropertyTypeCustomizationUtils& StructCustomizationUtils)
{
	AttributePropertyHandle = StructPropertyHandle->GetChildHandle(TEXT("Attribute"));
	OwnerPropertyHandle = StructPropertyHandle->GetChildHandle(TEXT("AttributeOwner"));
	NamePropertyHandle = StructPropertyHandle->GetChildHandle(TEXT("AttributeName"));
	FilterMetaData = StructPropertyHandle->GetProperty()->GetMetaData(TEXT("FilterMetaTag"));

	RefreshPropertyOptions();

	HeaderRow
		.NameContent()
		[
			StructPropertyHandle->CreatePropertyNameWidget()
		]
		.ValueContent()
		.MinDesiredWidth(360.f)
		.MaxDesiredWidth(4096.f)
		[
			SAssignNew(ComboButton, SComboButton)
			.OnGetMenuContent(this, &FOrionGameplayAttributeDetails::GenerateAttributePicker)
			.ContentPadding(FMargin(2.f))
			.ButtonContent()
			[
				SNew(STextBlock)
				.Text(this, &FOrionGameplayAttributeDetails::GetSelectedAttributeText)
			]
		];
}

void FOrionGameplayAttributeDetails::CustomizeChildren(TSharedRef<IPropertyHandle> StructPropertyHandle, IDetailChildrenBuilder& StructBuilder, IPropertyTypeCustomizationUtils& StructCustomizationUtils)
{
}

FString FOrionGameplayAttributeDetails::GetFormattedClassName(const UClass* Class) const
{
	if (!Class)
	{
		return FString();
	}

	FString ClassName = Class->GetName();
	
	// Remove _C suffix for Blueprint classes
	if (ClassName.EndsWith(TEXT("_C")))
	{
		ClassName = ClassName.Left(ClassName.Len() - 2);
	}
	
	// Check if this is a Blueprint generated class and get its display name if available
	if (const UBlueprintGeneratedClass* BPClass = Cast<const UBlueprintGeneratedClass>(Class))
	{
		if (UBlueprint* Blueprint = Cast<UBlueprint>(BPClass->ClassGeneratedBy))
		{
			if (Blueprint->GeneratedClass)
			{
				// Check for DisplayName metadata on the generated class
				const FString DisplayName = Blueprint->GeneratedClass->GetMetaData(TEXT("DisplayName"));
				if (!DisplayName.IsEmpty())
				{
					return DisplayName;
				}
			}
			
		}
	}
	
	return ClassName;
}

FString FOrionGameplayAttributeDetails::GetFormattedPropertyName(const FProperty* Property) const
{
	if (!Property)
	{
		return FString();
	}
	
	// Check for DisplayName metadata on the property
	const FString DisplayName = Property->GetMetaData(TEXT("DisplayName"));
	if (!DisplayName.IsEmpty())
	{
		return DisplayName;
	}
	
	return Property->GetName();
}

void FOrionGameplayAttributeDetails::RefreshPropertyOptions()
{
	PropertyOptions.Empty();
	PropertyOptions.Add(MakeShared<FOrionGameplayAttributeOption>(nullptr, TEXT("None")));

	for (TObjectIterator<UClass> ClassIt; ClassIt; ++ClassIt)
	{
		UClass* Class = *ClassIt;
		if (!ShouldIncludeAttributeClass(Class))
		{
			continue;
		}

		const FString FormattedClassName = GetFormattedClassName(Class);
		
		for (TFieldIterator<FProperty> PropertyIt(Class, EFieldIteratorFlags::ExcludeSuper); PropertyIt; ++PropertyIt)
		{
			FProperty* Property = *PropertyIt;
			if (!ShouldIncludeAttributeProperty(Property))
			{
				continue;
			}

			const FString FormattedPropertyName = GetFormattedPropertyName(Property);
			const FString DisplayName = FString::Printf(TEXT("%s.%s"), *FormattedClassName, *FormattedPropertyName);
			
			if (!SearchText.IsEmpty() && !DisplayName.Contains(SearchText.ToString(), ESearchCase::IgnoreCase))
			{
				// Also check against raw class name for search
				const FString RawDisplayName = FString::Printf(TEXT("%s.%s"), *Class->GetName(), *Property->GetName());
				if (!RawDisplayName.Contains(SearchText.ToString(), ESearchCase::IgnoreCase))
				{
					continue;
				}
			}

			PropertyOptions.Add(MakeShared<FOrionGameplayAttributeOption>(Property, DisplayName));
		}
	}
}

bool FOrionGameplayAttributeDetails::ShouldIncludeAttributeClass(const UClass* Class) const
{
	if (!Class)
	{
		return false;
	}

	if (Class->HasAnyClassFlags(CLASS_Deprecated | CLASS_NewerVersionExists))
	{
		return false;
	}

	if (Class->GetName().StartsWith(TEXT("SKEL_")) || Class->GetName().StartsWith(TEXT("REINST_")))
	{
		return false;
	}

	const bool bNativeAttributeSet = Class->IsChildOf(UAttributeSet::StaticClass()) && !Class->ClassGeneratedBy;
	const bool bOrionBlueprintAttributeSet = Class->IsChildOf(UOrionAttributeSet::StaticClass()) && Class->ClassGeneratedBy;
	const bool bNativeAbilitySystemComponent = Class->IsChildOf(UAbilitySystemComponent::StaticClass()) && !Class->ClassGeneratedBy;

	if (!(bNativeAttributeSet || bOrionBlueprintAttributeSet || bNativeAbilitySystemComponent))
	{
		return false;
	}

	return !Class->HasMetaData(TEXT("HideInDetailsView"));
}

bool FOrionGameplayAttributeDetails::ShouldIncludeAttributeProperty(const FProperty* Property) const
{
	if (!Property)
	{
		return false;
	}

	const UClass* OwnerClass = Property->GetOwnerClass();
	if (OwnerClass && OwnerClass->IsChildOf(UAbilitySystemComponent::StaticClass()))
	{
		if (!Property->HasMetaData(TEXT("SystemGameplayAttribute")))
		{
			return false;
		}
	}
	else if (!CastField<FNumericProperty>(Property) && !FGameplayAttribute::IsGameplayAttributeDataProperty(Property))
	{
		return false;
	}

	if (!FilterMetaData.IsEmpty() && Property->HasMetaData(*FilterMetaData))
	{
		return false;
	}

	return !Property->HasMetaData(TEXT("HideInDetailsView"));
}

TSharedRef<SWidget> FOrionGameplayAttributeDetails::GenerateAttributePicker()
{
	RefreshPropertyOptions();

	return SNew(SBox)
		.WidthOverride(360.f)
		.MaxDesiredHeight(500.f)
		[
			SNew(SVerticalBox)
			+ SVerticalBox::Slot()
			.AutoHeight()
			.Padding(0.f, 0.f, 0.f, 4.f)
			[
				SAssignNew(SearchBox, SSearchBox)
				.HintText(LOCTEXT("SearchAttributesHint", "Search Attributes"))
				.OnTextChanged(this, &FOrionGameplayAttributeDetails::OnFilterTextChanged)
				.DelayChangeNotificationsWhileTyping(true)
			]
			+ SVerticalBox::Slot()
			.FillHeight(1.f)
			[
				SAssignNew(AttributeList, SListView<TSharedPtr<FOrionGameplayAttributeOption>>)
				.ListItemsSource(&PropertyOptions)
				.SelectionMode(ESelectionMode::Single)
				.OnGenerateRow(this, &FOrionGameplayAttributeDetails::GenerateAttributeRow)
				.OnSelectionChanged(this, &FOrionGameplayAttributeDetails::OnAttributeSelected)
			]
		];
}

TSharedRef<ITableRow> FOrionGameplayAttributeDetails::GenerateAttributeRow(TSharedPtr<FOrionGameplayAttributeOption> Item, const TSharedRef<STableViewBase>& OwnerTable) const
{
	return SNew(STableRow<TSharedPtr<FOrionGameplayAttributeOption>>, OwnerTable)
		[
			SNew(STextBlock)
			.Text(Item.IsValid() ? FText::FromString(Item->DisplayName) : FText::GetEmpty())
			.HighlightText(SearchText)
		];
}

void FOrionGameplayAttributeDetails::OnFilterTextChanged(const FText& InFilterText)
{
	SearchText = InFilterText;
	RefreshPropertyOptions();

	if (AttributeList.IsValid())
	{
		AttributeList->RequestListRefresh();
	}
}

void FOrionGameplayAttributeDetails::OnAttributeSelected(TSharedPtr<FOrionGameplayAttributeOption> Item, ESelectInfo::Type SelectInfo)
{
	if (!Item.IsValid() || SelectInfo == ESelectInfo::Direct)
	{
		return;
	}

	const FScopedTransaction Transaction(LOCTEXT("SetGameplayAttributeTransaction", "Set Gameplay Attribute"));
	FProperty* SelectedAttribute = Item->Property;

	if (AttributePropertyHandle.IsValid())
	{
		AttributePropertyHandle->SetValue(SelectedAttribute);
	}

	if (OwnerPropertyHandle.IsValid())
	{
		OwnerPropertyHandle->SetValue(SelectedAttribute ? SelectedAttribute->GetOwnerStruct() : nullptr);
	}

	if (NamePropertyHandle.IsValid())
	{
		FString AttributeName;
		if (SelectedAttribute)
		{
			SelectedAttribute->GetName(AttributeName);
		}
		NamePropertyHandle->SetValue(AttributeName);
	}

	if (ComboButton.IsValid())
	{
		ComboButton->SetIsOpen(false);
	}
}

FProperty* FOrionGameplayAttributeDetails::GetSelectedAttribute() const
{
	if (AttributePropertyHandle.IsValid())
	{
		FProperty* PropertyValue = nullptr;
		if (AttributePropertyHandle->GetValue(PropertyValue) == FPropertyAccess::Success)
		{
			return PropertyValue;
		}
	}

	return nullptr;
}

FText FOrionGameplayAttributeDetails::GetSelectedAttributeText() const
{
	if (FProperty* SelectedAttribute = GetSelectedAttribute())
	{
		if (const UClass* OwnerClass = SelectedAttribute->GetOwnerClass())
		{
			const FString FormattedClassName = GetFormattedClassName(OwnerClass);
			const FString FormattedPropertyName = GetFormattedPropertyName(SelectedAttribute);
			return FText::FromString(FString::Printf(TEXT("%s.%s"), *FormattedClassName, *FormattedPropertyName));
		}
	}

	return LOCTEXT("NoAttributeSelected", "None");
}

#undef LOCTEXT_NAMESPACE