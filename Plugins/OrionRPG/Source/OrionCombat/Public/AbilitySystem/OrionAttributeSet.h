// Copyright 2025 Ivan Chandra. All Rights Reserved.

#pragma once

#include "AttributeSet.h"
#include "OrionAttributeSet.generated.h"

/**
 * Base attribute set class for Orion combat module.
 */
UCLASS(BlueprintType, Blueprintable)
class ORIONCOMBAT_API UOrionAttributeSet : public UAttributeSet
{
	GENERATED_BODY()

public:
	UOrionAttributeSet();
};
