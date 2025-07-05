// Copyright 2025 Ivan Chandra. All Rights Reserved.


#include "UI/OrionUIManagerSubsystem.h"
#include "Framework/Application/SlateApplication.h"

UOrionUIManagerSubsystem::UOrionUIManagerSubsystem() : 
	OrionNavigationConfig(MakeShared<FOrionNavigationConfig>())
{
}

void UOrionUIManagerSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	FSlateApplication::Get().SetNavigationConfig(OrionNavigationConfig);
}

void UOrionUIManagerSubsystem::Deinitialize()
{
	Super::Deinitialize();
}
