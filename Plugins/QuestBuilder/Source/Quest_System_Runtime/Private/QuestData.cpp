// Copyright 2024 Ivan Chandra. All Rights Reserved.

#include "QuestData.h"
#include "Quest_System_Runtime.h"


void FQuestDataHandle::PostSerialize(const FArchive& Ar)
{
	if (Ar.IsSaving() && !IsNull())
	{
		// Note which row we are pointing to for later searching
		FQuest_System_RuntimeModule& RuntimeModule = FModuleManager::GetModuleChecked<FQuest_System_RuntimeModule>(TEXT("Quest_System_Runtime"));
		RuntimeModule.CachedQuestID = QuestID;
	}
}
