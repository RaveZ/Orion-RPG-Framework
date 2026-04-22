#include "SDialogStageManager.h"

#include "DialogBuilderGraph.h"
#include "DialogBuilderNode_DialogSequence.h"
#include "DialogDefinition.h"
#include "DialogSequence.h"
#include "DialogBuilder_EditorStyle.h"
#include "DialogStage.h"
#include "PropertyEditorModule.h"
#include "IDetailsView.h"
#include "Modules/ModuleManager.h"
#include "ScopedTransaction.h"

#include "Framework/Commands/GenericCommands.h"
#include "Framework/MultiBox/MultiBoxBuilder.h"
#include "Styling/AppStyle.h"
#include "Widgets/SBoxPanel.h"
#include "Widgets/Layout/SSplitter.h"
#include "Widgets/Images/SImage.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Text/SInlineEditableTextBlock.h"
#include "Widgets/Text/STextBlock.h"
#include "Widgets/Views/SListView.h"
#include "Widgets/Layout/SBorder.h"

#define LOCTEXT_NAMESPACE "DialogStageManager"

// -----------------------------------------------------------------------------
// Local palette row widgets (defined inside DialogStageManager, as requested)
// -----------------------------------------------------------------------------
class SDialogStagePaletteItem : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SDialogStagePaletteItem) {}
		SLATE_ARGUMENT(TWeakObjectPtr<UDialogStage>, DialogStage)
		SLATE_EVENT(FOnTextCommitted, OnNameCommitted)
	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs)
	{
		DialogStage = InArgs._DialogStage;
		OnNameCommitted = InArgs._OnNameCommitted;

		ChildSlot
		[
			SNew(SHorizontalBox)

			/*+ SHorizontalBox::Slot()
			.AutoWidth()
			.VAlign(VAlign_Center)
			.Padding(0.f, 0.f, 4.f, 0.f)
			[
				SNew(SImage).Image(FDialogBuilder_EditorStyle::Get().GetBrush("ClassIcon.Dialog"))
			]*/

			+ SHorizontalBox::Slot()
			.FillWidth(1.0f)
			.Padding(FMargin(7.0f, 4.0f, 4.0f, 4.0f))
			.VAlign(VAlign_Center)
			[
				SNew(SInlineEditableTextBlock)
				.Text_Lambda([this]()
				{
					if (const UDialogStage* LocalSet = DialogStage.Get())
					{
						return LocalSet->Name.IsEmpty() ? LOCTEXT("UnnamedDialogStage", "None") : LocalSet->Name;
					}
					return LOCTEXT("InvalidDialogStage", "Invalid");
				})
				.OnTextCommitted(OnNameCommitted)
				.IsReadOnly(false)
				.Font(FCoreStyle::GetDefaultFontStyle("Bold", 10.0f))
			]
		];
	}

private:
	TWeakObjectPtr<UDialogStage> DialogStage;
	FOnTextCommitted OnNameCommitted;
};

class SDialogSlotPaletteItem : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SDialogSlotPaletteItem) {}
		SLATE_ARGUMENT(TWeakObjectPtr<UDialogSequenceSlot>, Slot)
		SLATE_ARGUMENT(bool, IsCameraSlot)
	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs)
	{
		Slot = InArgs._Slot;
		bIsCameraSlot = InArgs._IsCameraSlot;

		ChildSlot
		[
			SNew(SHorizontalBox)

			/*+ SHorizontalBox::Slot()
			.AutoWidth()
			.VAlign(VAlign_Center)
			.Padding(0.f, 0.f, 4.f, 0.f)
			[
				SNew(SImage)
				.Image(FDialogBuilder_EditorStyle::Get().GetBrush("ClassIcon.Dialog.Prop"))
			]*/

			+ SHorizontalBox::Slot()
			.FillWidth(1.0f)
			.Padding(FMargin(7.0f, 4.0f, 4.0f, 4.0f))
			.VAlign(VAlign_Center)
			[
				SNew(STextBlock)
				.Text_Lambda([this]()
				{
					const UDialogSequenceSlot* LocalSlot = Slot.Get();
					UDialogDefinition* DialogDef = LocalSlot ? LocalSlot->DialogDefinition : nullptr;
					if (!LocalSlot)
					{
						return LOCTEXT("InvalidSlot", "Invalid");
					}

					const FString Label = LocalSlot->GetActorTag().IsValid() && DialogDef
						? DialogDef->DisplayName.ToString()
						: "None";

					return FText::FromString(Label);
				})
				.Font(FCoreStyle::GetDefaultFontStyle("Bold", 10.0f))
			]
		];
	}

private:
	TWeakObjectPtr<UDialogSequenceSlot> Slot;
	bool bIsCameraSlot = false;
};

// -----------------------------------------------------------------------------
// SDialogStageManager
// -----------------------------------------------------------------------------
void SDialogStageManager::Construct(const FArguments& InArgs, TWeakPtr<FDialogBuilderEditor> InDialogEditor,  UDialogBuilderGraph* InDialogGraph)
{
	DialogGraph = InDialogGraph;
	DialogEditorPtr = InDialogEditor;
	CommandList = MakeShareable(new FUICommandList);
	CommandList->MapAction(
		FGenericCommands::Get().Delete,
		FExecuteAction::CreateSP(this, &SDialogStageManager::OnDeleteSelection),
		FCanExecuteAction::CreateSP(this, &SDialogStageManager::CanDeleteSelection));

	FPropertyEditorModule& PropertyModule = FModuleManager::LoadModuleChecked<FPropertyEditorModule>("PropertyEditor");

	FDetailsViewArgs DetailsArgs;
	DetailsArgs.NameAreaSettings = FDetailsViewArgs::HideNameArea;
	DetailsArgs.bUpdatesFromSelection = false;
	DetailsArgs.DefaultsOnlyVisibility = EEditDefaultsOnlyNodeVisibility::Hide;

	DialogStageDetailsView = PropertyModule.CreateDetailView(DetailsArgs);
	SlotDetailsView = PropertyModule.CreateDetailView(DetailsArgs);

	ChildSlot
	[
		SNew(SSplitter)

		// 1) DialogStage List
		+ SSplitter::Slot()
		.Value(0.25f)
		[
			SNew(SBorder)
			.BorderImage(FAppStyle::GetBrush("ToolPanel.GroupBorder"))
			.Padding(4.0f)
			[
				SNew(SVerticalBox)

				+ SVerticalBox::Slot()
				.AutoHeight()
				.Padding(4.0f)
				[
					SNew(SHorizontalBox)
					+ SHorizontalBox::Slot()
					.FillWidth(1.0f)
					.VAlign(VAlign_Center)
					[
						SNew(STextBlock)
							.Text(LOCTEXT("DialogStageListTitle", "Dialog Stages"))
							.Font(FCoreStyle::GetDefaultFontStyle("Bold", 10.5f))
					]
					+ SHorizontalBox::Slot()
					.AutoWidth()
					[
						SNew(SButton)
						.ButtonStyle(FAppStyle::Get(), "SimpleButton")
						.ContentPadding(FMargin(2.f))
						.ToolTipText(LOCTEXT("AddDialogStageTooltip", "Add Dialog Stage"))
						.OnClicked(this, &SDialogStageManager::OnAddDialogStage)
						[
							SNew(SImage).Image(FAppStyle::GetBrush("Icons.Plus"))
						]
					]
				]

				+ SVerticalBox::Slot()
				.FillHeight(1.0f)
				.Padding(2.0f)
				[
					SAssignNew(DialogStageListView, SListView<FDialogStageListItemPtr>)
					.ListItemsSource(&DialogStageItems)
					.OnGenerateRow(this, &SDialogStageManager::GenerateDialogStageRow)
					.OnSelectionChanged(this, &SDialogStageManager::OnDialogStageSelectionChanged)
					.OnContextMenuOpening(this, &SDialogStageManager::OnDialogStageContextMenuOpening)
					.SelectionMode(ESelectionMode::Single)
				]
			]
		]

		// 2) DialogStage Details
		+ SSplitter::Slot()
		.Value(0.25f)
		[
			SNew(SBorder)
			.BorderImage(FAppStyle::GetBrush("ToolPanel.GroupBorder"))
			.Padding(4.0f)
			[
				DialogStageDetailsView.ToSharedRef()
			]
		]

		// 3) Slot List
		+ SSplitter::Slot()
		.Value(0.25f)
		[
			SNew(SBorder)
			.BorderImage(FAppStyle::GetBrush("ToolPanel.GroupBorder"))
			.Padding(4.0f)
			[
				SNew(SVerticalBox)

				+ SVerticalBox::Slot()
				.AutoHeight()
				.Padding(4.0f)
				[
					SNew(SHorizontalBox)
					+ SHorizontalBox::Slot()
					.FillWidth(1.0f)
					.VAlign(VAlign_Center)
					[
						SNew(STextBlock)
							.Text(LOCTEXT("SlotListTitle", "Slots"))
							.Font(FCoreStyle::GetDefaultFontStyle("Bold", 10.5f))
					]
					+ SHorizontalBox::Slot()
					.AutoWidth()
					.Padding(0.f, 0.f, 2.f, 0.f)
					[
						SNew(SButton)
						.ButtonStyle(FAppStyle::Get(), "SimpleButton")
						.ContentPadding(FMargin(2.f))
						.ToolTipText(LOCTEXT("AddSlotTooltip", "Add Slot"))
						.OnClicked(this, &SDialogStageManager::OnAddSlot)
						[
							SNew(SImage).Image(FAppStyle::GetBrush("Icons.Plus"))
						]
					]
				]

				+ SVerticalBox::Slot()
				.FillHeight(1.0f)
				.Padding(2.0f)
				[
					SAssignNew(SlotListView, SListView<FDialogSlotListItemPtr>)
					.ListItemsSource(&SlotItems)
					.OnGenerateRow(this, &SDialogStageManager::GenerateSlotRow)
					.OnSelectionChanged(this, &SDialogStageManager::OnSlotSelectionChanged)
					.OnContextMenuOpening(this, &SDialogStageManager::OnSlotContextMenuOpening)
					.SelectionMode(ESelectionMode::Single)
				]
			]
		]

		// 4) Slot Details
		+ SSplitter::Slot()
		.Value(0.25f)
		[
			SNew(SBorder)
			.BorderImage(FAppStyle::GetBrush("ToolPanel.GroupBorder"))
			.Padding(4.0f)
			[
				SlotDetailsView.ToSharedRef()
			]
		]
	];

	RefreshDialogStageList();
}

FReply SDialogStageManager::OnKeyDown(const FGeometry& MyGeometry, const FKeyEvent& InKeyEvent)
{
	if (CommandList.IsValid() && CommandList->ProcessCommandBindings(InKeyEvent))
	{
		return FReply::Handled();
	}
	return FReply::Unhandled();
}

TSharedRef<ITableRow> SDialogStageManager::GenerateDialogStageRow(FDialogStageListItemPtr InItem, const TSharedRef<STableViewBase>& InOwnerTable)
{
	const TWeakObjectPtr<UDialogStage> WeakDialogStage = InItem.IsValid() ? InItem->DialogStage : nullptr;

	return SNew(STableRow<FDialogStageListItemPtr>, InOwnerTable)
	[
		SNew(SDialogStagePaletteItem)
		.DialogStage(WeakDialogStage)
		.OnNameCommitted(FOnTextCommitted::CreateSP(this, &SDialogStageManager::OnDialogStageNameCommitted, WeakDialogStage))
	];
}

void SDialogStageManager::OnDialogStageNameCommitted(const FText& InText, ETextCommit::Type InCommitType, TWeakObjectPtr<UDialogStage> InDialogStage)
{
	UDialogStage* DialogStage = InDialogStage.Get();
	if (!DialogStage)
	{
		return;
	}

	DialogStage->Modify();
	DialogStage->Name = InText;

	MarkGraphDirty();
	RefreshDialogStageList();
}

void SDialogStageManager::OnDialogStageSelectionChanged(FDialogStageListItemPtr InItem, ESelectInfo::Type InSelectInfo)
{
	TSharedPtr<FDialogBuilderEditor> DialogEditor = DialogEditorPtr.Pin();
	SelectedDialogStage = InItem.IsValid() ? InItem->DialogStage.Get() : nullptr;
	SelectedSlot.Reset();
	SelectedSlotIndex = INDEX_NONE;

	if (DialogStageDetailsView.IsValid())
	{
		if (SelectedDialogStage == DialogEditor->GetDialogStageTemplate())
		{
			if (GEditor)
			{
				AActor* SequencePivot = DialogEditor ? DialogEditor->GetSequencePivot() : nullptr;
				GEditor->SelectNone(true/*bNoteSelectionChange*/, true/*bDeselectBSPSurfs*/, false/*bWarnAboutManyActors*/);
				if (SequencePivot)
				{
					GEditor->SelectActor(SequencePivot, true, true/*bNoteSelectionChange*/);
				}

				GEditor->NoteSelectionChange();
			}
		}
		DialogStageDetailsView->SetObject(SelectedDialogStage.Get());
	}
	if (SlotDetailsView.IsValid())
	{
		SlotDetailsView->SetObject(nullptr);
	}

	RefreshSlotList();
}

FReply SDialogStageManager::OnAddDialogStage()
{
	UDialogBuilderGraph* Graph = DialogGraph.Get();
	if (!Graph)
	{
		return FReply::Handled();
	}

	const FScopedTransaction Transaction(LOCTEXT("AddDialogStageTransaction", "Add Dialog Stage"));
	Graph->Modify();

	UDialogStage* NewDialogStage = NewObject<UDialogStage>(Graph, NAME_None, RF_Transactional);
	if (NewDialogStage)
	{
		NewDialogStage->OwningDialogGraph = Graph;
		NewDialogStage->Name = FText::FromString(FString::Printf(TEXT("DialogStage%d"), Graph->DialogStages.Num() + 1));
		Graph->DialogStages.Add(NewDialogStage);

		MarkGraphDirty();
		RefreshDialogStageList();

		SelectedDialogStage = NewDialogStage;
		if (DialogStageDetailsView.IsValid())
		{
			DialogStageDetailsView->SetObject(NewDialogStage);
		}
		RefreshSlotList();
	}

	return FReply::Handled();
}

void SDialogStageManager::OnRemoveDialogStage(UDialogStage* InDialogStage)
{
	UDialogBuilderGraph* Graph = DialogGraph.Get();
	if (!Graph || !InDialogStage)
	{
		return;
	}

	const FScopedTransaction Transaction(LOCTEXT("DeleteDialogStageTransaction", "Delete Dialog Stage"));
	Graph->Modify();
	InDialogStage->Modify();

	Graph->DialogStages.Remove(InDialogStage);

	if (SelectedDialogStage.Get() == InDialogStage)
	{
		SelectedDialogStage.Reset();
		SelectedSlot.Reset();

		if (DialogStageDetailsView.IsValid())
		{
			DialogStageDetailsView->SetObject(nullptr);
		}
		if (SlotDetailsView.IsValid())
		{
			SlotDetailsView->SetObject(nullptr);
		}
	}

	MarkGraphDirty();
	RefreshDialogStageList();
	RefreshSlotList();
}

TSharedRef<ITableRow> SDialogStageManager::GenerateSlotRow(FDialogSlotListItemPtr InItem, const TSharedRef<STableViewBase>& InOwnerTable)
{
	const TWeakObjectPtr<UDialogSequenceSlot> WeakSlot = InItem.IsValid() ? InItem->Slot : nullptr;
	const bool bCamera = InItem.IsValid() ? InItem->bIsCameraSlot : false;

	return SNew(STableRow<FDialogSlotListItemPtr>, InOwnerTable)
	[
		SNew(SDialogSlotPaletteItem)
		.Slot(WeakSlot)
		.IsCameraSlot(bCamera)
	];
}

void SDialogStageManager::OnSlotSelectionChanged(FDialogSlotListItemPtr InItem, ESelectInfo::Type InSelectInfo)
{
	TSharedPtr<FDialogBuilderEditor> DialogEditor = DialogEditorPtr.Pin();
	SelectedSlot = InItem.IsValid() ? InItem->Slot.Get() : nullptr;
	SelectedSlotIndex = InItem.IsValid() ? InItem->SlotIndex : INDEX_NONE;

	if (SlotDetailsView.IsValid())
	{
		SlotDetailsView->SetObject(SelectedSlot.Get());

		if (DialogEditor)
		{
			DialogEditor->SelectDialogSlotActor(SelectedSlotIndex);
		}
	}
}

FReply SDialogStageManager::OnAddSlot()
{
	UDialogStage* DialogStage = SelectedDialogStage.Get();
	UDialogBuilderGraph* Graph = DialogGraph.Get();
	if (!DialogStage || !Graph)
	{
		return FReply::Handled();
	}

	const FScopedTransaction Transaction(LOCTEXT("AddSlotTransaction", "Add Slot"));
	DialogStage->Modify();

	UDialogSequenceSlot* NewSlot = NewObject<UDialogSequenceSlot>(DialogStage, NAME_None, RF_Transactional);
	if (NewSlot)
	{
		NewSlot->OwningDialogGraph = Graph;
		DialogStage->Slots.Add(NewSlot);

		MarkGraphDirty();
		RefreshSlotList();

		SelectedSlot = NewSlot;
		if (SlotDetailsView.IsValid())
		{
			SlotDetailsView->SetObject(NewSlot);
		}
	}

	return FReply::Handled();
}

FReply SDialogStageManager::OnAddCameraSlot()
{
	UDialogStage* DialogStage = SelectedDialogStage.Get();
	UDialogBuilderGraph* Graph = DialogGraph.Get();
	if (!DialogStage || !Graph)	
	{
		return FReply::Handled();
	}

	const FScopedTransaction Transaction(LOCTEXT("AddCameraSlotTransaction", "Add Camera Slot"));
	DialogStage->Modify();

	UDialogSequenceSlot* NewSlot = NewObject<UDialogSequenceSlot>(DialogStage, NAME_None, RF_Transactional);
	if (NewSlot)
	{
		NewSlot->OwningDialogGraph = Graph;
		DialogStage->CameraSlots.Add(NewSlot);

		MarkGraphDirty();
		RefreshSlotList();

		SelectedSlot = NewSlot;
		if (SlotDetailsView.IsValid())
		{
			SlotDetailsView->SetObject(NewSlot);
		}
	}

	return FReply::Handled();
}

void SDialogStageManager::OnRemoveSlot(UDialogSequenceSlot* InSlot, int32 InSlotIndex, bool bIsCameraSlot)
{
	UDialogStage* DialogStage = SelectedDialogStage.Get();
	if (!DialogStage || !InSlot)
	{
		return;
	}

	const FScopedTransaction Transaction(LOCTEXT("DeleteSlotTransaction", "Delete Slot"));
	DialogStage->Modify();
	InSlot->Modify();

	TArray<UDialogBuilderNode_DialogSequence*> SequenceNodes;
	DialogGraph->GetNodesOfType(SequenceNodes);

	for (UDialogBuilderNode_DialogSequence* SequenceNode : SequenceNodes)
	{
		if (SequenceNode && SequenceNode->DialogStageToUse == DialogStage)
		{
			UDialogSequence* DialogSequence = SequenceNode->DialogSequence;
			UDialogStage* DialogStageInstance = SequenceNode->DialogStage;
			if (DialogSequence && DialogStageInstance && DialogSequence->MovieScene)
			{
				UDialogSequenceSlot* SlotInSequence = DialogStageInstance->Slots.IsValidIndex(InSlotIndex) ? DialogStageInstance->Slots[InSlotIndex] : nullptr;
				
				if (SlotInSequence)
				{
					DialogSequence->MovieScene->RemovePossessable(SlotInSequence->ID);
				}
				
			}
		}
		
	}

	if (bIsCameraSlot)
	{
		if (DialogStage->CameraSlots.IsValidIndex(InSlotIndex))
		{
			DialogStage->CameraSlots.RemoveAt(InSlotIndex);
		}
		else
		{
			DialogStage->CameraSlots.RemoveSingle(InSlot);
		}
	}
	else
	{
		if (DialogStage->Slots.IsValidIndex(InSlotIndex))
		{
			DialogStage->Slots.RemoveAt(InSlotIndex);
		}
		else
		{
			DialogStage->Slots.RemoveSingle(InSlot);
		}
	}

	if (SelectedSlot.Get() == InSlot)
	{
		SelectedSlot.Reset();
		SelectedSlotIndex = INDEX_NONE;

		if (SlotDetailsView.IsValid())
		{
			SlotDetailsView->SetObject(nullptr);
		}
	}

	MarkGraphDirty();
	RefreshSlotList();
}

TSharedPtr<SWidget> SDialogStageManager::OnDialogStageContextMenuOpening()
{
	FMenuBuilder MenuBuilder(true, nullptr);		

	MenuBuilder.AddMenuEntry(
		LOCTEXT("AddDialogStageCtx", "Add Dialog Stage"),
		LOCTEXT("AddDialogStageCtxTooltip", "Add a new Dialog Stage"),
		FSlateIcon(),
		FUIAction(FExecuteAction::CreateLambda([this]()
		{
			OnAddDialogStage();
		}))
	);

	if (SelectedDialogStage.IsValid())
	{
		MenuBuilder.AddMenuEntry(
			LOCTEXT("DeleteDialogStageCtx", "Delete"),
			LOCTEXT("DeleteDialogStageCtxTooltip", "Delete selected Dialog Stage"),
			FSlateIcon(),
			FUIAction(FExecuteAction::CreateLambda([this]()
			{
				OnRemoveDialogStage(SelectedDialogStage.Get());
			}))
		);
	}

	return MenuBuilder.MakeWidget();
}

TSharedPtr<SWidget> SDialogStageManager::OnSlotContextMenuOpening()
{
	FMenuBuilder MenuBuilder(true, nullptr);

	MenuBuilder.AddMenuEntry(
		LOCTEXT("AddSlotCtx", "Add Slot"),
		LOCTEXT("AddSlotCtxTooltip", "Add Slot"),
		FSlateIcon(),
		FUIAction(FExecuteAction::CreateLambda([this]()
		{
			OnAddSlot();
		}))
	);

	/*MenuBuilder.AddMenuEntry(
		LOCTEXT("AddCameraSlotCtx", "Add Camera Slot"),
		LOCTEXT("AddCameraSlotCtxTooltip", "Add Camera Slot"),
		FSlateIcon(),
		FUIAction(FExecuteAction::CreateLambda([this]()
		{
			OnAddCameraSlot();
		}))
	);*/

	if (SelectedSlot.IsValid() && SelectedDialogStage.IsValid())
	{
		MenuBuilder.AddMenuEntry(
			LOCTEXT("DeleteSlotCtx", "Delete"),
			LOCTEXT("DeleteSlotCtxTooltip", "Delete selected slot"),
			FSlateIcon(),
			FUIAction(FExecuteAction::CreateLambda([this]()
			{
				OnRemoveSlot(SelectedSlot.Get(), SelectedSlotIndex, IsSelectedSlotCamera());
			}))
		);
	}

	return MenuBuilder.MakeWidget();
}

void SDialogStageManager::OnDeleteSelection()
{
	if (SelectedSlot.IsValid() && SelectedDialogStage.IsValid())
	{
		OnRemoveSlot(SelectedSlot.Get(), SelectedSlotIndex, IsSelectedSlotCamera());
		return;
	}

	if (SelectedDialogStage.IsValid())
	{
		OnRemoveDialogStage(SelectedDialogStage.Get());
	}
}

bool SDialogStageManager::CanDeleteSelection() const
{
	return SelectedSlot.IsValid() || SelectedDialogStage.IsValid();
}

bool SDialogStageManager::IsSelectedSlotCamera() const
{
	const UDialogStage* DialogStage = SelectedDialogStage.Get();
	const UDialogSequenceSlot* Slot = SelectedSlot.Get();

	return DialogStage && Slot && DialogStage->CameraSlots.Contains(const_cast<UDialogSequenceSlot*>(Slot));
}

void SDialogStageManager::Refresh()
{
	const FName CachedDialogStageObjectName = SelectedDialogStage.IsValid() ? SelectedDialogStage->GetFName() : NAME_None;
	const FGuid CachedSlotId = SelectedSlot.IsValid() ? SelectedSlot->ID : FGuid();

	RefreshDialogStageList();
	RestoreSelection(CachedDialogStageObjectName, CachedSlotId);
}

void SDialogStageManager::RefreshDialogStageList()
{
	DialogStageItems.Empty();

	if (UDialogBuilderGraph* Graph = DialogGraph.Get())
	{
		for (UDialogStage* DialogStage : Graph->DialogStages)
		{
			if (!DialogStage)
			{
				continue;
			}

			FDialogStageListItemPtr NewItem = MakeShared<FDialogStageListItem>();
			NewItem->DialogStage = DialogStage;
			DialogStageItems.Add(NewItem);
		}
	}

	if (DialogStageListView.IsValid())
	{
		DialogStageListView->RequestListRefresh();
	}
}

void SDialogStageManager::RefreshSlotList()
{
	SlotItems.Empty();

	UDialogStage* DialogStage = SelectedDialogStage.Get();
	if (DialogStage)
	{
		for (int32 SlotIndex = 0; SlotIndex < DialogStage->Slots.Num(); ++SlotIndex)
		{
			UDialogSequenceSlot* Slot = DialogStage->Slots[SlotIndex];
			if (!Slot)
			{
				continue;
			}

			FDialogSlotListItemPtr NewItem = MakeShared<FDialogSlotListItem>();
			NewItem->Slot = Slot;
			NewItem->bIsCameraSlot = false;
			NewItem->SlotIndex = SlotIndex;
			SlotItems.Add(NewItem);
		}

		for (int32 CameraSlotIndex = 0; CameraSlotIndex < DialogStage->CameraSlots.Num(); ++CameraSlotIndex)
		{
			UDialogSequenceSlot* Slot = DialogStage->CameraSlots[CameraSlotIndex];
			if (!Slot)
			{
				continue;
			}

			FDialogSlotListItemPtr NewItem = MakeShared<FDialogSlotListItem>();
			NewItem->Slot = Slot;
			NewItem->bIsCameraSlot = true;
			NewItem->SlotIndex = CameraSlotIndex;
			SlotItems.Add(NewItem);
		}
	}

	if (SlotListView.IsValid())
	{
		SlotListView->RequestListRefresh();
	}
}

void SDialogStageManager::MarkGraphDirty() const
{
	if (UDialogBuilderGraph* Graph = DialogGraph.Get())
	{
		Graph->Modify();
		if (UPackage* Package = Graph->GetOutermost())
		{
			Package->MarkPackageDirty();
		}
	}
}

FDialogStageListItemPtr SDialogStageManager::FindDialogStageItemByObjectName(const FName& InDialogStageObjectName) const
{
	if (InDialogStageObjectName.IsNone())
	{
		return nullptr;
	}

	for (const FDialogStageListItemPtr& Item : DialogStageItems)
	{
		if (Item.IsValid())
		{
			UDialogStage* DialogStage = Item->DialogStage.Get();
			if (DialogStage && DialogStage->GetFName() == InDialogStageObjectName)
			{
				return Item;
			}
		}
	}

	return nullptr;
}

FDialogSlotListItemPtr SDialogStageManager::FindSlotItemById(const FGuid& InSlotId) const
{
	if (!InSlotId.IsValid())
	{
		return nullptr;
	}

	for (const FDialogSlotListItemPtr& Item : SlotItems)
	{
		if (Item.IsValid())
		{
			UDialogSequenceSlot* Slot = Item->Slot.Get();
			if (Slot && Slot->ID == InSlotId)
			{
				return Item;
			}
		}
	}

	return nullptr;
}

void SDialogStageManager::SelectDialogStageByObjectName(const FName& InDialogStageObjectName)
{
	if (!DialogStageListView.IsValid())
	{
		return;
	}

	if (FDialogStageListItemPtr FoundItem = FindDialogStageItemByObjectName(InDialogStageObjectName))
	{
		DialogStageListView->SetSelection(FoundItem, ESelectInfo::Direct);
		DialogStageListView->RequestScrollIntoView(FoundItem);
	}
}

void SDialogStageManager::SelectSlotById(const FGuid& InSlotId)
{
	if (!SlotListView.IsValid())
	{
		return;
	}

	if (FDialogSlotListItemPtr FoundItem = FindSlotItemById(InSlotId))
	{
		SlotListView->SetSelection(FoundItem, ESelectInfo::Direct);
		SlotListView->RequestScrollIntoView(FoundItem);
	}
}

void SDialogStageManager::RestoreSelection(const FName& InDialogStageObjectName, const FGuid& InSlotId)
{
	SelectDialogStageByObjectName(InDialogStageObjectName);
	RefreshSlotList();
	SelectSlotById(InSlotId);
}

#undef LOCTEXT_NAMESPACE