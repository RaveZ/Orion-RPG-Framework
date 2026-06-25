// Copyright 2025 Ivan Chandra. All Rights Reserved.


#include "EdGraphSchema_OrionInventory.h"
#include "ToolMenus.h"
#include "OrionInventory_EditorCommands.h"
#include "OrionInventoryEditorModule.h"
#include "EdGraphNode_Comment.h"
#include "AIGraphTypes.h"
#include "Framework/Commands/GenericCommands.h"
#include "Settings/EditorStyleSettings.h"
#include "OrionInventoryEditorUtils.h"
#include "Inventory/OrionInventoryNode_ItemContainer.h"
#include "OrionInventoryEdNode_ItemContainer.h"
#include "OrionInventoryEdSubNode_Item.h"
#include "OrionInventoryEdNode.h"
#include "GraphEditorActions.h"
#include "Decorator/OrionDecorator.h"
#include "Event/OrionEvent.h"


#define LOCTEXT_NAMESPACE "EdGraphSchema_OrionInventory"

namespace
{
	// Maximum distance a drag can be off a node edge to require 'push off' from node
	const int32 NodeDistance = 60;
}

int32 UEdGraphSchema_OrionInventory::CurrentCacheRefreshID = 0;

UEdGraphNode* FAssetSchemaAction_Inventory_NewNode::PerformAction(UEdGraph* ParentGraph, UEdGraphPin* FromPin, const FVector2D Location, bool bSelectNewNode)
{
	UEdGraphNode* ResultNode = NULL;

	//// If there is a template, we actually use it
	if (NodeTemplate != NULL)
	{
		const FScopedTransaction Transaction(LOCTEXT("AddNode", "Add Node"));
		ParentGraph->Modify();
		if (FromPin)
		{
			FromPin->Modify();
		}

		NodeTemplate->SetFlags(RF_Transactional);

	//	// set outer to be the graph so it doesn't go away
		NodeTemplate->Rename(NULL, ParentGraph, REN_NonTransactional);
		ParentGraph->AddNode(NodeTemplate, true);

		NodeTemplate->CreateNewGuid();
		NodeTemplate->PostPlacedNewNode();

		// For input pins, new node will generally overlap node being dragged off
		// Work out if we want to visually push away from connected node
		int32 XLocation = Location.X;
		if (FromPin && FromPin->Direction == EGPD_Input)
		{
			UEdGraphNode* PinNode = FromPin->GetOwningNode();
			const float XDelta = FMath::Abs(PinNode->NodePosX - Location.X);

			if (XDelta < NodeDistance)
			{
				// Set location to edge of current node minus the max move distance
				// to force node to push off from connect node enough to give selection handle
				XLocation = PinNode->NodePosX - NodeDistance;
			}
		}

		NodeTemplate->NodePosX = XLocation;
		NodeTemplate->NodePosY = Location.Y;
		NodeTemplate->SnapToGrid(GetDefault<UEditorStyleSettings>()->GridSnapSize);

		// setup pins after placing node in correct spot, since pin sorting will happen as soon as link connection change occurs
		NodeTemplate->AllocateDefaultPins();
		NodeTemplate->AutowireNewNode(FromPin);

		ResultNode = NodeTemplate;
	}

	return ResultNode;

}

void FAssetSchemaAction_Inventory_NewNode::AddReferencedObjects(FReferenceCollector& Collector)
{
	FEdGraphSchemaAction::AddReferencedObjects(Collector);
	Collector.AddReferencedObject(NodeTemplate);
}


UEdGraphSchema_OrionInventory::UEdGraphSchema_OrionInventory()
{
	InventoryItemClass = UOrionInventoryEdSubNode_Item::StaticClass();
}

void UEdGraphSchema_OrionInventory::GetBreakLinkToSubMenuActions(UToolMenu* Menu, UEdGraphPin* InGraphPin)
{
	// Make sure we have a unique name for every entry in the list
	TMap< FString, uint32 > LinkTitleCount;

	FToolMenuSection& Section = Menu->FindOrAddSection("OrionInventoryAssetGraphSchemaPinActions");

	// Add all the links we could break from
	for (TArray<class UEdGraphPin*>::TConstIterator Links(InGraphPin->LinkedTo); Links; ++Links)
	{
		UEdGraphPin* Pin = *Links;
		FString TitleString = Pin->GetOwningNode()->GetNodeTitle(ENodeTitleType::ListView).ToString();
		FText Title = FText::FromString(TitleString);
		if (Pin->PinName != TEXT(""))
		{
			TitleString = FString::Printf(TEXT("%s (%s)"), *TitleString, *Pin->PinName.ToString());

			// Add name of connection if possible
			FFormatNamedArguments Args;
			Args.Add(TEXT("NodeTitle"), Title);
			Args.Add(TEXT("PinName"), Pin->GetDisplayName());
			Title = FText::Format(LOCTEXT("BreakDescPin", "{NodeTitle} ({PinName})"), Args);
		}

		uint32& Count = LinkTitleCount.FindOrAdd(TitleString);

		FText Description;
		FFormatNamedArguments Args;
		Args.Add(TEXT("NodeTitle"), Title);
		Args.Add(TEXT("NumberOfNodes"), Count);

		if (Count == 0)
		{
			Description = FText::Format(LOCTEXT("BreakDesc", "Break link to {NodeTitle}"), Args);
		}
		else
		{
			Description = FText::Format(LOCTEXT("BreakDescMulti", "Break link to {NodeTitle} ({NumberOfNodes})"), Args);
		}
		++Count;

		Section.AddMenuEntry(NAME_None, Description, Description, FSlateIcon(), FUIAction(
			FExecuteAction::CreateUObject(this, &UEdGraphSchema_OrionInventory::BreakSinglePinLink, const_cast<UEdGraphPin*>(InGraphPin), *Links)));
	}
}

void UEdGraphSchema_OrionInventory::CreateDefaultNodesForGraph(UEdGraph& Graph) const
{
	/*UOrionInventoryGraph* InventoryGraph = CastChecked<UOrionInventoryGraph>(Graph.GetOuter());

	FGraphNodeCreator<UOrionInventoryEdNode_Root> NodeCreator(Graph);
	UOrionInventoryEdNode_Root* RootNode = NodeCreator.CreateNode();
	RootNode->NodeInstance = NewObject<UObject>(InventoryGraph, UOrionInventoryNode_Root::StaticClass());
	RootNode->NodeInstance->SetFlags(RF_Transactional);
	NodeCreator.Finalize();
	SetNodeMetaData(RootNode, FNodeMetadata::DefaultGraphNode);*/
}

EGraphType UEdGraphSchema_OrionInventory::GetGraphType(const UEdGraph* TestEdGraph) const
{
	return GT_StateMachine;
}

void UEdGraphSchema_OrionInventory::GetGraphContextActions(FGraphContextMenuBuilder& ContextMenuBuilder) const
{
	UOrionInventoryGraph* InventoryGraph = CastChecked<UOrionInventoryGraph>(ContextMenuBuilder.CurrentGraph->GetOuter());
	UEdGraph* EdGraph = (UEdGraph*)ContextMenuBuilder.CurrentGraph;

	
	FCategorizedGraphActionListBuilder ItemContainerBuilder(TEXT("ItemContainer"));
	UClass* ItemContainerEdNodeClass = UOrionInventoryEdNode_ItemContainer::StaticClass();
	UClass* ItemContainerNodeClass = UOrionInventoryNode_ItemContainer::StaticClass();

	TSharedPtr<FAssetSchemaAction_Inventory_NewNode> AddOpAction = UEdGraphSchema_OrionInventory::AddNewNodeAction(ItemContainerBuilder, FText::GetEmpty(), LOCTEXT("InventorySystemGraphNodeAction", "Add Item Container."), LOCTEXT("InventorySystemGraphNodeTooltip", "Create Item Container."));
	UOrionInventoryEdNode* OpNode = NewObject<UOrionInventoryEdNode>(ContextMenuBuilder.OwnerOfTemporaries, ItemContainerEdNodeClass);
	FGraphNodeClassData ItemContainerClassData = FGraphNodeClassData(ItemContainerNodeClass, "ItemContainerClassData");
	OpNode->ClassData = ItemContainerClassData;
	AddOpAction->NodeTemplate = OpNode;
	OpNode->NodeInstance = NewObject<UObject>(InventoryGraph, ItemContainerNodeClass);
	OpNode->NodeInstance->SetFlags(RF_Transactional);

	ContextMenuBuilder.Append(ItemContainerBuilder);
	
	
	// Add the ability to create a comment to the context menu too for discoverability
	{
		TSharedPtr<FInventorySchemaAction_AddComment> Action = TSharedPtr<FInventorySchemaAction_AddComment>(
			new FInventorySchemaAction_AddComment(LOCTEXT("AddComment", "Add Comment"), LOCTEXT("AddComment_Tooltip", "Adds a comment node to the graph."))
		);

		ContextMenuBuilder.AddAction(Action);
	}
}



void UEdGraphSchema_OrionInventory::GetGraphNodeContextActions(FGraphContextMenuBuilder& ContextMenuBuilder, int32 SubNodeFlags) const
{
	UEdGraph* Graph = (UEdGraph*)ContextMenuBuilder.CurrentGraph;
	UClass* GraphNodeClass = nullptr;
	TArray<FGraphNodeClassData> NodeClasses;
	GetSubNodeClasses(SubNodeFlags, NodeClasses, GraphNodeClass);

	if (GraphNodeClass)
	{
		for (const auto& NodeClass : NodeClasses)
		{
			const FText NodeTypeName = FText::FromString(FName::NameToDisplayString(NodeClass.ToString(), false));

			UOrionInventoryEdNode* OpNode = NewObject<UOrionInventoryEdNode>(Graph, GraphNodeClass);
			OpNode->ClassData = NodeClass;

			TSharedPtr<FAssetSchemaAction_Inventory_NewSubNode> AddOpAction = UEdGraphSchema_OrionInventory::AddNewSubNodeAction(ContextMenuBuilder, NodeClass.GetCategory(), NodeTypeName, NodeClass.GetTooltip());
			AddOpAction->ParentNode = Cast<UOrionInventoryEdNode>(ContextMenuBuilder.SelectedObjects[0]);
			AddOpAction->NodeTemplate = OpNode;
		}
	}

}

void UEdGraphSchema_OrionInventory::GetSubNodeClasses(int32 SubNodeFlags, TArray<FGraphNodeClassData>& ClassData, UClass*& GraphNodeClass) const
{
	FGraphNodeClassHelper& ClassCache = GetClassCache(SubNodeFlags);

	if (SubNodeFlags == EInventorySubNode::Item)
	{
		ClassCache.GatherClasses(UOrionDecorator::StaticClass(), ClassData);
		GraphNodeClass = InventoryItemClass;
	}
}


FGraphNodeClassHelper& UEdGraphSchema_OrionInventory::GetClassCache(int32 SubNodeFlags) const
{
	FOrionInventoryEditorModule& EditorModule = FModuleManager::GetModuleChecked<FOrionInventoryEditorModule>(TEXT("OrionInventoryEditor"));
	FGraphNodeClassHelper* ClassHelper = nullptr;
	if (SubNodeFlags == EInventorySubNode::Item)
	{
		ClassHelper = EditorModule.GetInventoryItemClassCache().Get();
	}
	
	
	check(ClassHelper);
	return *ClassHelper;
}

void UEdGraphSchema_OrionInventory::GetContextMenuActions(UToolMenu* Menu, UGraphNodeContextMenuContext* Context) const
{
	if (Context->Pin) {
		FToolMenuSection& Section = Menu->AddSection("InventorySystemGraphAssetGraphSchemaNodeActions", LOCTEXT("PinActionsMenuHeader", "Pin Actions"));
		// Only display the 'Break Links' option if there is a link to break!
			if (Context->Pin->LinkedTo.Num() > 0)
			{
				Section.AddMenuEntry(FGraphEditorCommands::Get().BreakPinLinks);

				// add sub menu for break link to
				if (Context->Pin->LinkedTo.Num() > 1)
				{
					Section.AddSubMenu(
						"BreakLinkTo",
						LOCTEXT("BreakLinkTo", "Break Link To..."),
						LOCTEXT("BreakSpecificLinks", "Break a specific link..."),
						FNewToolMenuDelegate::CreateUObject((UEdGraphSchema_OrionInventory* const)this, &UEdGraphSchema_OrionInventory::GetBreakLinkToSubMenuActions, const_cast<UEdGraphPin*>(Context->Pin)));
				}
				else
				{
					((UEdGraphSchema_OrionInventory* const)this)->GetBreakLinkToSubMenuActions(Menu, const_cast<UEdGraphPin*>(Context->Pin));
				}
			}
	}
	else if (Context->Node)
	{
		{
			FToolMenuSection& Section = Menu->AddSection("OrionInventoryAssetGraphSchemaNodeActions", LOCTEXT("ClassActionsMenuHeader", "Node Actions"));
			Section.AddMenuEntry(FOrionInventory_EditorCommands::Get().AddInventoryItem);
			Section.AddMenuEntry(FGenericCommands::Get().Delete);
			Section.AddMenuEntry(FGenericCommands::Get().Cut);
			Section.AddMenuEntry(FGenericCommands::Get().Copy);
			Section.AddMenuEntry(FGenericCommands::Get().Duplicate);

		}
	}

	Super::GetContextMenuActions(Menu, Context);
}

void UEdGraphSchema_OrionInventory::CreateAddInventoryItemSubMenu(UToolMenu* Menu, UEdGraph* Graph) const
{
	
}


const FPinConnectionResponse UEdGraphSchema_OrionInventory::CanMergeNodes(const UEdGraphNode* NodeA, const UEdGraphNode* NodeB) const
{
	// Make sure the nodes are not the same 
	if (NodeA == NodeB)
	{
		return FPinConnectionResponse(CONNECT_RESPONSE_DISALLOW, TEXT("Both are the same node"));
	}

	const UOrionInventoryEdNode* InventoryEdNodeA = Cast<UOrionInventoryEdNode>(NodeA);
	const UOrionInventoryEdNode* InventoryEdNodeB = Cast<UOrionInventoryEdNode>(NodeB);

	const bool bNodeAIsItemContainer = NodeA->IsA(UOrionInventoryEdNode_ItemContainer::StaticClass());
	const bool bNodeAIsItem = NodeA->IsA(UOrionInventoryEdSubNode_Item::StaticClass());
	const bool bNodeBIsItemContainer = NodeB->IsA(UOrionInventoryEdNode_ItemContainer::StaticClass());
	const bool bNodeBIsItem = NodeB->IsA(UOrionInventoryEdSubNode_Item::StaticClass());

	if ((bNodeAIsItem && (bNodeBIsItem || bNodeBIsItemContainer)))
	{
		return FPinConnectionResponse(CONNECT_RESPONSE_MAKE, TEXT(""));
	}
	return FPinConnectionResponse(CONNECT_RESPONSE_DISALLOW, TEXT(""));
}


bool UEdGraphSchema_OrionInventory::IsCacheVisualizationOutOfDate(int32 InVisualizationCacheID) const
{
	return CurrentCacheRefreshID != InVisualizationCacheID;
}

int32 UEdGraphSchema_OrionInventory::GetCurrentVisualizationCacheID() const
{
	return CurrentCacheRefreshID;
}

void UEdGraphSchema_OrionInventory::ForceVisualizationCacheClear() const
{
	++CurrentCacheRefreshID;
}

TSharedPtr<FEdGraphSchemaAction> UEdGraphSchema_OrionInventory::GetCreateCommentAction() const
{
	return TSharedPtr<FEdGraphSchemaAction>(static_cast<FEdGraphSchemaAction*>(new FInventorySchemaAction_AddComment));
}

TSharedPtr<FAssetSchemaAction_Inventory_NewNode> UEdGraphSchema_OrionInventory::AddNewNodeAction(FGraphActionListBuilderBase& ContextMenuBuilder, const FText& Category, const FText& MenuDesc, const FText& Tooltip)
{
	TSharedPtr<FAssetSchemaAction_Inventory_NewNode> NewAction = TSharedPtr<FAssetSchemaAction_Inventory_NewNode>(new FAssetSchemaAction_Inventory_NewNode(Category, MenuDesc, Tooltip, 0));
	ContextMenuBuilder.AddAction(NewAction);

	return NewAction;
}

TSharedPtr<FAssetSchemaAction_Inventory_NewSubNode> UEdGraphSchema_OrionInventory::AddNewSubNodeAction(FGraphActionListBuilderBase& ContextMenuBuilder, const FText& Category, const FText& MenuDesc, const FText& Tooltip)
{
	TSharedPtr<FAssetSchemaAction_Inventory_NewSubNode> NewAction = TSharedPtr<FAssetSchemaAction_Inventory_NewSubNode>(new FAssetSchemaAction_Inventory_NewSubNode(Category, MenuDesc, Tooltip, 0));
	ContextMenuBuilder.AddAction(NewAction);
	return NewAction;
}



#if (ENGINE_MAJOR_VERSION == 5 && ENGINE_MINOR_VERSION >= 6)
UEdGraphNode* FAssetSchemaAction_Inventory_NewSubNode::PerformAction(UEdGraph* ParentGraph, UEdGraphPin* FromPin, const FVector2f& Location, bool bSelectNewNode)
{
	ParentNode->AddSubNode(NodeTemplate, ParentGraph);
	return NULL;
}

UEdGraphNode* FAssetSchemaAction_Inventory_NewSubNode::PerformAction(UEdGraph* ParentGraph, TArray<UEdGraphPin*>& FromPins, const FVector2f& Location, bool bSelectNewNode)
{
	return PerformAction(ParentGraph, NULL, Location, bSelectNewNode);
}
#else if (ENGINE_MAJOR_VERSION == 5 && ENGINE_MINOR_VERSION <= 5)
UEdGraphNode* FAssetSchemaAction_Inventory_NewSubNode::PerformAction(UEdGraph* ParentGraph, UEdGraphPin* FromPin, const FVector2D Location, bool bSelectNewNode)
{
	ParentNode->AddSubNode(NodeTemplate, ParentGraph);
	return NULL;
}

UEdGraphNode* FAssetSchemaAction_Inventory_NewSubNode::PerformAction(UEdGraph* ParentGraph, TArray<UEdGraphPin*>& FromPins, const FVector2D Location, bool bSelectNewNode)
{
	return PerformAction(ParentGraph, NULL, Location, bSelectNewNode);
}
#endif

void FAssetSchemaAction_Inventory_NewSubNode::AddReferencedObjects(FReferenceCollector& Collector)
{
	FEdGraphSchemaAction::AddReferencedObjects(Collector);

	// These don't get saved to disk, but we want to make sure the objects don't get GC'd while the action array is around
	Collector.AddReferencedObject(NodeTemplate);
	Collector.AddReferencedObject(ParentNode);
}

#if (ENGINE_MAJOR_VERSION == 5 && ENGINE_MINOR_VERSION >= 6)
UEdGraphNode* FInventorySchemaAction_AddComment::PerformAction(UEdGraph* ParentGraph, UEdGraphPin* FromPin, const FVector2f& Location, bool bSelectNewNode)
{
	UEdGraphNode_Comment* const CommentTemplate = NewObject<UEdGraphNode_Comment>();
	CommentTemplate->bCommentBubbleVisible_InDetailsPanel = true;
	CommentTemplate->bCanRenameNode = true;

	FVector2f SpawnLocation = Location;
	FSlateRect Bounds;

	TSharedPtr<SGraphEditor> GraphEditorPtr = SGraphEditor::FindGraphEditorForGraph(ParentGraph);
	if (GraphEditorPtr.IsValid())
	{
		// If they have a selection, build a bounding box around the selection
		if (GraphEditorPtr->GetBoundsForSelectedNodes(/*out*/ Bounds, 50.0f))
		{
			CommentTemplate->SetBounds(Bounds);
			SpawnLocation.X = CommentTemplate->NodePosX;
			SpawnLocation.Y = CommentTemplate->NodePosY;
		}
		else
		{
			// Otherwise initialize a default comment at the user's cursor location.
			SpawnLocation = GraphEditorPtr->GetPasteLocation2f();
		}
	}

	UEdGraphNode* const NewNode = FEdGraphSchemaAction_NewNode::SpawnNodeFromTemplate<UEdGraphNode_Comment>(ParentGraph, CommentTemplate, SpawnLocation, bSelectNewNode);

	return NewNode;
}
#else if (ENGINE_MAJOR_VERSION == 5 && ENGINE_MINOR_VERSION <= 5)
UEdGraphNode* FInventorySchemaAction_AddComment::PerformAction(UEdGraph* ParentGraph, UEdGraphPin* FromPin, const FVector2D Location, bool bSelectNewNode)
{
	UEdGraphNode_Comment* const CommentTemplate = NewObject<UEdGraphNode_Comment>();
	CommentTemplate->bCommentBubbleVisible_InDetailsPanel = true;

	FVector2D SpawnLocation = Location;
	FSlateRect Bounds;

	TSharedPtr<SGraphEditor> GraphEditorPtr = SGraphEditor::FindGraphEditorForGraph(ParentGraph);
	if (GraphEditorPtr.IsValid())
	{
		// If they have a selection, build a bounding box around the selection
		if (GraphEditorPtr->GetBoundsForSelectedNodes(/*out*/ Bounds, 50.0f))
		{
			CommentTemplate->SetBounds(Bounds);
			SpawnLocation.X = CommentTemplate->NodePosX;
			SpawnLocation.Y = CommentTemplate->NodePosY;
		}
		else
		{
			// Otherwise initialize a default comment at the user's cursor location.
			SpawnLocation = GraphEditorPtr->GetPasteLocation();
		}
	}

	UEdGraphNode* const NewNode = FEdGraphSchemaAction_NewNode::SpawnNodeFromTemplate<UEdGraphNode_Comment>(ParentGraph, CommentTemplate, SpawnLocation, bSelectNewNode);

	return NewNode;
}

#endif
#undef LOCTEXT_NAMESPACE

