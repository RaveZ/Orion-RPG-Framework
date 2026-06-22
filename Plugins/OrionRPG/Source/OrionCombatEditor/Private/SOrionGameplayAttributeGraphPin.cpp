// Copyright 2025 Ivan Chandra. All Rights Reserved.

#include "SOrionGameplayAttributeGraphPin.h"

#include "AbilitySystem/Attributes/OrionAttributeSet.h"
#include "AbilitySystemComponent.h"
#include "AttributeSet.h"
#include "ScopedTransaction.h"
#include "Widgets/Input/SComboButton.h"
#include "Widgets/Input/SSearchBox.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/Text/STextBlock.h"
#include "Widgets/Views/SListView.h"
#include "Widgets/Views/STableRow.h"

#define LOCTEXT_NAMESPACE "OrionGameplayAttributeGraphPin"

struct FOrionGraphAttributeOption
{
	FOrionGraphAttributeOption(FProperty* InProperty, FString InDisplayName)
		: Property(InProperty)
		, DisplayName(MoveTemp(InDisplayName))
	{
	}

	FProperty* Property = nullptr;
	FString DisplayName;
};

class SOrionGameplayAttributePicker : public SCompoundWidget
{
public:
	DECLARE_DELEGATE_OneParam(FOnAttributePicked, FProperty*);

	SLATE_BEGIN_ARGS(SOrionGameplayAttributePicker) {}
		SLATE_ARGUMENT(FProperty*, DefaultProperty)
		SLATE_EVENT(FOnAttributePicked, OnAttributePicked)
	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs)
	{
		SelectedProperty = InArgs._DefaultProperty;
		OnAttributePicked = InArgs._OnAttributePicked;

		RefreshPropertyOptions();

		ChildSlot
		[
			SAssignNew(ComboButton, SComboButton)
			.OnGetMenuContent(this, &SOrionGameplayAttributePicker::GenerateAttributePicker)
			.ContentPadding(FMargin(2.f))
			.ToolTipText(this, &SOrionGameplayAttributePicker::GetSelectedAttributeText)
			.ButtonContent()
			[
				SNew(STextBlock)
				.Text(this, &SOrionGameplayAttributePicker::GetSelectedAttributeText)
			]
		];
	}

private:
	static FString GetFormattedClassName(const UClass* Class)
	{
		if (!Class)
		{
			return FString();
		}

		FString ClassName = Class->GetName();
		if (ClassName.EndsWith(TEXT("_C")))
		{
			ClassName.LeftChopInline(2);
		}

		return ClassName;
	}

	static bool ShouldIncludeAttributeClass(const UClass* Class)
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

		return (bNativeAttributeSet || bOrionBlueprintAttributeSet || bNativeAbilitySystemComponent)
			&& !Class->HasMetaData(TEXT("HideInDetailsView"));
	}

	static bool ShouldIncludeAttributeProperty(const FProperty* Property)
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

		return !Property->HasMetaData(TEXT("HideInDetailsView"));
	}

	void RefreshPropertyOptions()
	{
		PropertyOptions.Empty();
		PropertyOptions.Add(MakeShared<FOrionGraphAttributeOption>(nullptr, TEXT("None")));

		for (TObjectIterator<UClass> ClassIt; ClassIt; ++ClassIt)
		{
			UClass* Class = *ClassIt;
			if (!ShouldIncludeAttributeClass(Class))
			{
				continue;
			}

			const FString ClassName = GetFormattedClassName(Class);
			for (TFieldIterator<FProperty> PropertyIt(Class, EFieldIteratorFlags::ExcludeSuper); PropertyIt; ++PropertyIt)
			{
				FProperty* Property = *PropertyIt;
				if (!ShouldIncludeAttributeProperty(Property))
				{
					continue;
				}

				const FString DisplayName = FString::Printf(TEXT("%s.%s"), *ClassName, *Property->GetName());
				if (!SearchText.IsEmpty() && !DisplayName.Contains(SearchText.ToString(), ESearchCase::IgnoreCase))
				{
					continue;
				}

				PropertyOptions.Add(MakeShared<FOrionGraphAttributeOption>(Property, DisplayName));
			}
		}
	}

	TSharedRef<SWidget> GenerateAttributePicker()
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
					.OnTextChanged(this, &SOrionGameplayAttributePicker::OnFilterTextChanged)
					.DelayChangeNotificationsWhileTyping(true)
				]
				+ SVerticalBox::Slot()
				.FillHeight(1.f)
				[
					SAssignNew(AttributeList, SListView<TSharedPtr<FOrionGraphAttributeOption>>)
					.ListItemsSource(&PropertyOptions)
					.SelectionMode(ESelectionMode::Single)
					.OnGenerateRow(this, &SOrionGameplayAttributePicker::GenerateAttributeRow)
					.OnSelectionChanged(this, &SOrionGameplayAttributePicker::OnAttributeSelected)
				]
			];
	}

	TSharedRef<ITableRow> GenerateAttributeRow(TSharedPtr<FOrionGraphAttributeOption> Item, const TSharedRef<STableViewBase>& OwnerTable) const
	{
		return SNew(STableRow<TSharedPtr<FOrionGraphAttributeOption>>, OwnerTable)
			[
				SNew(STextBlock)
				.Text(Item.IsValid() ? FText::FromString(Item->DisplayName) : FText::GetEmpty())
				.HighlightText(SearchText)
			];
	}

	void OnFilterTextChanged(const FText& InFilterText)
	{
		SearchText = InFilterText;
		RefreshPropertyOptions();

		if (AttributeList.IsValid())
		{
			AttributeList->RequestListRefresh();
		}
	}

	void OnAttributeSelected(TSharedPtr<FOrionGraphAttributeOption> Item, ESelectInfo::Type SelectInfo)
	{
		if (!Item.IsValid() || SelectInfo == ESelectInfo::Direct)
		{
			return;
		}

		SelectedProperty = Item->Property;
		OnAttributePicked.ExecuteIfBound(SelectedProperty);

		if (ComboButton.IsValid())
		{
			ComboButton->SetIsOpen(false);
		}
	}

	FText GetSelectedAttributeText() const
	{
		if (SelectedProperty)
		{
			if (const UClass* OwnerClass = SelectedProperty->GetOwnerClass())
			{
				return FText::FromString(FString::Printf(TEXT("%s.%s"), *GetFormattedClassName(OwnerClass), *SelectedProperty->GetName()));
			}
		}

		return LOCTEXT("NoAttributeSelected", "None");
	}

	FProperty* SelectedProperty = nullptr;
	FOnAttributePicked OnAttributePicked;
	TSharedPtr<SComboButton> ComboButton;
	TSharedPtr<SSearchBox> SearchBox;
	TSharedPtr<SListView<TSharedPtr<FOrionGraphAttributeOption>>> AttributeList;
	TArray<TSharedPtr<FOrionGraphAttributeOption>> PropertyOptions;
	FText SearchText;
};

void SOrionGameplayAttributeGraphPin::Construct(const FArguments& InArgs, UEdGraphPin* InGraphPinObj)
{
	SGraphPin::Construct(SGraphPin::FArguments(), InGraphPinObj);
}

TSharedRef<SWidget> SOrionGameplayAttributeGraphPin::GetDefaultValueWidget()
{
	FGameplayAttribute DefaultAttribute;

	const FString DefaultString = GraphPinObj->GetDefaultAsString();
	if (!DefaultString.IsEmpty())
	{
		UScriptStruct* PinLiteralStructType = FGameplayAttribute::StaticStruct();
		PinLiteralStructType->ImportText(*DefaultString, &DefaultAttribute, nullptr, EPropertyPortFlags::PPF_SerializedAsImportText, GError, PinLiteralStructType->GetName(), true);
	}

	return SNew(SOrionGameplayAttributePicker)
		.DefaultProperty(DefaultAttribute.GetUProperty())
		.OnAttributePicked(this, &SOrionGameplayAttributeGraphPin::OnAttributeChanged)
		.Visibility(this, &SGraphPin::GetDefaultValueVisibility)
		.IsEnabled(this, &SOrionGameplayAttributeGraphPin::GetDefaultValueIsEnabled);
}

void SOrionGameplayAttributeGraphPin::OnAttributeChanged(FProperty* SelectedAttribute)
{
	FString FinalValue;
	FGameplayAttribute NewAttributeStruct;
	NewAttributeStruct.SetUProperty(SelectedAttribute);

	FGameplayAttribute::StaticStruct()->ExportText(FinalValue, &NewAttributeStruct, &NewAttributeStruct, nullptr, EPropertyPortFlags::PPF_SerializedAsImportText, nullptr);

	if (FinalValue != GraphPinObj->GetDefaultAsString())
	{
		const FScopedTransaction Transaction(NSLOCTEXT("GraphEditor", "ChangePinValue", "Change Pin Value"));
		GraphPinObj->Modify();
		GraphPinObj->GetSchema()->TrySetDefaultValue(*GraphPinObj, FinalValue);
	}
}

#undef LOCTEXT_NAMESPACE
