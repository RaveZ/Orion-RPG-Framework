#include "SDialogCameraPresets.h"

#include "DialogBuilderEditor.h"
#include "DialogBuilderGraph.h"
#include "DialogSequenceShot.h"
#include "AssetRegistry/AssetRegistryModule.h"
#include "AssetRegistry/IAssetRegistry.h"
#include "Engine/Blueprint.h"
#include "IDetailsView.h"
#include "Modules/ModuleManager.h"
#include "PropertyEditorModule.h"
#include "Styling/AppStyle.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/Layout/SSplitter.h"
#include "Widgets/SBoxPanel.h"
#include "Widgets/Text/STextBlock.h"

#define LOCTEXT_NAMESPACE "DialogCameraPresets"

namespace
{
	bool HasClassPathToken(const FString& InTagValue, const UClass* InClass)
	{
		if (!InClass)
		{
			return false;
		}

		const FString ClassName = InClass->GetName();
		const FString ClassPath = InClass->GetPathName();

		return InTagValue.Contains(ClassName) || InTagValue.Contains(ClassPath);
	}
}

void SDialogCameraPresets::Construct(const FArguments& InArgs, TWeakPtr<FDialogBuilderEditor> InDialogEditor)
{
	DialogEditorPtr = InDialogEditor;

	FPropertyEditorModule& PropertyModule = FModuleManager::LoadModuleChecked<FPropertyEditorModule>("PropertyEditor");

	FDetailsViewArgs DetailsArgs;
	DetailsArgs.NameAreaSettings = FDetailsViewArgs::HideNameArea;
	DetailsArgs.bUpdatesFromSelection = false;
	DetailsArgs.DefaultsOnlyVisibility = EEditDefaultsOnlyNodeVisibility::Hide;

	ShotDetailsView = PropertyModule.CreateDetailView(DetailsArgs);
	ShotDetailsView->OnFinishedChangingProperties().AddSP(this, &SDialogCameraPresets::OnShotPropertyChanged);

	ChildSlot
	[
		SNew(SVerticalBox)
		+ SVerticalBox::Slot()
		.FillHeight(1.0f)
		[
			SNew(SSplitter)
			.Orientation(EOrientation::Orient_Vertical)

			+ SSplitter::Slot()
			.Value(0.35f)
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
						SNew(STextBlock)
						.Text(LOCTEXT("CameraPresetListTitle", "Camera Presets"))
						.Font(FCoreStyle::GetDefaultFontStyle("Bold", 10.5f))
					]
					+ SVerticalBox::Slot()
					.FillHeight(1.0f)
					.Padding(2.0f)
					[
						SAssignNew(CameraPresetListView, SListView<FDialogCameraPresetItemPtr>)
						.ListItemsSource(&CameraPresetItems)
						.OnGenerateRow(this, &SDialogCameraPresets::GenerateCameraPresetRow)
						.OnSelectionChanged(this, &SDialogCameraPresets::OnCameraPresetSelectionChanged)
						.SelectionMode(ESelectionMode::Single)
					]
				]
			]
		]
	];

	RebuildCameraPresetList();
}

SDialogCameraPresets::~SDialogCameraPresets()
{
	ReleaseShotDuplicates();
}

void SDialogCameraPresets::Refresh()
{
	RebuildCameraPresetList();
}

TSharedRef<ITableRow> SDialogCameraPresets::GenerateCameraPresetRow(
	FDialogCameraPresetItemPtr InItem,
	const TSharedRef<STableViewBase>& InOwnerTable)
{
	return SNew(STableRow<FDialogCameraPresetItemPtr>, InOwnerTable)
	[
		SNew(SButton)
		.ButtonStyle(FAppStyle::Get(), "SimpleButton")
		.ContentPadding(FMargin(0.0f))
		.OnClicked_Lambda([this, InItem]()
		{
			if (!InItem.IsValid() || !InItem->ShotInstance.IsValid())
			{
				return FReply::Handled();
			}

			if (CameraPresetListView.IsValid() && CameraPresetListView->IsItemSelected(InItem))
			{
				ApplyCameraPresetItem(InItem);
			}
			else if (CameraPresetListView.IsValid())
			{
				CameraPresetListView->SetSelection(InItem, ESelectInfo::Direct);
			}

			return FReply::Handled();
		})
		[
			SNew(SBorder)
			.BorderImage(FAppStyle::GetBrush("NoBorder"))
			.Padding(FMargin(2.0f, 2.0f, 2.0f, 4.0f))
			[
				SNew(STextBlock)
				.Text(InItem.IsValid() ? InItem->DisplayName : LOCTEXT("InvalidCameraPreset", "Invalid"))
				.Font(FCoreStyle::GetDefaultFontStyle("Bold", 10.0f))
			]
		]
	];
}

void SDialogCameraPresets::OnCameraPresetSelectionChanged(FDialogCameraPresetItemPtr InItem, ESelectInfo::Type InSelectInfo)
{
	ApplyCameraPresetItem(InItem);
}

void SDialogCameraPresets::ApplyCameraPresetItem(FDialogCameraPresetItemPtr InItem)
{
	FDialogBuilderEditor* DialogEditor = DialogEditorPtr.Pin().Get();

	if (!InItem.IsValid() || !InItem->ShotInstance.IsValid())
	{
		ActiveShot.Reset();

		if (DialogEditor)
		{
			DialogEditor->SetDetailsObject(nullptr);
		}

		if (ShotDetailsView.IsValid())
		{
			ShotDetailsView->SetObject(nullptr);
		}

		return;
	}

	UDialogSequenceShot* Shot = InItem->ShotInstance.Get();
	ActiveShot = Shot;

	if (ShotDetailsView.IsValid())
	{
		ShotDetailsView->SetObject(Shot);
	}

	if (DialogEditor)
	{
		DialogEditor->SetDetailsObject(Shot);
		DialogEditor->ApplyCameraPreset(Shot);
	}
}

void SDialogCameraPresets::OnShotPropertyChanged(const FPropertyChangedEvent& PropertyChangedEvent)
{
	FDialogBuilderEditor* DialogEditor = DialogEditorPtr.Pin().Get();
	UDialogSequenceShot* Shot = ActiveShot.Get();
	if (!DialogEditor || !Shot)
	{
		return;
	}

	DialogEditor->ApplyCameraPreset(Shot);
}

void SDialogCameraPresets::RebuildCameraPresetList()
{
	ReleaseShotDuplicates();
	CameraPresetItems.Empty();

	FAssetRegistryModule& AssetRegistryModule = FModuleManager::LoadModuleChecked<FAssetRegistryModule>("AssetRegistry");
	IAssetRegistry& AssetRegistry = AssetRegistryModule.Get();

	TArray<FAssetData> Assets;
	AssetRegistry.GetAssetsByClass(UBlueprint::StaticClass()->GetClassPathName(), Assets);

	for (const FAssetData& AssetData : Assets)
	{
		if (!IsCameraPresetBlueprint(AssetData))
		{
			continue;
		}

		UBlueprint* Blueprint = Cast<UBlueprint>(AssetData.GetAsset());
		if (!Blueprint || !Blueprint->GeneratedClass)
		{
			continue;
		}

		TStrongObjectPtr<UDialogSequenceShot> SelectedShot(CreateSelectedShotInstance(AssetData));
		if (!SelectedShot.IsValid())
		{
			continue;
		}

		ShotDuplicates.Add(MoveTemp(SelectedShot));

		FDialogCameraPresetItemPtr Item = MakeShared<FDialogCameraPresetItem>();
		Item->DisplayName = !Blueprint->BlueprintDisplayName.IsEmpty()
			? FText::FromString(Blueprint->BlueprintDisplayName)
			: FText::FromName(AssetData.AssetName);
		Item->AssetData = AssetData;
		Item->ShotInstance = ShotDuplicates.Last();

		CameraPresetItems.Add(Item);
	}

	CameraPresetItems.Sort([](const FDialogCameraPresetItemPtr& A, const FDialogCameraPresetItemPtr& B)
	{
		const FString AName = A.IsValid() ? A->DisplayName.ToString() : FString();
		const FString BName = B.IsValid() ? B->DisplayName.ToString() : FString();
		return AName < BName;
	});

	if (CameraPresetListView.IsValid())
	{
		CameraPresetListView->RequestListRefresh();

		if (CameraPresetItems.Num() > 0)
		{
			CameraPresetListView->SetSelection(CameraPresetItems[0], ESelectInfo::Direct);
		}
	}
}

bool SDialogCameraPresets::IsCameraPresetBlueprint(const FAssetData& AssetData) const
{
	FString ParentClassPath;

	if (!AssetData.GetTagValue(FName(TEXT("NativeParentClassPath")), ParentClassPath) &&
		!AssetData.GetTagValue(FName(TEXT("ParentClass")), ParentClassPath))
	{
		return false;
	}

	return HasClassPathToken(ParentClassPath, UDialogSequenceShot::StaticClass());
}

UDialogSequenceShot* SDialogCameraPresets::CreateSelectedShotInstance(const FAssetData& AssetData) const
{
	UBlueprint* Blueprint = Cast<UBlueprint>(AssetData.GetAsset());
	if (!Blueprint || !Blueprint->GeneratedClass)
	{
		return nullptr;
	}

	UDialogSequenceShot* SourceShot = Cast<UDialogSequenceShot>(Blueprint->GeneratedClass->GetDefaultObject());
	if (!SourceShot)
	{
		return nullptr;
	}

	UDialogSequenceShot* OutShot = NewObject<UDialogSequenceShot>(
		GetTransientPackage(), Blueprint->GeneratedClass, NAME_None, RF_Transient | RF_Transactional);

	FDialogBuilderEditor* DialogEditor = DialogEditorPtr.Pin().Get();
	UDialogBuilderGraph* DialogGraph = DialogEditor ? DialogEditor->GetDialogBuilderGraph() : nullptr;
	OutShot->OwningDialogGraph = DialogGraph;
	return OutShot;
}

void SDialogCameraPresets::ReleaseShotDuplicates()
{
	if (ShotDetailsView.IsValid())
	{
		ShotDetailsView->SetObject(nullptr);
	}

	ShotDuplicates.Empty();
	CameraPresetItems.Empty();
}

#undef LOCTEXT_NAMESPACE