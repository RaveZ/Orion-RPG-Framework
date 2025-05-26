// Copyright 2025 Ivan Chandra. All Rights Reserved.


#include "DialogBuilderNode_Speaker.h"
#include "Dialog.h"
#include "DialogComponent.h"


#define LOCTEXT_NAMESPACE "DialogSystemGraphNode"

void UDialogBuilderNode_Speaker::Initialize()
{
}

void UDialogBuilderNode_Speaker::Deinitialize()
{
}


#if WITH_EDITOR
FText UDialogBuilderNode_Speaker::GetNodeTitle() const
{
    return FText::FromName(ID).IsEmpty() ? FText::FromString(UDialogBuilderNode::GetShortTypeName(this)) : FText::FromName(ID);
}

void UDialogBuilderNode_Speaker::SetNodeTitle(const FText& NewTitle)
{
    ID = FName(NewTitle.ToString());
}

FText UDialogBuilderNode_Speaker::GetNodeDescription() const
{
    /*FString TaskDesc;

    for (const UDialogTask* Task : Tasks)
    {
        if (Task)
        {
            if(!TaskDesc.IsEmpty()) TaskDesc += TEXT("\n");
            TaskDesc += Task->GetDescription().ToString();
        }
        
    }

    return FText::FromString(TaskDesc);*/
    return Description;
}
#endif

#undef LOCTEXT_NAMESPACE