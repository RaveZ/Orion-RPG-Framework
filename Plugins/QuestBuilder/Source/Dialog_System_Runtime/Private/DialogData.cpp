// Copyright 2025 Ivan Chandra. All Rights Reserved.

#include "DialogData.h"
#include "Dialog_System_Runtime.h"


void FDialogDataHandle::PostSerialize(const FArchive& Ar)
{
	if (Ar.IsSaving() && !IsNull())
	{
		FDialog_System_RuntimeModule& RuntimeModule = FModuleManager::GetModuleChecked<FDialog_System_RuntimeModule>(TEXT("Dialog_System_Runtime"));
		//RuntimeModule.CachedDialogID = ID;
	}
}
