#include "SDialogSetManager.h"

#include "DialogBuilderGraph.h"
#include "DialogBuilder_EditorStyle.h"
#include "DialogSet.h"
#include "PropertyEditorModule.h"
#include "IDetailsView.h"
#include "Modules/ModuleManager.h"
#include "ScopedTransaction.h"

#include "Framework/Commands/GenericCommands.h"
#include "Framework/MultiBox/MultiBoxBuilder.h"
#include "Styling/AppStyle.h"
#include "Widgets/SBoxPanel.h"
#include "Widgets/Images/SImage.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Text/SInlineEditableTextBlock.h"
#include "Widgets/Text/STextBlock.h"
#include "Widgets/Views/SListView.h"
#include "Widgets/Layout/SBorder.h"

#define LOCTEXT_NAMESPACE "DialogSetManager"

// -----------------------------------------------------------------------------
// Local palette row widgets (defined inside DialogSetManager, as requested)
// -----------------------------------------------------------------------------
class SDialogSetPaletteItem : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SDialogSetPaletteItem) {}
		SLATE_ARGUMENT(TWeakObjectPtr<UDialogSet>, DialogSet)
		SLATE_EVENT(FOnTextCommitted, OnNameCommitted)
	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs)
	{
		DialogSet = InArgs._DialogSet;
		OnNameCommitted = InArgs._OnNameCommitted;

		ChildSlot
		[
			SNew(SHorizontalBox)

			+ SHorizontalBox::Slot()
			.AutoWidth()
			.VAlign(VAlign_Center)
			.Padding(0.f, 0.f, 4.f, 0.f)
			[
				SNew(SImage).Image(FDialogBuilder_EditorStyle::Get().GetBrush("ClassIcon.Dialog"))
			]

			+ SHorizontalBox::Slot()
			.FillWidth(1.0f)
			.VAlign(VAlign_Center)
			[
				SNew(SInlineEditableTextBlock)
				.Text_Lambda([this]()
				{
					if (const UDialogSet* LocalSet = DialogSet.Get())
					{
						return LocalSet->Name.IsEmpty() ? LOCTEXT("UnnamedDialogSet", "Unnamed Dialog Set") : LocalSet->Name;
					}
					return LOCTEXT("InvalidDialogSet", "Invalid");
				})
				.OnTextCommitted(OnNameCommitted)
				.IsReadOnly(false)
			]
		];
	}

private:
	TWeakObjectPtr<UDialogSet> DialogSet;
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

			+ SHorizontalBox::Slot()
			.AutoWidth()
			.VAlign(VAlign_Center)
			.Padding(0.f, 0.f, 4.f, 0.f)
			[
				SNew(SImage)
				.Image(FDialogBuilder_EditorStyle::Get().GetBrush(bIsCameraSlot ? "ClassIcon.Dialog.Prop" : "ClassIcon.Dialog.Participant"))
			]

			+ SHorizontalBox::Slot()
			.FillWidth(1.0f)
			.VAlign(VAlign_Center)
			[
				SNew(STextBlock)
				.Text_Lambda([this]()
				{
					const UDialogSequenceSlot* LocalSlot = Slot.Get();
					if (!LocalSlot)
					{
						return LOCTEXT("InvalidSlot", "Invalid");
					}

					const FString Prefix = bIsCameraSlot ? TEXT("[Camera] ") : TEXT("[Slot] ");
					const FString Label = LocalSlot->ActorTag.IsValid()
						? LocalSlot->ActorTag.GetTagName().ToString()
						: LocalSlot->GetID().ToString();

					return FText::FromString(Prefix + Label);
				})
			]
		];
	}

private:
	TWeakObjectPtr<UDialogSequenceSlot> Slot;
	bool bIsCameraSlot = false;
};

// -----------------------------------------------------------------------------
// SDialogSetManager
// -----------------------------------------------------------------------------
void SDialogSetManager::Construct(const FArguments& InArgs, UDialogBuilderGraph* InDialogGraph)
{
	DialogGraph = InDialogGraph;

	CommandList = MakeShareable(new FUICommandList);
	CommandList->MapAction(
		FGenericCommands::Get().Delete,
		FExecuteAction::CreateSP(this, &SDialogSetManager::OnDeleteSelection),
		FCanExecuteAction::CreateSP(this, &SDialogSetManager::CanDeleteSelection));

	FPropertyEditorModule& PropertyModule = FModuleManager::LoadModuleChecked<FPropertyEditorModule>("PropertyEditor");

	FDetailsViewArgs DetailsArgs;
	DetailsArgs.NameAreaSettings = FDetailsViewArgs::HideNameArea;
	DetailsArgs.bUpdatesFromSelection = false;
	DetailsArgs.DefaultsOnlyVisibility = EEditDefaultsOnlyNodeVisibility::Hide;

	DialogSetDetailsView = PropertyModule.CreateDetailView(DetailsArgs);
	SlotDetailsView = PropertyModule.CreateDetailView(DetailsArgs);

	ChildSlot
	[
		SNew(SHorizontalBox)

		// 1) DialogSet List
		+ SHorizontalBox::Slot()
		.FillWidth(0.25f)
		.Padding(2.0f)
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
						SNew(STextBlock).Text(LOCTEXT("DialogSetListTitle", "Dialog Sets"))
					]
					+ SHorizontalBox::Slot()
					.AutoWidth()
					[
						SNew(SButton)
						.ButtonStyle(FAppStyle::Get(), "SimpleButton")
						.ContentPadding(FMargin(2.f))
						.ToolTipText(LOCTEXT("AddDialogSetTooltip", "Add Dialog Set"))
						.OnClicked(this, &SDialogSetManager::OnAddDialogSet)
						[
							SNew(SImage).Image(FAppStyle::GetBrush("Icons.Plus"))
						]
					]
				]

				+ SVerticalBox::Slot()
				.FillHeight(1.0f)
				.Padding(2.0f)
				[
					SAssignNew(DialogSetListView, SListView<FDialogSetListItemPtr>)
					.ListItemsSource(&DialogSetItems)
					.OnGenerateRow(this, &SDialogSetManager::GenerateDialogSetRow)
					.OnSelectionChanged(this, &SDialogSetManager::OnDialogSetSelectionChanged)
					.OnContextMenuOpening(this, &SDialogSetManager::OnDialogSetContextMenuOpening)
					.SelectionMode(ESelectionMode::Single)
				]
			]
		]

		// 2) DialogSet Details
		+ SHorizontalBox::Slot()
		.FillWidth(0.25f)
		.Padding(2.0f)
		[
			SNew(SBorder)
			.BorderImage(FAppStyle::GetBrush("ToolPanel.GroupBorder"))
			.Padding(4.0f)
			[
				DialogSetDetailsView.ToSharedRef()
			]
		]

		// 3) Slot List
		+ SHorizontalBox::Slot()
		.FillWidth(0.25f)
		.Padding(2.0f)
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
						SNew(STextBlock).Text(LOCTEXT("SlotListTitle", "Slots"))
					]
					+ SHorizontalBox::Slot()
					.AutoWidth()
					.Padding(0.f, 0.f, 2.f, 0.f)
					[
						SNew(SButton)
						.ButtonStyle(FAppStyle::Get(), "SimpleButton")
						.ContentPadding(FMargin(2.f))
						.ToolTipText(LOCTEXT("AddSlotTooltip", "Add Slot"))
						.OnClicked(this, &SDialogSetManager::OnAddSlot)
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
					.OnGenerateRow(this, &SDialogSetManager::GenerateSlotRow)
					.OnSelectionChanged(this, &SDialogSetManager::OnSlotSelectionChanged)
					.OnContextMenuOpening(this, &SDialogSetManager::OnSlotContextMenuOpening)
					.SelectionMode(ESelectionMode::Single)
				]
			]
		]

		// 4) Slot Details
		+ SHorizontalBox::Slot()
		.FillWidth(0.25f)
		.Padding(2.0f)
		[
			SNew(SBorder)
			.BorderImage(FAppStyle::GetBrush("ToolPanel.GroupBorder"))
			.Padding(4.0f)
			[
				SlotDetailsView.ToSharedRef()
			]
		]
	];

	RefreshDialogSetList();
}

FReply SDialogSetManager::OnKeyDown(const FGeometry& MyGeometry, const FKeyEvent& InKeyEvent)
{
	if (CommandList.IsValid() && CommandList->ProcessCommandBindings(InKeyEvent))
	{
		return FReply::Handled();
	}
	return FReply::Unhandled();
}

TSharedRef<ITableRow> SDialogSetManager::GenerateDialogSetRow(FDialogSetListItemPtr InItem, const TSharedRef<STableViewBase>& InOwnerTable)
{
	const TWeakObjectPtr<UDialogSet> WeakDialogSet = InItem.IsValid() ? InItem->DialogSet : nullptr;

	return SNew(STableRow<FDialogSetListItemPtr>, InOwnerTable)
	[
		SNew(SDialogSetPaletteItem)
		.DialogSet(WeakDialogSet)
		.OnNameCommitted(FOnTextCommitted::CreateSP(this, &SDialogSetManager::OnDialogSetNameCommitted, WeakDialogSet))
	];
}

void SDialogSetManager::OnDialogSetNameCommitted(const FText& InText, ETextCommit::Type InCommitType, TWeakObjectPtr<UDialogSet> InDialogSet)
{
	UDialogSet* DialogSet = InDialogSet.Get();
	if (!DialogSet)
	{
		return;
	}

	DialogSet->Modify();
	DialogSet->Name = InText;

	MarkGraphDirty();
	RefreshDialogSetList();
}

void SDialogSetManager::OnDialogSetSelectionChanged(FDialogSetListItemPtr InItem, ESelectInfo::Type InSelectInfo)
{
	SelectedDialogSet = InItem.IsValid() ? InItem->DialogSet.Get() : nullptr;
	SelectedSlot.Reset();

	if (DialogSetDetailsView.IsValid())
	{
		DialogSetDetailsView->SetObject(SelectedDialogSet.Get());
	}
	if (SlotDetailsView.IsValid())
	{
		SlotDetailsView->SetObject(nullptr);
	}

	RefreshSlotList();
}

FReply SDialogSetManager::OnAddDialogSet()
{
	UDialogBuilderGraph* Graph = DialogGraph.Get();
	if (!Graph)
	{
		return FReply::Handled();
	}

	const FScopedTransaction Transaction(LOCTEXT("AddDialogSetTransaction", "Add Dialog Set"));
	Graph->Modify();

	UDialogSet* NewDialogSet = NewObject<UDialogSet>(Graph, NAME_None, RF_Transactional);
	if (NewDialogSet)
	{
		NewDialogSet->OwningDialogGraph = Graph;
		NewDialogSet->Name = FText::FromString(FString::Printf(TEXT("DialogSet_%d"), Graph->DialogSets.Num() + 1));
		Graph->DialogSets.Add(NewDialogSet);

		MarkGraphDirty();
		RefreshDialogSetList();

		SelectedDialogSet = NewDialogSet;
		if (DialogSetDetailsView.IsValid())
		{
			DialogSetDetailsView->SetObject(NewDialogSet);
		}
		RefreshSlotList();
	}

	return FReply::Handled();
}

void SDialogSetManager::OnRemoveDialogSet(UDialogSet* InDialogSet)
{
	UDialogBuilderGraph* Graph = DialogGraph.Get();
	if (!Graph || !InDialogSet)
	{
		return;
	}

	const FScopedTransaction Transaction(LOCTEXT("DeleteDialogSetTransaction", "Delete Dialog Set"));
	Graph->Modify();
	InDialogSet->Modify();

	Graph->DialogSets.Remove(InDialogSet);

	if (SelectedDialogSet.Get() == InDialogSet)
	{
		SelectedDialogSet.Reset();
		SelectedSlot.Reset();

		if (DialogSetDetailsView.IsValid())
		{
			DialogSetDetailsView->SetObject(nullptr);
		}
		if (SlotDetailsView.IsValid())
		{
			SlotDetailsView->SetObject(nullptr);
		}
	}

	MarkGraphDirty();
	RefreshDialogSetList();
	RefreshSlotList();
}

TSharedRef<ITableRow> SDialogSetManager::GenerateSlotRow(FDialogSlotListItemPtr InItem, const TSharedRef<STableViewBase>& InOwnerTable)
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

void SDialogSetManager::OnSlotSelectionChanged(FDialogSlotListItemPtr InItem, ESelectInfo::Type InSelectInfo)
{
	SelectedSlot = InItem.IsValid() ? InItem->Slot.Get() : nullptr;

	if (SlotDetailsView.IsValid())
	{
		SlotDetailsView->SetObject(SelectedSlot.Get());
	}
}

FReply SDialogSetManager::OnAddSlot()
{
	UDialogSet* DialogSet = SelectedDialogSet.Get();
	UDialogBuilderGraph* Graph = DialogGraph.Get();
	if (!DialogSet || !Graph)
	{
		return FReply::Handled();
	}

	const FScopedTransaction Transaction(LOCTEXT("AddSlotTransaction", "Add Slot"));
	DialogSet->Modify();

	UDialogSequenceSlot* NewSlot = NewObject<UDialogSequenceSlot>(DialogSet, NAME_None, RF_Transactional);
	if (NewSlot)
	{
		NewSlot->OwningDialogGraph = Graph;
		DialogSet->Slots.Add(NewSlot);

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

FReply SDialogSetManager::OnAddCameraSlot()
{
	UDialogSet* DialogSet = SelectedDialogSet.Get();
	UDialogBuilderGraph* Graph = DialogGraph.Get();
	if (!DialogSet || !Graph)
	{
		return FReply::Handled();
	}

	const FScopedTransaction Transaction(LOCTEXT("AddCameraSlotTransaction", "Add Camera Slot"));
	DialogSet->Modify();

	UDialogSequenceSlot* NewSlot = NewObject<UDialogSequenceSlot>(DialogSet, NAME_None, RF_Transactional);
	if (NewSlot)
	{
		NewSlot->OwningDialogGraph = Graph;
		DialogSet->CameraSlots.Add(NewSlot);

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

void SDialogSetManager::OnRemoveSlot(UDialogSequenceSlot* InSlot, bool bIsCameraSlot)
{
	UDialogSet* DialogSet = SelectedDialogSet.Get();
	if (!DialogSet || !InSlot)
	{
		return;
	}

	const FScopedTransaction Transaction(LOCTEXT("DeleteSlotTransaction", "Delete Slot"));
	DialogSet->Modify();
	InSlot->Modify();

	if (bIsCameraSlot)
	{
		DialogSet->CameraSlots.Remove(InSlot);
	}
	else
	{
		DialogSet->Slots.Remove(InSlot);
	}

	if (SelectedSlot.Get() == InSlot)
	{
		SelectedSlot.Reset();
		if (SlotDetailsView.IsValid())
		{
			SlotDetailsView->SetObject(nullptr);
		}
	}

	MarkGraphDirty();
	RefreshSlotList();
}

TSharedPtr<SWidget> SDialogSetManager::OnDialogSetContextMenuOpening()
{
	FMenuBuilder MenuBuilder(true, nullptr);		

	MenuBuilder.AddMenuEntry(
		LOCTEXT("AddDialogSetCtx", "Add Dialog Set"),
		LOCTEXT("AddDialogSetCtxTooltip", "Add a new Dialog Set"),
		FSlateIcon(),
		FUIAction(FExecuteAction::CreateLambda([this]()
		{
			OnAddDialogSet();
		}))
	);

	if (SelectedDialogSet.IsValid())
	{
		MenuBuilder.AddMenuEntry(
			LOCTEXT("DeleteDialogSetCtx", "Delete"),
			LOCTEXT("DeleteDialogSetCtxTooltip", "Delete selected Dialog Set"),
			FSlateIcon(),
			FUIAction(FExecuteAction::CreateLambda([this]()
			{
				OnRemoveDialogSet(SelectedDialogSet.Get());
			}))
		);
	}

	return MenuBuilder.MakeWidget();
}

TSharedPtr<SWidget> SDialogSetManager::OnSlotContextMenuOpening()
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

	if (SelectedSlot.IsValid() && SelectedDialogSet.IsValid())
	{
		MenuBuilder.AddMenuEntry(
			LOCTEXT("DeleteSlotCtx", "Delete"),
			LOCTEXT("DeleteSlotCtxTooltip", "Delete selected slot"),
			FSlateIcon(),
			FUIAction(FExecuteAction::CreateLambda([this]()
			{
				OnRemoveSlot(SelectedSlot.Get(), IsSelectedSlotCamera());
			}))
		);
	}

	return MenuBuilder.MakeWidget();
}

void SDialogSetManager::OnDeleteSelection()
{
	if (SelectedSlot.IsValid() && SelectedDialogSet.IsValid())
	{
		OnRemoveSlot(SelectedSlot.Get(), IsSelectedSlotCamera());
		return;
	}

	if (SelectedDialogSet.IsValid())
	{
		OnRemoveDialogSet(SelectedDialogSet.Get());
	}
}

bool SDialogSetManager::CanDeleteSelection() const
{
	return SelectedSlot.IsValid() || SelectedDialogSet.IsValid();
}

bool SDialogSetManager::IsSelectedSlotCamera() const
{
	const UDialogSet* DialogSet = SelectedDialogSet.Get();
	const UDialogSequenceSlot* Slot = SelectedSlot.Get();

	return DialogSet && Slot && DialogSet->CameraSlots.Contains(const_cast<UDialogSequenceSlot*>(Slot));
}

void SDialogSetManager::Refresh()
{
	RefreshDialogSetList();
	RefreshSlotList();
}

void SDialogSetManager::RefreshDialogSetList()
{
	DialogSetItems.Empty();

	if (UDialogBuilderGraph* Graph = DialogGraph.Get())
	{
		for (UDialogSet* DialogSet : Graph->DialogSets)
		{
			if (!DialogSet)
			{
				continue;
			}

			FDialogSetListItemPtr NewItem = MakeShared<FDialogSetListItem>();
			NewItem->DialogSet = DialogSet;
			DialogSetItems.Add(NewItem);
		}
	}

	if (DialogSetListView.IsValid())
	{
		DialogSetListView->RequestListRefresh();
	}
}

void SDialogSetManager::RefreshSlotList()
{
	SlotItems.Empty();

	UDialogSet* DialogSet = SelectedDialogSet.Get();
	if (DialogSet)
	{
		for (UDialogSequenceSlot* Slot : DialogSet->Slots)
		{
			if (!Slot)
			{
				continue;
			}

			FDialogSlotListItemPtr NewItem = MakeShared<FDialogSlotListItem>();
			NewItem->Slot = Slot;
			NewItem->bIsCameraSlot = false;
			SlotItems.Add(NewItem);
		}

		for (UDialogSequenceSlot* Slot : DialogSet->CameraSlots)
		{
			if (!Slot)
			{
				continue;
			}

			FDialogSlotListItemPtr NewItem = MakeShared<FDialogSlotListItem>();
			NewItem->Slot = Slot;
			NewItem->bIsCameraSlot = true;
			SlotItems.Add(NewItem);
		}
	}

	if (SlotListView.IsValid())
	{
		SlotListView->RequestListRefresh();
	}
}

void SDialogSetManager::MarkGraphDirty() const
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

#undef LOCTEXT_NAMESPACE