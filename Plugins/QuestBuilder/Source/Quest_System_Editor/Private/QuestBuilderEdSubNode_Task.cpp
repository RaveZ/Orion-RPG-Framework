// Copyright 2025 Ivan Chandra. All Rights Reserved.


#include "QuestBuilderEdSubNode_Task.h"
#include "QuestBuilderSetting.h"
#include "Task/QuestTask.h"

UQuestBuilderEdSubNode_Task::UQuestBuilderEdSubNode_Task()
{
}

FText UQuestBuilderEdSubNode_Task::GetNodeTitle(ENodeTitleType::Type TitleType) const
{
	UQuestTask* Task = Cast<UQuestTask>(NodeInstance);
	if (Task != NULL)
	{
		return FText::FromString(Task->GetNodeName());
	}
	else if (!ClassData.GetClassName().IsEmpty())
	{
		FString StoredClassName = ClassData.GetClassName();
		StoredClassName.RemoveFromEnd(TEXT("_C"));

		return FText::Format(NSLOCTEXT("QuestGraph", "NodeClassError", "Class {0} not found, make sure it's saved!"), FText::FromString(StoredClassName));
	}

	return Super::GetNodeTitle(TitleType);
}

FName UQuestBuilderEdSubNode_Task::GetNameIcon() const
{
    return FName("BTEditor.Graph.NewTask");
}

FText UQuestBuilderEdSubNode_Task::GetDescription() const
{
	UQuestTask* Task = Cast<UQuestTask>(NodeInstance);
	if (Task)
	{
		FString Desc = Task->GetDescription().ToString();
		if (Task->bUseAmount)
		{
			Desc += " " + FString::Printf(TEXT("(0/%d)"), Task->RequiredAmount);
		}
		return FText::FromString(Desc);
	}

	return FText();
}
