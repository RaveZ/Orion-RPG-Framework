// Copyright 2024 Ivan Chandra. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

class FDialogBuilderEditor;
class FExtender;
class FToolBarBuilder;

class DIALOG_SYSTEM_EDITOR_API FDialogBuilderEditorToolbar : public TSharedFromThis<FDialogBuilderEditorToolbar>
{
public:
	FDialogBuilderEditorToolbar(TSharedPtr<FDialogBuilderEditor> InQuestBuilderEditor)
		: QuestSystemEditor(InQuestBuilderEditor) {}

	void AddQuestSystemToolbar(TSharedPtr<FExtender> Extender);

private:
	void FillQuestSystemToolbar(FToolBarBuilder& ToolbarBuilder);

protected:
	/** Pointer back to the blueprint editor tool that owns us */
	TWeakPtr<FDialogBuilderEditor> QuestSystemEditor;
};
