// Copyright 2025 Ivan Chandra. All Rights Reserved.


#include "Inventory/OrionInventorySetting.h"
#include "UObject/ConstructorHelpers.h"

UOrionInventorySetting::UOrionInventorySetting()
{
	ItemContainerColor = FLinearColor(0.07f, 0.07f, 0.07f);
	bCanRenameNode = true;
}


UOrionInventorySetting::~UOrionInventorySetting()
{
}
