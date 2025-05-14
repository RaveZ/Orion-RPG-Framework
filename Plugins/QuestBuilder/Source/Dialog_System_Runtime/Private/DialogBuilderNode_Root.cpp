// Copyright 2025 Ivan Chandra. All Rights Reserved.


#include "DialogBuilderNode_Root.h"
#include "DialogBuilderNode.h"


#if WITH_EDITOR
FText UDialogBuilderNode_Root::GetNodeDescription() const
{
	return FText::FromString(TEXT("Root Node"));
}
#endif