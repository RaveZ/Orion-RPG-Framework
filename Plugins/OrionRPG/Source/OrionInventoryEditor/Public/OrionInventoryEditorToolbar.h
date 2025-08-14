// Copyright 2025 Ivan Chandra. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

class FOrionInventoryEditor;
class FExtender;
class FToolBarBuilder;

class ORIONINVENTORYEDITOR_API FOrionInventoryEditorToolbar : public TSharedFromThis<FOrionInventoryEditorToolbar>
{
public:
	FOrionInventoryEditorToolbar(TSharedPtr<FOrionInventoryEditor> InOrionInventoryEditor)
		: InventorySystemEditor(InOrionInventoryEditor) {}

	void AddInventorySystemToolbar(TSharedPtr<FExtender> Extender);

private:
	void FillInventorySystemToolbar(FToolBarBuilder& ToolbarBuilder);

protected:
	/** Pointer back to the blueprint editor tool that owns us */
	TWeakPtr<FOrionInventoryEditor> InventorySystemEditor;
};
