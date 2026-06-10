// Copyright 2025 Ivan Chandra. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "IDetailCustomization.h"

class IDetailLayoutBuilder;

class FOrionAttributeSetDetails : public IDetailCustomization
{
public:
	static TSharedRef<IDetailCustomization> MakeInstance();

	virtual void CustomizeDetails(IDetailLayoutBuilder& DetailBuilder) override;

private:
	void AddGameplayAttribute();
	bool CanAddGameplayAttribute() const;
	FText GetAddGameplayAttributeToolTip() const;

	TWeakObjectPtr<UBlueprint> AttributeSetBlueprint;
	TWeakPtr<class IPropertyUtilities> PropertyUtilities;
};
