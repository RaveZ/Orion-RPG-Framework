// Copyright 2025 Ivan Chandra. All Rights Reserved.

#include "Sequencer/DialogSection.h"
#include "MovieSceneDialogSection.h"

#include "Misc/PackageName.h"
#include "Widgets/DeclarativeSyntaxSupport.h"
#include "Styling/CoreStyle.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/Text/STextBlock.h"
#include "SequencerSectionPainter.h"
#include "SDropTarget.h"
#include "DragAndDrop/LevelDragDropOp.h"
#include "ScopedTransaction.h"
#include "Widgets/Layout/SBox.h"

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
	return SNew(SBox);
}

int32 FDialogSection::OnPaintSection(FSequencerSectionPainter& InPainter) const
{
	return InPainter.PaintSectionBackground();
}

FText FDialogSection::GetSectionTitle() const
{
	if (!SectionObject.DialogLineData.Line.IsEmpty())
	{
		return SectionObject.DialogLineData.Line;
	}

	return FText::FromString(TEXT("Dialog"));
}