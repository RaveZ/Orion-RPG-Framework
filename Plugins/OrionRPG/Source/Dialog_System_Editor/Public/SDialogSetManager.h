#pragma once

#include "CoreMinimal.h"
#include "Widgets/SCompoundWidget.h"
#include "Widgets/Views/SListView.h"

class IDetailsView;
class UDialogBuilderGraph;
class UDialogSet;
class UDialogSequenceSlot;
class FUICommandList;

struct FDialogSetListItem
{
	TWeakObjectPtr<UDialogSet> DialogSet;
};

struct FDialogSlotListItem
{
	TWeakObjectPtr<UDialogSequenceSlot> Slot;
	bool bIsCameraSlot = false;
};

using FDialogSetListItemPtr = TSharedPtr<FDialogSetListItem>;
using FDialogSlotListItemPtr = TSharedPtr<FDialogSlotListItem>;

class DIALOG_SYSTEM_EDITOR_API SDialogSetManager : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SDialogSetManager) {}
	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs, UDialogBuilderGraph* InDialogGraph);

	// Keyboard commands (Delete)
	virtual FReply OnKeyDown(const FGeometry& MyGeometry, const FKeyEvent& InKeyEvent) override;

	void Refresh();

private:
	// DialogSet list
	TSharedRef<ITableRow> GenerateDialogSetRow(FDialogSetListItemPtr InItem, const TSharedRef<STableViewBase>& InOwnerTable);
	void OnDialogSetSelectionChanged(FDialogSetListItemPtr InItem, ESelectInfo::Type InSelectInfo);
	FReply OnAddDialogSet();
	void OnRemoveDialogSet(UDialogSet* InDialogSet);
	void OnDialogSetNameCommitted(const FText& InText, ETextCommit::Type InCommitType, TWeakObjectPtr<UDialogSet> InDialogSet);
	TSharedPtr<SWidget> OnDialogSetContextMenuOpening();

	// Slot list
	TSharedRef<ITableRow> GenerateSlotRow(FDialogSlotListItemPtr InItem, const TSharedRef<STableViewBase>& InOwnerTable);
	void OnSlotSelectionChanged(FDialogSlotListItemPtr InItem, ESelectInfo::Type InSelectInfo);
	FReply OnAddSlot();
	FReply OnAddCameraSlot();
	void OnRemoveSlot(UDialogSequenceSlot* InSlot, bool bIsCameraSlot);
	TSharedPtr<SWidget> OnSlotContextMenuOpening();

	// Delete command helpers
	void OnDeleteSelection();
	bool CanDeleteSelection() const;
	bool IsSelectedSlotCamera() const;

	// Refresh
	void RefreshDialogSetList();
	void RefreshSlotList();
	void MarkGraphDirty() const;

private:
	TWeakObjectPtr<UDialogBuilderGraph> DialogGraph;
	TWeakObjectPtr<UDialogSet> SelectedDialogSet;
	TWeakObjectPtr<UDialogSequenceSlot> SelectedSlot;

	TArray<FDialogSetListItemPtr> DialogSetItems;
	TArray<FDialogSlotListItemPtr> SlotItems;

	TSharedPtr<SListView<FDialogSetListItemPtr>> DialogSetListView;
	TSharedPtr<SListView<FDialogSlotListItemPtr>> SlotListView;

	TSharedPtr<IDetailsView> DialogSetDetailsView;
	TSharedPtr<IDetailsView> SlotDetailsView;

	TSharedPtr<FUICommandList> CommandList;
};