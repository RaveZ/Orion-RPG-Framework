#pragma once

#include "CoreMinimal.h"
#include "Widgets/SCompoundWidget.h"
#include "Widgets/Views/SListView.h"
#include "DialogBuilderEditor.h"

class IDetailsView;
class UDialogBuilderGraph;
class UDialogStage;
class UDialogSequenceSlot;
class FUICommandList;

struct FDialogStageListItem
{
	TWeakObjectPtr<UDialogStage> DialogStage;
};

struct FDialogSlotListItem
{
	TWeakObjectPtr<UDialogSequenceSlot> Slot;
	bool bIsCameraSlot = false;
	int32 SlotIndex = INDEX_NONE;
};

using FDialogStageListItemPtr = TSharedPtr<FDialogStageListItem>;
using FDialogSlotListItemPtr = TSharedPtr<FDialogSlotListItem>;

class DIALOG_SYSTEM_EDITOR_API SDialogStageManager : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SDialogStageManager) {}
	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs, TWeakPtr<FDialogBuilderEditor> InDialogEditor, UDialogBuilderGraph* InDialogGraph);

	// Keyboard commands (Delete)
	virtual FReply OnKeyDown(const FGeometry& MyGeometry, const FKeyEvent& InKeyEvent) override;

	void Refresh();

private:
	// DialogStage list
	TSharedRef<ITableRow> GenerateDialogStageRow(FDialogStageListItemPtr InItem, const TSharedRef<STableViewBase>& InOwnerTable);
	void OnDialogStageSelectionChanged(FDialogStageListItemPtr InItem, ESelectInfo::Type InSelectInfo);
	FReply OnAddDialogStage();
	void OnRemoveDialogStage(UDialogStage* InDialogStage);
	void OnDialogStageNameCommitted(const FText& InText, ETextCommit::Type InCommitType, TWeakObjectPtr<UDialogStage> InDialogStage);
	TSharedPtr<SWidget> OnDialogStageContextMenuOpening();

	// Slot list
	TSharedRef<ITableRow> GenerateSlotRow(FDialogSlotListItemPtr InItem, const TSharedRef<STableViewBase>& InOwnerTable);
	void OnSlotSelectionChanged(FDialogSlotListItemPtr InItem, ESelectInfo::Type InSelectInfo);
	FReply OnAddSlot();
	FReply OnAddCameraSlot();
	void OnRemoveSlot(UDialogSequenceSlot* InSlot, int32 InSlotIndex, bool bIsCameraSlot);
	TSharedPtr<SWidget> OnSlotContextMenuOpening();

	// Delete command helpers
	void OnDeleteSelection();
	bool CanDeleteSelection() const;
	bool IsSelectedSlotCamera() const;

	// Refresh
	void RefreshDialogStageList();
	void RefreshSlotList();
	void MarkGraphDirty() const;

private:
	TWeakObjectPtr<UDialogBuilderGraph> DialogGraph;
	TWeakObjectPtr<UDialogStage> SelectedDialogStage;
	TWeakObjectPtr<UDialogSequenceSlot> SelectedSlot;

	TArray<FDialogStageListItemPtr> DialogStageItems;
	TArray<FDialogSlotListItemPtr> SlotItems;

	TSharedPtr<SListView<FDialogStageListItemPtr>> DialogStageListView;
	TSharedPtr<SListView<FDialogSlotListItemPtr>> SlotListView;

	TSharedPtr<IDetailsView> DialogStageDetailsView;
	TSharedPtr<IDetailsView> SlotDetailsView;

	TSharedPtr<FUICommandList> CommandList;

	int32 SelectedSlotIndex = INDEX_NONE;

	/** Pointer back to the Dialog editor that owns us */
	TWeakPtr<FDialogBuilderEditor> DialogEditorPtr;

private:
	void RestoreSelection(const FName& InDialogStageObjectName, const FGuid& InSlotId);
	void SelectDialogStageByObjectName(const FName& InDialogStageObjectName);
	void SelectSlotById(const FGuid& InSlotId);
	FDialogStageListItemPtr FindDialogStageItemByObjectName(const FName& InDialogStageObjectName) const;
	FDialogSlotListItemPtr FindSlotItemById(const FGuid& InSlotId) const;
};