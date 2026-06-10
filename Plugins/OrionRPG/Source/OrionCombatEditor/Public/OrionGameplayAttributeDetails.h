// Copyright 2025 Ivan Chandra. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "IPropertyTypeCustomization.h"
#include "Widgets/Views/SListView.h"
#include "Widgets/Views/STableRow.h"

class IPropertyHandle;
struct FOrionGameplayAttributeOption;

class FOrionGameplayAttributeDetails : public IPropertyTypeCustomization
{
public:
	static TSharedRef<IPropertyTypeCustomization> MakeInstance();

	virtual void CustomizeHeader(TSharedRef<IPropertyHandle> StructPropertyHandle, FDetailWidgetRow& HeaderRow, IPropertyTypeCustomizationUtils& StructCustomizationUtils) override;
	virtual void CustomizeChildren(TSharedRef<IPropertyHandle> StructPropertyHandle, IDetailChildrenBuilder& StructBuilder, IPropertyTypeCustomizationUtils& StructCustomizationUtils) override;

private:
	void RefreshPropertyOptions();
	TSharedRef<SWidget> GenerateAttributePicker();
	TSharedRef<ITableRow> GenerateAttributeRow(TSharedPtr<FOrionGameplayAttributeOption> Item, const TSharedRef<STableViewBase>& OwnerTable) const;
	void OnFilterTextChanged(const FText& InFilterText);
	void OnAttributeSelected(TSharedPtr<FOrionGameplayAttributeOption> Item, ESelectInfo::Type SelectInfo);
	FText GetSelectedAttributeText() const;
	FProperty* GetSelectedAttribute() const;
	bool ShouldIncludeAttributeProperty(const FProperty* Property) const;
	bool ShouldIncludeAttributeClass(const UClass* Class) const;

	TSharedPtr<IPropertyHandle> AttributePropertyHandle;
	TSharedPtr<IPropertyHandle> OwnerPropertyHandle;
	TSharedPtr<IPropertyHandle> NamePropertyHandle;
	TSharedPtr<class SComboButton> ComboButton;
	TSharedPtr<class SSearchBox> SearchBox;
	TSharedPtr<class SListView<TSharedPtr<FOrionGameplayAttributeOption>>> AttributeList;
	TArray<TSharedPtr<FOrionGameplayAttributeOption>> PropertyOptions;
	FString FilterMetaData;
	FText SearchText;
};
