#pragma once

#include "CoreMinimal.h"
#include "AssetRegistry/AssetData.h"
#include "Widgets/SCompoundWidget.h"
#include "Widgets/Views/SListView.h"

class FDialogBuilderEditor;
class IDetailsView;
class UDialogSequenceShot;

struct FDialogCameraPresetItem
{
	FText DisplayName;
	FAssetData AssetData;
	TObjectPtr<UDialogSequenceShot> ShotInstance = nullptr;
};

using FDialogCameraPresetItemPtr = TSharedPtr<FDialogCameraPresetItem>;

class DIALOG_SYSTEM_EDITOR_API SDialogCameraPresets : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SDialogCameraPresets) {}
	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs, TWeakPtr<FDialogBuilderEditor> InDialogEditor);
	virtual ~SDialogCameraPresets();

	void Refresh();

private:
	TSharedRef<ITableRow> GenerateCameraPresetRow(
		FDialogCameraPresetItemPtr InItem,
		const TSharedRef<STableViewBase>& InOwnerTable);

	void OnCameraPresetSelectionChanged(FDialogCameraPresetItemPtr InItem, ESelectInfo::Type InSelectInfo);

	void RebuildCameraPresetList();
	UDialogSequenceShot* CreateSelectedShotInstance(const FAssetData& AssetData) const;
	bool IsCameraPresetBlueprint(const FAssetData& AssetData) const;
	void ReleaseShotDuplicates();

private:
	TWeakPtr<FDialogBuilderEditor> DialogEditorPtr;

	TArray<FDialogCameraPresetItemPtr> CameraPresetItems;
	TArray<TObjectPtr<UDialogSequenceShot>> ShotDuplicates;

	TSharedPtr<SListView<FDialogCameraPresetItemPtr>> CameraPresetListView;
	TSharedPtr<IDetailsView> ShotDetailsView;
};