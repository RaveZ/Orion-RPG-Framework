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

	// FEditorViewportClient interface
	virtual bool InputKey(const FInputKeyEventArgs& EventArgs) override;
	virtual void ProcessClick(class FSceneView& View, class HHitProxy* HitProxy, FKey Key, EInputEvent Event, uint32 HitX, uint32 HitY) override;
	virtual bool InputWidgetDelta(FViewport* Viewport, EAxisList::Type CurrentAxis, FVector& Drag, FRotator& Rot, FVector& Scale) override;
	virtual UE::Widget::EWidgetMode GetWidgetMode() const override;
	virtual FVector GetWidgetLocation() const override;
	virtual void TrackingStarted(const struct FInputEventState& InInputState, bool bIsDragging, bool bNudge) override;
	virtual void TrackingStopped() override;
	virtual bool BeginTransform(const FGizmoState& InState) override;
	virtual bool EndTransform(const FGizmoState& InState) override;
	virtual ECoordSystem GetWidgetCoordSystemSpace() const override;


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

private:
	/** The current transaction for undo/redo */
	FScopedTransaction* ScopedTransaction;

	/** If true then we are manipulating a specific property or component */
	bool bIsManipulating;

	bool HandleBeginTransform();
	bool HandleEndTransform();

	TWeakPtr<FDialogBuilderEditor> Editor;

	UE::Widget::EWidgetMode WidgetMode;
	ECoordSystem WidgetCoordSystem;

	FPreviewScene* CurrentPreviewScene = nullptr;
};