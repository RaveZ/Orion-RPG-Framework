#include "UI/OrionDialogWidget.h"

void UOrionDialogWidget::SetDialogData(
	const FText& InSpeakerName,
	const FText& InDialogLine,
	const FGameplayTag& InSpeakerParticipantTag,
	const FGameplayTag& InListenerParticipantTag)
{
	SpeakerName = InSpeakerName;
	DialogLine = InDialogLine;
	SpeakerParticipantTag = InSpeakerParticipantTag;
	ListenerParticipantTag = InListenerParticipantTag;

	BP_OnDialogDataUpdated();
}