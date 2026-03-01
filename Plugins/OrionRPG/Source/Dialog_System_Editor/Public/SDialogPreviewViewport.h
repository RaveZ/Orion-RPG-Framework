#pragma once

#include "CoreMinimal.h"
#include "SEditorViewport.h"

class FAdvancedPreviewScene;
class FEditorViewportClient;
class FDialogBuilderEditor;

class SDialogPreviewViewport : public SEditorViewport
{
public:
	SLATE_BEGIN_ARGS(SDialogPreviewViewport) {}
	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs, TSharedRef<FDialogBuilderEditor> InEditor, TSharedRef<FAdvancedPreviewScene> InPreviewScene);

	void SetUseLevelWorld(bool bInUseLevelWorld);

protected:
	virtual TSharedRef<FEditorViewportClient> MakeEditorViewportClient() override;
	virtual TSharedPtr<SWidget> BuildViewportToolbar() override;


private:
	TWeakPtr<FDialogBuilderEditor> Editor;
	TWeakPtr<class FAssetEditorToolkit> AssetEditorToolkitPtr;

	TSharedPtr<FAdvancedPreviewScene> PreviewScene;
	TSharedPtr<FEditorViewportClient> ViewportClient;

	bool bUseLevelWorld = false;
};