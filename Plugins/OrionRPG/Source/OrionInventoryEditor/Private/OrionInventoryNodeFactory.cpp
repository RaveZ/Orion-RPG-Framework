// Copyright 2025 Ivan Chandra. All Rights Reserved.

#include "OrionInventoryNodeFactory.h"
#include <EdGraph/EdGraphNode.h>
#include "SGraphNode_OrionInventoryEdge.h"
#include "SGraphNode_OrionInventoryNode.h"
#include "OrionInventoryEdNode.h"
#include "OrionInventoryEdNode_Edge.h"


TSharedPtr<class SGraphNode> FOrionInventoryNodeFactory::CreateNode(UEdGraphNode* Node) const
{
	if (UOrionInventoryEdNode* EdNode_GraphNode = Cast<UOrionInventoryEdNode>(Node))
	{
		return SNew(SGraphNode_OrionInventoryNode, EdNode_GraphNode);
	}
	return nullptr;
}
