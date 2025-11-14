// Copyright 2025 Ivan Chandra. All Rights Reserved.


#pragma once

#include "UI/OrionActivatableWidget.h"
#include "OrionGridContainerBase.generated.h"


// Grid container base class
UCLASS(Abstract, Blueprintable, meta = (DisableNativeTick))
class UOrionGridContainerBase : public UOrionActivatableWidget
{
	GENERATED_BODY()

public:
	// UUserWidget interface
	virtual void NativePreConstruct() override;
	// End of UUserWidget interface
public:
};
