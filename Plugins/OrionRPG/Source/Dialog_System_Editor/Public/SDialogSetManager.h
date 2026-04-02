#pragma once

#include "CoreMinimal.h"
#include "Widgets/SCompoundWidget.h"
#include "Widgets/Views/STreeView.h"
#include "Widgets/Input/SButton.h"

class UDialogBuilderGraph;
class UDialogSet;
class UDialogSequenceSlot;
class FDialogSetManagerItem;

using FDialogSetManagerItemPtr = TSharedPtr<FDialogSetManagerItem>;

/**
 * Tree item representing either a DialogSet or a Slot within it
 */
class FDialogSetManagerItem : public TSharedFromThis<FDialogSetManagerItem>
{
public:
	enum class EItemType
	{
		DialogSet,
		Slot
	};

	FDialogSetManagerItem(EItemType InType, UDialogSet* InDialogSet = nullptr, UDialogSequenceSlot* InSlot = nullptr);

	EItemType ItemType;
	TWeakObjectPtr<UDialogSet> DialogSet;
	TWeakObjectPtr<UDialogSequenceSlot> Slot;
	TArray<FDialogSetManagerItemPtr> Children;

	FText GetDisplayName() const;
	bool HasChildren() const { return Children.Num() > 0; }
};

/**
 * Widget to manage DialogSets and their Slots within a DialogBuilderGraph
 */
class DIALOG_SYSTEM_EDITOR_API SDialogSetManager : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SDialogSetManager)
	{}
	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs, UDialogBuilderGraph* InDialogGraph);

	virtual ~SDialogSetManager();

private:
	// Tree view callbacks
	TSharedRef<ITableRow> GenerateTreeRow(FDialogSetManagerItemPtr InItem, const TSharedRef<STableViewBase>& InOwnerTable);
	void OnGetChildren(FDialogSetManagerItemPtr InItem, TArray<FDialogSetManagerItemPtr>& OutChildren);
	void OnSelectionChanged(FDialogSetManagerItemPtr InSelectedItem, ESelectInfo::Type SelectInfo);

	// Button callbacks
	FReply OnAddDialogSet();
	FReply OnRemoveSelected();
	FReply OnAddSlotToSelectedSet();

	// Helper functions
	void RefreshTreeView();
	void RebuildTree();
	void SelectDialogSet(UDialogSet* InDialogSet);

	TWeakObjectPtr<UDialogBuilderGraph> DialogGraph;
	TSharedPtr<STreeView<FDialogSetManagerItemPtr>> TreeView;
	TArray<FDialogSetManagerItemPtr> RootItems;
	FDialogSetManagerItemPtr SelectedItem;
};