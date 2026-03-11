#pragma once

#include "CoreMinimal.h"
#include "SEditorViewport.h"

class FPreviewScene;
class FEditorViewportClient;
class FDialogBuilderEditor;

class SDialogPreviewViewport : public SEditorViewport
{
public:
	SLATE_BEGIN_ARGS(SDialogPreviewViewport) {}
	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs, TSharedRef<FDialogBuilderEditor> InEditor);

	void SetUseLevelWorld(bool bInUseLevelWorld);

	/**
	 * Destructor.
	 */
	virtual ~SDialogPreviewViewport();
	
	/**
	 * Request a refresh of the preview scene/world. Will recreate actors as needed.
	 *
	 * @param bResetCamera If true, the camera will be reset to its default position based on the preview.
	 * @param bRefreshNow If true, the preview will be refreshed immediately. Otherwise, it will be deferred until the next tick (default behavior).
	 */
	void RequestRefresh(bool bResetCamera = false, bool bRefreshNow = false);
protected:
	virtual TSharedRef<FEditorViewportClient> MakeEditorViewportClient() override;
	virtual TSharedPtr<SWidget> BuildViewportToolbar() override;
	virtual void BindCommands() override;
	/**
	 * Focuses the viewport on the currently selected components
	 */
	virtual void OnFocusViewportToSelection() override;


private:
	/** One-off active timer to update the preview */
	EActiveTimerReturnType DeferredUpdatePreview(double InCurrentTime, float InDeltaTime, bool bResetCamera);

private:
	TWeakPtr<FDialogBuilderEditor> DialogEditor;
	TWeakPtr<class FAssetEditorToolkit> AssetEditorToolkitPtr;
	
	TSharedPtr<class FDialogBuilderViewportClient> ViewportClient;

	bool bUseLevelWorld = false;

	/** Whether the active timer (for updating the preview) is registered */
	bool bIsActiveTimerRegistered;

	/** Handle to the registered OnPreviewFeatureLevelChanged delegate. */
	FDelegateHandle PreviewFeatureLevelChangedHandle;
};