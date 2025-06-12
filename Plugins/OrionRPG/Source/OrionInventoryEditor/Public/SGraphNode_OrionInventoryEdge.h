// Copyright 2025 Ivan Chandra. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "SGraphNode.h"
#include "Widgets/SCompoundWidget.h"
#include "OrionInventoryEdNode_Edge.h"

/**
 * 
 */
class ORIONINVENTORYEDITOR_API SGraphNode_OrionInventoryEdge : public SGraphNode
{
public:
	SLATE_BEGIN_ARGS(SGraphNode_OrionInventoryEdge)
	{}
	SLATE_END_ARGS()

	/** Constructs this widget with InArgs */
	void Construct(const FArguments& InArgs, UOrionInventoryEdNode_Edge* InNode);
};
