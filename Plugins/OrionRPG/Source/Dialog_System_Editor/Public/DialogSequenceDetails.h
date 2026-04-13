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

class FDialogSequenceDetails : public IDetailCustomization
{
public:
	/** Makes a new instance of this detail layout class for a specific detail view requesting it */
	static TSharedRef<IDetailCustomization> MakeInstance();

	/** IDetailCustomization interface */
	virtual void CustomizeDetails(IDetailLayoutBuilder& DetailBuilder) override;

protected:
	TSharedRef<SWidget> GetWidgetForParticipantPicker(const TAttribute<FText>& InParticipantNameText, FOnGetContent GetContentMenu);
	TSharedRef<SWidget> OnGetParticipantListMenu();
	void UseParticipantDefinition(UDialogParticipant* InParticipantDefinition);

private:
	TWeakObjectPtr<UDialogSequenceSlot> DialogSequenceSlot;
	TSharedPtr<IPropertyHandle> ActorTagPropertyHandle;
};