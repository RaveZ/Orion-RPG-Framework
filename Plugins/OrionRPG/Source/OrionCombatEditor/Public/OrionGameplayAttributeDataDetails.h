// Copyright 2025 Ivan Chandra. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "IPropertyTypeCustomization.h"

class IPropertyHandle;
class IPropertyUtilities;

class FOrionGameplayAttributeDataDetails : public IPropertyTypeCustomization
{
public:
	static TSharedRef<IPropertyTypeCustomization> MakeInstance();

	virtual void CustomizeHeader(TSharedRef<IPropertyHandle> StructPropertyHandle, FDetailWidgetRow& HeaderRow, IPropertyTypeCustomizationUtils& StructCustomizationUtils) override;
	virtual void CustomizeChildren(TSharedRef<IPropertyHandle> StructPropertyHandle, IDetailChildrenBuilder& StructBuilder, IPropertyTypeCustomizationUtils& StructCustomizationUtils) override;

private:
	TOptional<float> GetBaseValue() const;
	TOptional<float> GetCurrentValue() const;
	void SetBaseValue(float NewValue);
	void SetCurrentValue(float NewValue);
	void SetAttributeValue(float NewValue, bool bSetBaseValue);

	TSharedPtr<IPropertyHandle> AttributeDataHandle;
	TWeakPtr<IPropertyUtilities> PropertyUtilities;
};
