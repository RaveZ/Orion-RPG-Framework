// Copyright 2025 Ivan Chandra. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "IPropertyTypeCustomization.h"

// Use struct instead of class to match the actual declaration
struct FGameplayAttributeData;

class IPropertyHandle;
class IPropertyUtilities;

class FOrionGameplayAttributeDataDetails : public IPropertyTypeCustomization
{
public:
	static TSharedRef<IPropertyTypeCustomization> MakeInstance();

	// IPropertyTypeCustomization interface
	virtual void CustomizeHeader(TSharedRef<IPropertyHandle> StructPropertyHandle, FDetailWidgetRow& HeaderRow, IPropertyTypeCustomizationUtils& StructCustomizationUtils) override;
	virtual void CustomizeChildren(TSharedRef<IPropertyHandle> StructPropertyHandle, IDetailChildrenBuilder& StructBuilder, IPropertyTypeCustomizationUtils& StructCustomizationUtils) override;

private:
	TOptional<float> GetBaseValue() const;
	TOptional<float> GetCurrentValue() const;
	
	FText GetBaseValueText() const;
	FText GetCurrentValueText() const;
	
	void SetBaseValue(float NewValue);
	
	FGameplayAttributeData* GetAttributeData() const;

private:
	TSharedPtr<IPropertyHandle> AttributeDataHandle;
	TWeakPtr<IPropertyUtilities> PropertyUtilities;
};