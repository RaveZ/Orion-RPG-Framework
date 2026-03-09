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


	/**
	 * Returns true if the grid is currently visible in the viewport
	 */
	bool GetShowGrid();

	/**
	 * Will toggle the grid's visibility in the viewport
	 */
	void ToggleShowGrid();

protected:
	/**
	 * Initiates a transaction.
	 */
	void BeginTransaction(const FText& Description);

	/**
	 * Ends the current transaction, if one exists.
	 */
	void EndTransaction();


private:
	bool HasValidSelection() const;
	void HandleClick(FViewport* InViewport);

private:
	/** The current transaction for undo/redo */
	FScopedTransaction* ScopedTransaction;

	TWeakPtr<FDialogBuilderEditor> Editor;

	UE::Widget::EWidgetMode WidgetMode;
	ECoordSystem WidgetCoordSystem;

	FPreviewScene* CurrentPreviewScene = nullptr;
};