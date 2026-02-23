// Copyright 2025 Ivan Chandra. All Rights Reserved.

#pragma once

#include "Delegates/Delegate.h"
#include "EdGraph/EdGraph.h"
#include "Internationalization/Text.h"
#include "Misc/Attribute.h"
#include "Templates/SharedPointer.h"
#include "WorkflowOrientedApp/WorkflowTabFactory.h"
#include "WorkflowOrientedApp/WorkflowUObjectDocuments.h"

class SDockTab;
class SGraphEditor;
class SWidget;
struct FSlateBrush;


struct FDialogEditorDetailsSummoner : public FWorkflowTabFactory
{
public:
	FDialogEditorDetailsSummoner(TSharedPtr<class FDialogBuilderEditor> InDialogEditorPtr);

	virtual TSharedRef<SWidget> CreateTabBody(const FWorkflowTabSpawnInfo& Info) const override;
	virtual FText GetTabToolTipText(const FWorkflowTabSpawnInfo& Info) const override;

protected:
	TWeakPtr<class FDialogBuilderEditor> DialogEditorPtr;
};

struct FDialogEditorListSummoner : public FWorkflowTabFactory
{
public:
	FDialogEditorListSummoner(TSharedPtr<class FDialogBuilderEditor> InDialogEditorPtr);

	virtual TSharedRef<SWidget> CreateTabBody(const FWorkflowTabSpawnInfo& Info) const override;
	virtual FText GetTabToolTipText(const FWorkflowTabSpawnInfo& Info) const override;

protected:
	TWeakPtr<class FDialogBuilderEditor> DialogEditorPtr;
};

struct FDialogGraphEditorSummoner : public FDocumentTabFactoryForObjects<UEdGraph>
{
public:
	DECLARE_DELEGATE_RetVal_OneParam(TSharedRef<SGraphEditor>, FOnCreateGraphEditorWidget, UEdGraph*);
public:
	FDialogGraphEditorSummoner(TSharedPtr<class FDialogBuilderEditor> InDialogEditorPtr, FOnCreateGraphEditorWidget CreateGraphEditorWidgetCallback);

	virtual void OnTabActivated(TSharedPtr<SDockTab> Tab) const override;
	virtual void OnTabRefreshed(TSharedPtr<SDockTab> Tab) const override;

protected:
	virtual TAttribute<FText> ConstructTabNameForObject(UEdGraph* DocumentID) const override;
	virtual TSharedRef<SWidget> CreateTabBodyForObject(const FWorkflowTabSpawnInfo& Info, UEdGraph* DocumentID) const override;
	virtual const FSlateBrush* GetTabIconForObject(const FWorkflowTabSpawnInfo& Info, UEdGraph* DocumentID) const override;
	virtual void SaveState(TSharedPtr<SDockTab> Tab, TSharedPtr<FTabPayload> Payload) const override;

protected:
	TWeakPtr<class FDialogBuilderEditor> DialogEditorPtr;
	FOnCreateGraphEditorWidget OnCreateGraphEditorWidget;
};
