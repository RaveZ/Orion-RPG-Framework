#include "SDialogSetManager.h"
#include "DialogBuilderGraph.h"
#include "DialogSet.h"
#include "Widgets/SBoxPanel.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Text/STextBlock.h"
#include "Widgets/Views/STreeView.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/Layout/SBox.h"
#include "Styling/AppStyle.h"
#include "Framework/Application/SlateApplication.h"

#define LOCTEXT_NAMESPACE "DialogSetManager"

FDialogSetManagerItem::FDialogSetManagerItem(EItemType InType, UDialogSet* InDialogSet, UDialogSequenceSlot* InSlot)
	: ItemType(InType)
	, DialogSet(InDialogSet)
	, Slot(InSlot)
{
}

FText FDialogSetManagerItem::GetDisplayName() const
{
	if (ItemType == EItemType::DialogSet && DialogSet.IsValid())
	{
		return FText::FromString(DialogSet->Name.ToString());
	}
	else if (ItemType == EItemType::Slot && Slot.IsValid())
	{
		UDialogSequenceSlot* SlotPtr = Slot.Get();
		FString SlotName = SlotPtr->DialogDefinition 
			? SlotPtr->DialogDefinition->DisplayName.ToString() 
			: TEXT("Unnamed Slot");
		return FText::FromString(SlotName);
	}
	return FText::FromString(TEXT("Unknown"));
}

void SDialogSetManager::Construct(const FArguments& InArgs, UDialogBuilderGraph* InDialogGraph)
{
	DialogGraph = InDialogGraph;

	ChildSlot
	[
		SNew(SVerticalBox)

		// Header with Add/Remove buttons
		+ SVerticalBox::Slot()
		.AutoHeight()
		.Padding(4.0f)
		[
			SNew(SHorizontalBox)
			+ SHorizontalBox::Slot()
			.AutoWidth()
			.Padding(2.0f)
			[
				SNew(SButton)
				.Text(LOCTEXT("AddDialogSet", "Add Dialog Set"))
				.OnClicked(this, &SDialogSetManager::OnAddDialogSet)
				.ToolTipText(LOCTEXT("AddDialogSetTooltip", "Add a new Dialog Set to the graph"))
			]

			+ SHorizontalBox::Slot()
			.AutoWidth()
			.Padding(2.0f)
			[
				SNew(SButton)
				.Text(LOCTEXT("AddSlot", "Add Slot"))
				.OnClicked(this, &SDialogSetManager::OnAddSlotToSelectedSet)
				.ToolTipText(LOCTEXT("AddSlotTooltip", "Add a slot to the selected Dialog Set"))
			]

			+ SHorizontalBox::Slot()
			.AutoWidth()
			.Padding(2.0f)
			[
				SNew(SButton)
				.Text(LOCTEXT("Remove", "Remove"))
				.OnClicked(this, &SDialogSetManager::OnRemoveSelected)
				.ToolTipText(LOCTEXT("RemoveTooltip", "Remove the selected item"))
			]
		]

		// Tree view
		+ SVerticalBox::Slot()
		.FillHeight(1.0f)
		.Padding(4.0f)
		[
			SAssignNew(TreeView, STreeView<FDialogSetManagerItemPtr>)
			.TreeItemsSource(&RootItems)
			.OnGenerateRow(this, &SDialogSetManager::GenerateTreeRow)
			.OnGetChildren(this, &SDialogSetManager::OnGetChildren)
			.OnSelectionChanged(this, &SDialogSetManager::OnSelectionChanged)
			.SelectionMode(ESelectionMode::Single)
		]
	];

	RefreshTreeView();
}

SDialogSetManager::~SDialogSetManager()
{
}

TSharedRef<ITableRow> SDialogSetManager::GenerateTreeRow(FDialogSetManagerItemPtr InItem, const TSharedRef<STableViewBase>& InOwnerTable)
{
	return SNew(STableRow<FDialogSetManagerItemPtr>, InOwnerTable)
	[
		SNew(STextBlock)
		.Text_Lambda([InItem]() { return InItem->GetDisplayName(); })
	];
}

void SDialogSetManager::OnGetChildren(FDialogSetManagerItemPtr InItem, TArray<FDialogSetManagerItemPtr>& OutChildren)
{
	OutChildren = InItem->Children;
}

void SDialogSetManager::OnSelectionChanged(FDialogSetManagerItemPtr InSelectedItem, ESelectInfo::Type SelectInfo)
{
	SelectedItem = InSelectedItem;
}

FReply SDialogSetManager::OnAddDialogSet()
{
	if (!DialogGraph.IsValid())
	{
		return FReply::Handled();
	}

	UDialogBuilderGraph* Graph = DialogGraph.Get();
	Graph->Modify();

	// Create new DialogSet
	UDialogSet* NewDialogSet = NewObject<UDialogSet>(Graph, NAME_None, RF_Transactional);
	if (NewDialogSet)
	{
		NewDialogSet->Name = FText::FromString(FString::Printf(TEXT("DialogSet_%d"), Graph->DialogSets.Num() + 1));
		Graph->DialogSets.Add(NewDialogSet);

		RefreshTreeView();
	}

	return FReply::Handled();
}

FReply SDialogSetManager::OnRemoveSelected()
{
	if (!SelectedItem.IsValid() || !DialogGraph.IsValid())
	{
		return FReply::Handled();
	}

	UDialogBuilderGraph* Graph = DialogGraph.Get();
	Graph->Modify();

	if (SelectedItem->ItemType == FDialogSetManagerItem::EItemType::DialogSet && SelectedItem->DialogSet.IsValid())
	{
		UDialogSet* SetToRemove = SelectedItem->DialogSet.Get();
		Graph->DialogSets.Remove(SetToRemove);
		RefreshTreeView();
	}
	else if (SelectedItem->ItemType == FDialogSetManagerItem::EItemType::Slot && SelectedItem->Slot.IsValid())
	{
		// Find which DialogSet contains this slot and remove it
		UDialogSequenceSlot* SlotToRemove = SelectedItem->Slot.Get();
		for (UDialogSet* DialogSet : Graph->DialogSets)
		{
			if (DialogSet)
			{
				DialogSet->Modify();
				DialogSet->Slots.Remove(SlotToRemove);
				DialogSet->CameraSlots.Remove(SlotToRemove);
			}
		}
		RefreshTreeView();
	}

	SelectedItem.Reset();
	return FReply::Handled();
}

FReply SDialogSetManager::OnAddSlotToSelectedSet()
{
	if (!SelectedItem.IsValid() || !DialogGraph.IsValid())
	{
		return FReply::Handled();
	}

	UDialogBuilderGraph* Graph = DialogGraph.Get();

	// If a DialogSet is selected, add slot to it
	if (SelectedItem->ItemType == FDialogSetManagerItem::EItemType::DialogSet && SelectedItem->DialogSet.IsValid())
	{
		UDialogSet* DialogSet = SelectedItem->DialogSet.Get();
		DialogSet->Modify();

		UDialogSequenceSlot* NewSlot = NewObject<UDialogSequenceSlot>(Graph, NAME_None, RF_Transactional);
		if (NewSlot)
		{
			DialogSet->Slots.Add(NewSlot);
			RefreshTreeView();
		}
	}
	// If a slot is selected, add to its parent DialogSet
	else if (SelectedItem->ItemType == FDialogSetManagerItem::EItemType::Slot && SelectedItem->Slot.IsValid())
	{
		// Find parent DialogSet
		for (UDialogSet* DialogSet : Graph->DialogSets)
		{
			if (DialogSet && (DialogSet->Slots.Contains(SelectedItem->Slot.Get()) || DialogSet->CameraSlots.Contains(SelectedItem->Slot.Get())))
			{
				DialogSet->Modify();
				UDialogSequenceSlot* NewSlot = NewObject<UDialogSequenceSlot>(Graph, NAME_None, RF_Transactional);
				if (NewSlot)
				{
					DialogSet->Slots.Add(NewSlot);
					RefreshTreeView();
					break;
				}
			}
		}
	}

	return FReply::Handled();
}

void SDialogSetManager::RefreshTreeView()
{
	RebuildTree();
	if (TreeView.IsValid())
	{
		TreeView->RebuildList();
	}
}

void SDialogSetManager::RebuildTree()
{
	RootItems.Empty();

	if (!DialogGraph.IsValid())
	{
		return;
	}

	UDialogBuilderGraph* Graph = DialogGraph.Get();

	for (UDialogSet* DialogSet : Graph->DialogSets)
	{
		if (DialogSet)
		{
			FDialogSetManagerItemPtr SetItem = MakeShareable(new FDialogSetManagerItem(FDialogSetManagerItem::EItemType::DialogSet, DialogSet));

			// Add Slots as children
			for (UDialogSequenceSlot* Slot : DialogSet->Slots)
			{
				if (Slot)
				{
					FDialogSetManagerItemPtr SlotItem = MakeShareable(new FDialogSetManagerItem(FDialogSetManagerItem::EItemType::Slot, DialogSet, Slot));
					SetItem->Children.Add(SlotItem);
				}
			}

			// Add Camera Slots as children
			for (UDialogSequenceSlot* CameraSlot : DialogSet->CameraSlots)
			{
				if (CameraSlot)
				{
					FDialogSetManagerItemPtr SlotItem = MakeShareable(new FDialogSetManagerItem(FDialogSetManagerItem::EItemType::Slot, DialogSet, CameraSlot));
					SetItem->Children.Add(SlotItem);
				}
			}

			RootItems.Add(SetItem);
		}
	}
}

#undef LOCTEXT_NAMESPACE