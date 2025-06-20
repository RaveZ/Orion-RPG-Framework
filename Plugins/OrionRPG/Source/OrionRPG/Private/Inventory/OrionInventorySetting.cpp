// Copyright 2025 Ivan Chandra. All Rights Reserved.


#include "Inventory/OrionInventorySetting.h"
#include "UObject/ConstructorHelpers.h"

UOrionInventorySetting::UOrionInventorySetting()
{
	ItemCategoryColor = FLinearColor(0.15f, 0.15f, 0.15f);
	bCanRenameNode = true;
}


UOrionInventorySetting::~UOrionInventorySetting()
{
}
