// Copyright 2025 Ivan Chandra. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Framework/SlateDelegates.h"
#include "IDetailCustomization.h"
#include "Misc/Attribute.h"
#include "Templates/SharedPointer.h"
#include "UObject/WeakObjectPtr.h"
#include "UObject/WeakObjectPtrTemplates.h"

class FText;
class IDetailLayoutBuilder;
class IPropertyHandle;
class SWidget;
class UDialogParticipant;
class UDialogSequenceSlot;
class UMovieSceneDialogSection;

class FDialogSequenceDetails : public IDetailCustomization
{
public:
	/** Makes a new instance of this detail layout class for a specific detail view requesting it */
	static TSharedRef<IDetailCustomization> MakeInstance();

	/** IDetailCustomization interface */
	virtual void CustomizeDetails(IDetailLayoutBuilder& DetailBuilder) override;

protected:
	// DialogSequenceSlot
	TSharedRef<SWidget> GetWidgetForSlotParticipantPicker(const TAttribute<FText>& InParticipantNameText, FOnGetContent GetContentMenu);
	TSharedRef<SWidget> OnGetSlotParticipantListMenu();
	void UseSlotParticipantDefinition(UDialogParticipant* InParticipantDefinition);

	// MovieSceneDialogSection - Speaker
	TSharedRef<SWidget> GetWidgetForSpeakerParticipantPicker(const TAttribute<FText>& InParticipantNameText, FOnGetContent GetContentMenu);
	TSharedRef<SWidget> OnGetSpeakerParticipantListMenu();
	void UseSpeakerParticipantDefinition(UDialogParticipant* InParticipantDefinition);

	// MovieSceneDialogSection - Listener
	TSharedRef<SWidget> GetWidgetForListenerParticipantPicker(const TAttribute<FText>& InParticipantNameText, FOnGetContent GetContentMenu);
	TSharedRef<SWidget> OnGetListenerParticipantListMenu();
	void UseListenerParticipantDefinition(UDialogParticipant* InParticipantDefinition);

private:
	FText GetSlotParticipantDisplayText() const;
	FText GetSectionSpeakerDisplayText() const;
	FText GetSectionListenerDisplayText() const;

private:
	TWeakObjectPtr<UDialogSequenceSlot> DialogSequenceSlot;
	TWeakObjectPtr<UMovieSceneDialogSection> MovieSceneDialogSection;

	TSharedPtr<IPropertyHandle> ActorTagPropertyHandle;
	TSharedPtr<IPropertyHandle> DialogLinePropertyHandle;
	TSharedPtr<IPropertyHandle> SpeakerTagPropertyHandle;
	TSharedPtr<IPropertyHandle> ListenerTagPropertyHandle;
};