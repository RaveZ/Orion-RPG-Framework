#pragma once

#include "CoreMinimal.h"
#include "UObject/WeakObjectPtr.h"
#include "Templates/SharedPointer.h"
#include "OrionDialogInterface.h"
#include "UObject/Object.h"
#include "DialogSequencePlaybackContext.generated.h"

class UDialogSequence;
class SDialogPreviewViewport;
class IMovieScenePlaybackClient;
class SWidget;
class UDialogPlaybackContextObject;
class UWorld;

class FDialogSequencePlaybackContext : public TSharedFromThis<FDialogSequencePlaybackContext>
{
public:
	FDialogSequencePlaybackContext(UDialogSequence* InDialogSequence);
	~FDialogSequencePlaybackContext();

	void SetPlaybackContext(UObject* InPlaybackContextObject);
	void SetDialogViewport(TSharedPtr<SDialogPreviewViewport> InDialogViewport);

	TArray<UObject*> GetEventContexts() const;
	UObject* GetPlaybackContextAsObject() const;
	IMovieScenePlaybackClient* GetPlaybackClientAsInterface() const;

	TSharedRef<SWidget> BuildWorldPickerCombo();

private:
	void RefreshDialogContextObject();
	UObject* ResolvePlaybackContextOuter() const;

	void OnMapChange(uint32);
	void OnPieEvent(bool);
	void OnWorldListChanged(UWorld*);

private:
	TWeakPtr<SDialogPreviewViewport> DialogViewport;
	TWeakObjectPtr<UDialogSequence> DialogSequence;
	TWeakObjectPtr<UObject> PlaybackContextObject;

	TStrongObjectPtr<UDialogPlaybackContextObject> DialogContextObject;

	bool bUseManualPlaybackContext = false;
};



class SDialogPreviewViewport;

UCLASS(Transient)
class DIALOG_SYSTEM_EDITOR_API UDialogPlaybackContextObject : public UObject, public IDialogContext
{
	GENERATED_BODY()

public:
	void SetDialogViewport(TWeakPtr<SDialogPreviewViewport> InDialogViewport);

	virtual void AddDialogToViewport(const TSharedRef<SWidget>& InWidget) override;
	virtual void RemoveDialogFromViewport() override;

private:
	TWeakPtr<SDialogPreviewViewport> DialogViewport;
};