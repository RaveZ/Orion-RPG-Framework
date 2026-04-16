// Copyright 2025 Ivan Chandra. All Rights Reserved.

#include "Sequencer/DialogSection.h"
#include "MovieSceneDialogSection.h"

#include "Rendering/DrawElements.h"
#include "SequencerSectionPainter.h"
#include "Sound/DialogueWave.h"
#include "Sound/SoundBase.h"
#include "Sound/SoundCue.h"
#include "Sound/SoundNodeDialoguePlayer.h"
#include "Sound/SoundNodeWavePlayer.h"
#include "Sound/SoundWave.h"
#include "Styling/AppStyle.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/Text/STextBlock.h"

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

USoundWave* FDialogSection::DeriveSoundWave(USoundBase* InSound)
{
	if (!InSound)
	{
		return nullptr;
	}

	if (USoundWave* SoundWave = Cast<USoundWave>(InSound))
	{
		return SoundWave;
	}

	if (USoundCue* SoundCue = Cast<USoundCue>(InSound))
	{
		for (USoundNode* Node : SoundCue->AllNodes)
		{
			if (USoundNodeWavePlayer* WavePlayer = Cast<USoundNodeWavePlayer>(Node))
			{
				return WavePlayer->GetSoundWave();
			}

			if (USoundNodeDialoguePlayer* DialoguePlayer = Cast<USoundNodeDialoguePlayer>(Node))
			{
				if (UDialogueWave* DialogueWave = DialoguePlayer->GetDialogueWave())
				{
					if (DialogueWave->ContextMappings.Num() > 0)
					{
						return DialogueWave->ContextMappings[0].SoundWave;
					}
				}
			}
		}
	}

	return nullptr;
}

bool FDialogSection::BuildWaveEnvelope(USoundBase* InSound, TArray<float>& OutEnvelope) const
{
	OutEnvelope.Reset();

	USoundWave* SoundWave = DeriveSoundWave(InSound);
	if (!SoundWave || SoundWave->IsProcedurallyGenerated())
	{
		return false;
	}

	uint32 SampleRate = 0;
	uint16 NumChannels = 0;
	TArray<uint8> RawPCMData;
	if (!SoundWave->GetImportedSoundWaveData(RawPCMData, SampleRate, NumChannels))
	{
		return false;
	}

	if (RawPCMData.Num() < static_cast<int32>(sizeof(int16)) || NumChannels == 0)
	{
		return false;
	}

	const int16* PCM = reinterpret_cast<const int16*>(RawPCMData.GetData());
	const int32 SampleCount = RawPCMData.Num() / static_cast<int32>(sizeof(int16));
	const int32 FrameCount = SampleCount / static_cast<int32>(NumChannels);
	if (FrameCount <= 0)
	{
		return false;
	}

	const int32 BucketCount = FMath::Clamp(FrameCount / 256, 64, 1024);
	OutEnvelope.SetNumZeroed(BucketCount);

	for (int32 BucketIndex = 0; BucketIndex < BucketCount; ++BucketIndex)
	{
		const int32 StartFrame = (BucketIndex * FrameCount) / BucketCount;
		const int32 EndFrame = FMath::Max(StartFrame + 1, ((BucketIndex + 1) * FrameCount) / BucketCount);

		int32 PeakAbs = 0;

		for (int32 FrameIndex = StartFrame; FrameIndex < EndFrame; ++FrameIndex)
		{
			const int32 BaseSampleIndex = FrameIndex * static_cast<int32>(NumChannels);
			for (uint16 ChannelIndex = 0; ChannelIndex < NumChannels; ++ChannelIndex)
			{
				const int32 CurrentAbs = FMath::Abs(static_cast<int32>(PCM[BaseSampleIndex + ChannelIndex]));
				PeakAbs = FMath::Max(PeakAbs, CurrentAbs);
			}
		}

		OutEnvelope[BucketIndex] = FMath::Clamp(static_cast<float>(PeakAbs) / 32768.0f, 0.0f, 1.0f);
	}

	return OutEnvelope.Num() > 0;
}

int32 FDialogSection::OnPaintSection(FSequencerSectionPainter& InPainter) const
{
	int32 LayerId = InPainter.PaintSectionBackground();

	USoundBase* DialogSound = SectionObject.DialogLine.DialogSound;
	if (!DialogSound)
	{
		CachedSoundWave.Reset();
		CachedEnvelope.Reset();
		return LayerId;
	}

	USoundWave* CurrentSoundWave = DeriveSoundWave(DialogSound);
	if (!CurrentSoundWave)
	{
		CachedSoundWave.Reset();
		CachedEnvelope.Reset();
		return LayerId;
	}

	if (CachedSoundWave.Get() != CurrentSoundWave || CachedEnvelope.Num() == 0)
	{
		BuildWaveEnvelope(DialogSound, CachedEnvelope);
		CachedSoundWave = CurrentSoundWave;
	}

	if (CachedEnvelope.Num() == 0)
	{
		return LayerId;
	}

	const FVector2D SectionSize = InPainter.SectionGeometry.GetLocalSize();
	if (SectionSize.X < 8.0f || SectionSize.Y < 8.0f)
	{
		return LayerId;
	}

	const float MidY = SectionSize.Y * 0.5f;
	const float MaxAmplitude = SectionSize.Y * 0.42f;
	const int32 NumPoints = FMath::Clamp(FMath::RoundToInt(SectionSize.X / 2.0f), 32, 1024);

	TArray<FVector2D> TopPoints;
	TArray<FVector2D> BottomPoints;
	TopPoints.Reserve(NumPoints);
	BottomPoints.Reserve(NumPoints);

	for (int32 PointIndex = 0; PointIndex < NumPoints; ++PointIndex)
	{
		const float Alpha = NumPoints > 1 ? static_cast<float>(PointIndex) / static_cast<float>(NumPoints - 1) : 0.0f;
		const float X = Alpha * SectionSize.X;

		const int32 EnvelopeIndex = FMath::Clamp(
			FMath::RoundToInt(Alpha * static_cast<float>(CachedEnvelope.Num() - 1)),
			0,
			CachedEnvelope.Num() - 1);

		const float Amp = CachedEnvelope[EnvelopeIndex] * MaxAmplitude;

		TopPoints.Add(FVector2D(X, MidY - Amp));
		BottomPoints.Add(FVector2D(X, MidY + Amp));
	}

	const ESlateDrawEffect DrawEffects = InPainter.bParentEnabled ? ESlateDrawEffect::None : ESlateDrawEffect::DisabledEffect;
	const FLinearColor WaveColor = InPainter.GetSectionColor().CopyWithNewOpacity(0.95f);

	FSlateDrawElement::MakeLines(
		InPainter.DrawElements,
		++LayerId,
		InPainter.SectionGeometry.ToPaintGeometry(),
		TopPoints,
		DrawEffects,
		WaveColor,
		true,
		1.25f);

	FSlateDrawElement::MakeLines(
		InPainter.DrawElements,
		LayerId,
		InPainter.SectionGeometry.ToPaintGeometry(),
		BottomPoints,
		DrawEffects,
		WaveColor,
		true,
		1.25f);

	return LayerId;
}

FText FDialogSection::GetSectionTitle() const
{
	if (!SectionObject.DialogLine.Line.IsEmpty())
	{
		return SectionObject.DialogLine.Line;
	}

	return FText::FromString(TEXT("Dialog"));
}