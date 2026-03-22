#include "DialogSequencePlaybackContext.h"
#include "SDialogPreviewViewport.h"
#include "DialogSequence.h"

#include "Editor.h"
#include "Widgets/SNullWidget.h"

FDialogSequencePlaybackContext::FDialogSequencePlaybackContext(UDialogSequence* InDialogSequence)
	: DialogSequence(InDialogSequence)
{
	UObject* OuterObject = nullptr;

	if (GEditor)
	{
		PlaybackContextObject = GEditor->GetEditorWorldContext().World();
		OuterObject = PlaybackContextObject.Get();
	}

	if (!OuterObject)
	{
		OuterObject = GetTransientPackage();
	}

	DialogContextObject = TStrongObjectPtr<UDialogPlaybackContextObject>(
		NewObject<UDialogPlaybackContextObject>(OuterObject)
	);
}

FDialogSequencePlaybackContext::~FDialogSequencePlaybackContext()
{
}

void FDialogSequencePlaybackContext::SetPlaybackContext(UObject* InPlaybackContextObject)
{
	PlaybackContextObject = InPlaybackContextObject;
}

void FDialogSequencePlaybackContext::SetDialogViewport(TSharedPtr<SDialogPreviewViewport> InDialogViewport)
{
	DialogViewport = InDialogViewport;

	if (DialogContextObject.IsValid())
	{
		DialogContextObject->SetDialogViewport(DialogViewport);
	}
}

TArray<UObject*> FDialogSequencePlaybackContext::GetEventContexts() const
{
	TArray<UObject*> Result;

	if (UObject* ContextObject = GetPlaybackContextAsObject())
	{
		Result.Add(ContextObject);
	}

	return Result;
}

UObject* FDialogSequencePlaybackContext::GetPlaybackContextAsObject() const
{
	if (DialogContextObject.IsValid())
	{
		return DialogContextObject.Get();
	}

	return PlaybackContextObject.IsValid() ? PlaybackContextObject.Get() : nullptr;
}

IMovieScenePlaybackClient* FDialogSequencePlaybackContext::GetPlaybackClientAsInterface() const
{
	// If needed later, return a real playback client object here.
	return nullptr;
}

TSharedRef<SWidget> FDialogSequencePlaybackContext::BuildWorldPickerCombo()
{
	// Optional UI; keep null widget for now.
	return SNullWidget::NullWidget;
}

void UDialogPlaybackContextObject::SetDialogViewport(TWeakPtr<SDialogPreviewViewport> InDialogViewport)
{
	DialogViewport = InDialogViewport;
}

void UDialogPlaybackContextObject::AddDialogToViewport(const TSharedRef<SWidget>& InWidget)
{
	if (TSharedPtr<SDialogPreviewViewport> Pinned = DialogViewport.Pin())
	{
		Pinned->AddDialogOverlayWidget(InWidget);
	}
}

void UDialogPlaybackContextObject::RemoveDialogFromViewport()
{
	if (TSharedPtr<SDialogPreviewViewport> Pinned = DialogViewport.Pin())
	{
		Pinned->RemoveDialogOverlayWidget();
	}
}