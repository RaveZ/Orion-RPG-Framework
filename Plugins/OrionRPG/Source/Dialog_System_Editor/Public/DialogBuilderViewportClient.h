#pragma once

#include "CoreMinimal.h"
#include "EditorViewportClient.h"

class FDialogBuilderEditor;
class FPreviewScene;
class SEditorViewport;

class FDialogBuilderViewportClient final : public FEditorViewportClient, public TSharedFromThis<FDialogBuilderViewportClient>
{
public:
	FDialogBuilderViewportClient(
		FPreviewScene* InPreviewScene,
		const TSharedRef<SEditorViewport>& InViewport,
		const TSharedRef<FDialogBuilderEditor>& InEditor,
		bool bUseLevelWorld);

	void SetUseLevelWorld(bool bInUseLevelWorld);

	virtual bool InputKey(const FInputKeyEventArgs& EventArgs) override;

	// --- Gizmo (transform widget) support ---
	virtual UE::Widget::EWidgetMode GetWidgetMode() const override;
	virtual FVector GetWidgetLocation() const override;
	virtual ECoordSystem GetWidgetCoordSystemSpace() const override;
	virtual bool InputWidgetDelta(FViewport* InViewport, EAxisList::Type CurrentAxis, FVector& Drag, FRotator& Rot, FVector& Scale) override;

	// Helper for switching mode (optional)
	void SetWidgetMode(UE::Widget::EWidgetMode InMode);

	/**
	 * Returns true if the grid is currently visible in the viewport
	 */
	bool GetShowGrid();

	/**
	 * Will toggle the grid's visibility in the viewport
	 */
	void ToggleShowGrid();


private:
	bool HasValidSelection() const;
	void HandleClick(FViewport* InViewport);

private:
	TWeakPtr<FDialogBuilderEditor> Editor;
	UE::Widget::EWidgetMode WidgetMode = UE::Widget::WM_Translate;
	FPreviewScene* CurrentPreviewScene = nullptr;
};