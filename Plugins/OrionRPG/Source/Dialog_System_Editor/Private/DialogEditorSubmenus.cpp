#include "DialogEditorSubmenus.h"

#include "DialogBuilderViewportClient.h"
#include "SDialogPreviewViewport.h"
#include "EngineUtils.h"
#include "Styling/SlateIconFinder.h"
#include "EditorViewportCommands.h"
#include "Engine/SceneCapture.h"
#include "Engine/GameViewportClient.h"
#include "SortHelper.h"
#include "Camera/CameraActor.h"
#include "ToolMenu.h"
#include "ToolMenus.h"
#include "Styling/AppStyle.h"
#include "ViewportToolbar/UnrealEdViewportToolbarContext.h"

#define LOCTEXT_NAMESPACE "DialogEditorViewportToolbar"

namespace UE::DialogEditor::Private
{
	void GeneratePlacedCameraMenuEntries(
		FToolMenuSection& InSection, TArray<AActor*> InLookThroughActors, const TSharedPtr<::SDialogPreviewViewport>& InDialogViewport
	)
	{
		// Sort the cameras to make the ordering predictable for users.
		InLookThroughActors.StableSort(
			[](const AActor& Left, const AActor& Right)
			{
				// Do "natural sorting" via SceneOutliner::FNumericStringWrapper to make more sense to humans (also matches
				// the Scene Outliner). This sorts "Camera2" before "Camera10" which a normal lexicographical sort wouldn't.
				SceneOutliner::FNumericStringWrapper LeftWrapper(FString(Left.GetActorLabel()));
				SceneOutliner::FNumericStringWrapper RightWrapper(FString(Right.GetActorLabel()));

				return LeftWrapper < RightWrapper;
			}
		);

		for (AActor* LookThroughActor : InLookThroughActors)
		{
			// Needed for the delegate hookup to work below
			AActor* GenericActor = LookThroughActor;

			FText ActorDisplayName = FText::FromString(LookThroughActor->GetActorLabel());
			FUIAction LookThroughCameraAction(
				FExecuteAction::CreateSP(InDialogViewport.ToSharedRef(), &::SDialogPreviewViewport::OnActorLockToggleFromMenu, GenericActor),
				FCanExecuteAction(),
				FIsActionChecked::CreateSP(
					InDialogViewport.ToSharedRef(), &::SDialogPreviewViewport::IsActorLocked, MakeWeakObjectPtr(GenericActor)
				)
			);

			FSlateIcon ActorIcon;

			if (LookThroughActor->IsA<ACameraActor>() || LookThroughActor->IsA<ASceneCapture>())
			{
				ActorIcon = FSlateIcon(FAppStyle::GetAppStyleSetName(), "ClassIcon.CameraComponent");
			}
			else
			{
				ActorIcon = FSlateIconFinder::FindIconForClass(LookThroughActor->GetClass());
			}

			InSection.AddMenuEntry(
				NAME_None,
				ActorDisplayName,
				FText::Format(LOCTEXT("LookThroughCameraActor_ToolTip", "Look through and pilot {0}"), ActorDisplayName),
				ActorIcon,
				LookThroughCameraAction,
				EUserInterfaceActionType::RadioButton
			);
		}
	}

	static void AddCameraActorSelectSection(UToolMenu* InMenu)
	{
		if (!InMenu)
		{
			return;
		}

		UUnrealEdViewportToolbarContext* const ToolbarContext = InMenu->FindContext<UUnrealEdViewportToolbarContext>();
		if (!ToolbarContext)
		{
			return;
		}

		const TSharedPtr<SDialogPreviewViewport> DialogViewport = StaticCastSharedPtr<SDialogPreviewViewport>(ToolbarContext->Viewport.Pin());
		if (!DialogViewport.IsValid())
		{
			return;
		}

		TArray<AActor*> LookThroughActors;

		if (UWorld* World = DialogViewport->GetWorld())
		{
			for (TActorIterator<ACameraActor> It(World); It; ++It)
			{
				LookThroughActors.Add(Cast<AActor>(*It));
			}

			for (TActorIterator<ASceneCapture> It(World); It; ++It)
			{
				LookThroughActors.Add(Cast<AActor>(*It));
			}
		}

		FText CameraActorsHeading = LOCTEXT("CameraActorsHeading", "Cameras");

		FToolMenuInsert InsertPosition("LevelViewportCameraType_Perspective", EToolMenuInsertType::After);

		FToolMenuSection& Section = InMenu->AddSection("CameraActors");
		Section.InsertPosition = InsertPosition;


		// Don't add too many cameras to the top level menu or else it becomes too large
		constexpr uint32 MaxCamerasInTopLevelMenu = 10;
		if (LookThroughActors.Num() > MaxCamerasInTopLevelMenu)
		{
			FToolMenuEntry& Entry = Section.AddSubMenu(
				"CameraActors",
				CameraActorsHeading,
				LOCTEXT("LookThroughPlacedCameras_ToolTip", "Look through and pilot placed cameras"),
				FNewToolMenuDelegate::CreateLambda(
					[LookThroughActors, DialogViewportWeak = DialogViewport.ToWeakPtr()](UToolMenu* InMenu)
					{
						if (TSharedPtr<::SDialogPreviewViewport> DialogViewport = DialogViewportWeak.Pin())
						{
							FToolMenuSection& Section = InMenu->FindOrAddSection(NAME_None);
							UE::DialogEditor::Private::GeneratePlacedCameraMenuEntries(Section, LookThroughActors, DialogViewport);
						}
					}
				)
			);
			Entry.Icon = FSlateIconFinder::FindIconForClass(ACameraActor::StaticClass());
		}
		else if (!LookThroughActors.IsEmpty())
		{
			Section.AddSeparator(NAME_None);
			UE::DialogEditor::Private::GeneratePlacedCameraMenuEntries(Section, LookThroughActors, DialogViewport);
		}

		TWeakObjectPtr<AActor> LockedActorWeak = DialogViewport->GetDialogViewportClient().GetActorLock().LockedActor;

		if (TStrongObjectPtr<AActor> LockedActor = LockedActorWeak.Pin())
		{
			if (!LockedActor->IsA<ACameraActor>() && !LockedActor->IsA<ASceneCapture>())
			{
				UE::DialogEditor::Private::GeneratePlacedCameraMenuEntries(Section, { LockedActor.Get() }, DialogViewport);
			}
		}
	}


}

namespace UE::DialogEditor
{
	void ExtendCameraSubmenu(FName InCameraOptionsSubmenuName)
	{
		UToolMenu* const Submenu = UToolMenus::Get()->ExtendMenu(InCameraOptionsSubmenuName);
		if (!Submenu)
		{
			return;
		}

		Submenu->AddDynamicSection(
			"DialogEditorCameraExtensionDynamicSection",
			FNewToolMenuDelegate::CreateLambda(
				[](UToolMenu* InDynamicMenu)
				{
					Private::AddCameraActorSelectSection(InDynamicMenu);
				}
			)
		);
	}
}

#undef LOCTEXT_NAMESPACE