// Copyright 2025 Ivan Chandra. All Rights Reserved.

#pragma once

#include "AttributeSet.h"
#include "EdGraphSchema_K2.h"
#include "EdGraphUtilities.h"
#include "SOrionGameplayAttributeGraphPin.h"

class FOrionGameplayAttributeGraphPanelPinFactory : public FGraphPanelPinFactory
{
public:
	virtual TSharedPtr<SGraphPin> CreatePin(UEdGraphPin* InPin) const override
	{
		if (InPin->PinType.PinCategory == UEdGraphSchema_K2::PC_Struct && InPin->PinType.PinSubCategoryObject == FGameplayAttribute::StaticStruct())
		{
			return SNew(SOrionGameplayAttributeGraphPin, InPin);
		}

		return nullptr;
	}
};
