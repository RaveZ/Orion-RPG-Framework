// Copyright 2025 Ivan Chandra. All Rights Reserved.


#include "DialogBuilderNode_Root.h"
#include "DialogBuilderNode.h"


#if WITH_EDITOR
void UDialogBuilderNode_Root::BeginNode()
{
	Super::BeginNode();
	EvaluateNextNode();
	Deinitialize();
}

FText UDialogBuilderNode_Root::GetNodeDescription() const
{
	return FText::FromString(TEXT("Root Node"));
}
#endif