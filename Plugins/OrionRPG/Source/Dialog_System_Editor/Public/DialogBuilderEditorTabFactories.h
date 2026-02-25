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


//Dialog Editor Tab Factories
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


//Dialog Sequencer Tab Factories
struct FDialogSequencerTabSummoner : public FWorkflowTabFactory
{
public:
	FDialogSequencerTabSummoner(TSharedPtr<class FDialogBuilderEditor> InDialogEditorPtr);

	virtual TSharedRef<SWidget> CreateTabBody(const FWorkflowTabSpawnInfo& Info) const override;
	virtual FText GetTabToolTipText(const FWorkflowTabSpawnInfo& Info) const override;

protected:
	TWeakPtr<class FDialogBuilderEditor> DialogEditorPtr;
};

struct FDialogSequencerViewportSummoner : public FWorkflowTabFactory
{
public:
	FDialogSequencerViewportSummoner(TSharedPtr<class FDialogBuilderEditor> InDialogEditorPtr);

	virtual TSharedRef<SWidget> CreateTabBody(const FWorkflowTabSpawnInfo& Info) const override;
	virtual FText GetTabToolTipText(const FWorkflowTabSpawnInfo& Info) const override;

protected:
	TWeakPtr<class FDialogBuilderEditor> DialogEditorPtr;
};