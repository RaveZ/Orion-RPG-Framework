// Copyright 2025 Ivan Chandra. All Rights Reserved.


#include "OrionInventoryEditorToolbar.h"
#include "OrionInventory_EditorCommands.h"
#include "OrionInventory_EditorStyle.h"
#include "OrionInventoryEditor.h"
#include "OrionInventoryEditorModule.h"

#define LOCTEXT_NAMESPACE "OrionInventoryEditorToolbar"
void FOrionInventoryEditorToolbar::AddInventorySystemToolbar(TSharedPtr<FExtender> Extender)
{
	check(InventorySystemEditor.IsValid());
	TSharedPtr<FOrionInventoryEditor> InventorySystemEditorPtr = InventorySystemEditor.Pin();
	
	TSharedPtr<FExtender> ToolbarExtender = MakeShareable(new FExtender);
	ToolbarExtender->AddToolBarExtension("Asset", EExtensionHook::After, InventorySystemEditorPtr->GetToolkitCommands(), FToolBarExtensionDelegate::CreateSP(this, &FOrionInventoryEditorToolbar::FillInventorySystemToolbar));
	InventorySystemEditorPtr->AddToolbarExtender(ToolbarExtender);
}

void FOrionInventoryEditorToolbar::FillInventorySystemToolbar(FToolBarBuilder& ToolbarBuilder)
{
	check(InventorySystemEditor.IsValid());
	TSharedPtr<FOrionInventoryEditor> InventorySystemEditorPtr = InventorySystemEditor.Pin();


	ToolbarBuilder.BeginSection("Inventory System");
	{
		const FText InventorySettingLabel = LOCTEXT("InventorySetting_Label", "Inventory Default Info");
		const FText InventorySettingTooltip = LOCTEXT("InventorySetting_ToolTip", "Access Inventory Default Info");
		const FSlateIcon InventorySettingIcon = FSlateIcon(FAppStyle::GetAppStyleSetName(), "ProjectSettings.TabIcon");


		ToolbarBuilder.AddToolBarButton(FOrionInventory_EditorCommands::Get().InventorySetting,
			NAME_None,
			InventorySettingLabel,
			InventorySettingTooltip,
			InventorySettingIcon
		);
	}
	ToolbarBuilder.EndSection();

	
}



#undef LOCTEXT_NAMESPACE
