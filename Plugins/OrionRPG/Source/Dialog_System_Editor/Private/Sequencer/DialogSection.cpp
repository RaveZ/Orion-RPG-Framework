// Copyright 2025 Ivan Chandra. All Rights Reserved.

#include "Sequencer/DialogSection.h"
#include "MovieSceneDialogSection.h"

#include "Widgets/Layout/SBorder.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/Text/STextBlock.h"
#include "Styling/AppStyle.h"
#include "SequencerSectionPainter.h"

FDialogSection::FDialogSection(UMovieSceneDialogSection& InSectionObject)
	: SectionObject(InSectionObject)
{
}

UMovieSceneSection* FDialogSection::GetSectionObject()
{
	return &SectionObject;
}

TSharedRef<SWidget> FDialogSection::GenerateSectionWidget()
{
	return SNew(SBorder)
		.BorderImage(FAppStyle::GetBrush("Sequencer.Section.Background"))
		.Padding(FMargin(4.0f, 2.0f))
		[
			SNew(STextBlock)
			.Text(FText())
		];
}

int32 FDialogSection::OnPaintSection(FSequencerSectionPainter& InPainter) const
{
	return InPainter.PaintSectionBackground();
}

FText FDialogSection::GetSectionTitle() const
{
	if (!SectionObject.DialogLine.Line.IsEmpty())
	{
		return SectionObject.DialogLine.Line;
	}

	return FText::FromString(TEXT("None"));
}