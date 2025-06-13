// Copyright 2025 Ivan Chandra. All Rights Reserved.


#include "EdGraphSchema_DialogBuilder.h"
#include "ToolMenus.h"
#include "Dialog_System_Editor.h"
#include "DialogBuilderSetting.h"
#include "AIGraphTypes.h"
#include "DialogBuilder_ConnectionDrawingPolicy.h"
#include "Framework/Commands/GenericCommands.h"
#include "Settings/EditorStyleSettings.h"
#include "DialogBuilderEditorUtils.h"
#include "DialogBuilderEdNode.h"
#include "DialogBuilderEdNode_DialogLine.h"
#include "DialogBuilderEdNode_OptionSelector.h"
#include "DialogBuilderEdNode_PlayerOption.h"
#include "DialogBuilderEdNode_Edge.h"
#include "DialogBuilderEdNode_Root.h"
#include "DialogBuilderEdSubNode_Decorator.h"
#include "DialogBuilderEdSubNode_Event.h"
#include "GraphEditorActions.h"
#include "DialogBuilderGraph.h"
#include "DialogBuilderEdge.h"
#include "DialogBuilderNode_Root.h"
#include "DialogBuilderNode_OptionSelector.h"
#include "DialogBuilderNode_PlayerOption.h"
#include "DialogBuilderNode_DialogLine.h"
#include "Decorator/OrionDecorator.h"
#include "Event/OrionEvent.h"


#define LOCTEXT_NAMESPACE "EdGraphSchema_DialogBuilder"

namespace
{
	// Maximum distance a drag can be off a node edge to require 'push off' from node
	const int32 NodeDistance = 60;
}

int32 UEdGraphSchema_DialogBuilder::CurrentCacheRefreshID = 0;

class FNodeVisitorCycleChecker
{
public:
	/** Check whether a loop in the graph would be caused by linking the passed-in nodes */
	bool CheckForLoop(UEdGraphNode* StartNode, UEdGraphNode* EndNode)
	{

		VisitedNodes.Add(StartNode);

		return TraverseNodes(EndNode);
	}

private:
	bool TraverseNodes(UEdGraphNode* Node)
	{
		VisitedNodes.Add(Node);

		for (auto MyPin : Node->Pins)
		{
			if (MyPin->Direction == EGPD_Output)
			{
				for (auto OtherPin : MyPin->LinkedTo)
				{
					UEdGraphNode* OtherNode = OtherPin->GetOwningNode();
					if (VisitedNodes.Contains(OtherNode))
					{
						// Only  an issue if this is a back-edge
						return false;
					}
					else if (!FinishedNodes.Contains(OtherNode))
					{
						// Only should traverse if this node hasn't been traversed
						if (!TraverseNodes(OtherNode))
							return false;
					}
				}
			}
		}

		VisitedNodes.Remove(Node);
		FinishedNodes.Add(Node);
		return true;
	};


	TSet<UEdGraphNode*> VisitedNodes;
	TSet<UEdGraphNode*> FinishedNodes;
};

UEdGraphNode* FAssetSchemaAction_DialogSystem_NewNode::PerformAction(UEdGraph* ParentGraph, UEdGraphPin* FromPin, const FVector2D Location, bool bSelectNewNode)
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

void FAssetSchemaAction_DialogSystem_NewNode::AddReferencedObjects(FReferenceCollector& Collector)
{
	FEdGraphSchemaAction::AddReferencedObjects(Collector);
	Collector.AddReferencedObject(NodeTemplate);
}

UEdGraphNode* FAssetSchemaAction_DialogSystem_NewEdge::PerformAction(UEdGraph* ParentGraph, UEdGraphPin* FromPin, const FVector2D Location, bool bSelectNewNode)
{
	UEdGraphNode* ResultNode = nullptr;

	if (NodeTemplate != nullptr)
	{
		const FScopedTransaction Transaction(LOCTEXT("DialogGraphEditorNewEdge", "Dialog Graph Editor: New Edge"));
		ParentGraph->Modify();
		if (FromPin != nullptr)
			FromPin->Modify();

		NodeTemplate->Rename(nullptr, ParentGraph);
		ParentGraph->AddNode(NodeTemplate, true, bSelectNewNode);

		NodeTemplate->CreateNewGuid();
		NodeTemplate->PostPlacedNewNode();
		NodeTemplate->AllocateDefaultPins();
		NodeTemplate->AutowireNewNode(FromPin);

		NodeTemplate->NodePosX = Location.X;
		NodeTemplate->NodePosY = Location.Y;

		NodeTemplate->DialogSystemGraphEdge->SetFlags(RF_Transactional);
		NodeTemplate->SetFlags(RF_Transactional);

		ResultNode = NodeTemplate;
	}

	return ResultNode;
}

void FAssetSchemaAction_DialogSystem_NewEdge::AddReferencedObjects(FReferenceCollector& Collector)
{
	FEdGraphSchemaAction::AddReferencedObjects(Collector);
}


UEdGraphSchema_DialogBuilder::UEdGraphSchema_DialogBuilder()
{
	DecoratorClass = UDialogBuilderEdSubNode_Decorator::StaticClass();
	EventClass = UDialogBuilderEdSubNode_Event::StaticClass();
}

void UEdGraphSchema_DialogBuilder::GetBreakLinkToSubMenuActions(UToolMenu* Menu, UEdGraphPin* InGraphPin)
{
	// Make sure we have a unique name for every entry in the list
	TMap< FString, uint32 > LinkTitleCount;

	FToolMenuSection& Section = Menu->FindOrAddSection("DialogBuilderAssetGraphSchemaPinActions");

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
			FExecuteAction::CreateUObject(this, &UEdGraphSchema_DialogBuilder::BreakSinglePinLink, const_cast<UEdGraphPin*>(InGraphPin), *Links)));
	}
}

void UEdGraphSchema_DialogBuilder::CreateDefaultNodesForGraph(UEdGraph& Graph) const
{
	UDialogBuilderGraph* DialogGraph = CastChecked<UDialogBuilderGraph>(Graph.GetOuter());

	FGraphNodeCreator<UDialogBuilderEdNode_Root> NodeCreator(Graph);
	UDialogBuilderEdNode_Root* RootNode = NodeCreator.CreateNode();
	RootNode->NodeInstance = NewObject<UObject>(DialogGraph, UDialogBuilderNode_Root::StaticClass());
	RootNode->NodeInstance->SetFlags(RF_Transactional);
	NodeCreator.Finalize();
	SetNodeMetaData(RootNode, FNodeMetadata::DefaultGraphNode);
}

EGraphType UEdGraphSchema_DialogBuilder::GetGraphType(const UEdGraph* TestEdGraph) const
{
	return GT_StateMachine;
}

void UEdGraphSchema_DialogBuilder::GetGraphContextActions(FGraphContextMenuBuilder& ContextMenuBuilder) const
{
	UDialogBuilderGraph* DialogGraph = CastChecked<UDialogBuilderGraph>(ContextMenuBuilder.CurrentGraph->GetOuter());
	UEdGraph* EdGraph = (UEdGraph*)ContextMenuBuilder.CurrentGraph;

	const bool bNoParent = (ContextMenuBuilder.FromPin == NULL);
	const bool bOnlySelector = ContextMenuBuilder.FromPin && Cast<UDialogBuilderEdNode_OptionSelector>(ContextMenuBuilder.FromPin->GetOwningNode()) ? true : false;
	const bool bAllowDialogLine = bNoParent || !bOnlySelector;
	const bool bAllowSelector = bNoParent || !bOnlySelector;
	const bool bAllowPlayerOption = bNoParent || bOnlySelector;


	//DialogLine
	if (bAllowDialogLine)
	{
		FCategorizedGraphActionListBuilder DialogLineBuilder(TEXT("Speaker"));
		UClass* DialogLineEdNodeClass = UDialogBuilderEdNode_DialogLine::StaticClass();
		UClass* DialogLineNodeClass = UDialogBuilderNode_DialogLine::StaticClass();

		TSharedPtr<FAssetSchemaAction_DialogSystem_NewNode> AddOpAction = UEdGraphSchema_DialogBuilder::AddNewNodeAction(DialogLineBuilder, FText::GetEmpty(), LOCTEXT("DialogSystemGraphNodeAction", "Add Dialog Line..."), LOCTEXT("DialogSystemGraphNodeTooltip", "Create Dialog Line Node."));
		UDialogBuilderEdNode* OpNode = NewObject<UDialogBuilderEdNode>(ContextMenuBuilder.OwnerOfTemporaries, DialogLineEdNodeClass);
		FGraphNodeClassData DialogLineClassData = FGraphNodeClassData(DialogLineNodeClass, "DialogLineClassData");
		OpNode->ClassData = DialogLineClassData;
		AddOpAction->NodeTemplate = OpNode;
		OpNode->NodeInstance = NewObject<UObject>(DialogGraph, DialogLineNodeClass);
		OpNode->NodeInstance->SetFlags(RF_Transactional);

		ContextMenuBuilder.Append(DialogLineBuilder);
	}
	
	//Player Options
	FCategorizedGraphActionListBuilder PlayerOptionsBuilder(TEXT("Player Options"));

	//Option Selector
	if (bAllowSelector)
	{
		UClass* OptionSelectorEdNodeClass = UDialogBuilderEdNode_OptionSelector::StaticClass();
		UClass* OptionSelectorNodeClass = UDialogBuilderNode_OptionSelector::StaticClass();

		TSharedPtr<FAssetSchemaAction_DialogSystem_NewNode> AddOpAction = UEdGraphSchema_DialogBuilder::AddNewNodeAction(PlayerOptionsBuilder, FText::GetEmpty(), LOCTEXT("DialogSystemGraphNodeAction", "Add Selector..."), LOCTEXT("DialogSystemGraphNodeTooltip", "Create Selector Node."));
		UDialogBuilderEdNode* OpNode = NewObject<UDialogBuilderEdNode>(ContextMenuBuilder.OwnerOfTemporaries, OptionSelectorEdNodeClass);
		FGraphNodeClassData OptionSelectorClassData = FGraphNodeClassData(OptionSelectorNodeClass, "OptionSelectorClassData");
		OpNode->ClassData = OptionSelectorClassData;
		AddOpAction->NodeTemplate = OpNode;
		OpNode->NodeInstance = NewObject<UObject>(DialogGraph, OptionSelectorNodeClass);
		OpNode->NodeInstance->SetFlags(RF_Transactional);
	}
	

	if (bAllowPlayerOption)
	{
		//Player Option
		UClass* PlayerOptionEdNodeClass = UDialogBuilderEdNode_PlayerOption::StaticClass();
		UClass* PlayerOptionNodeClass = UDialogBuilderNode_PlayerOption::StaticClass();

		TSharedPtr<FAssetSchemaAction_DialogSystem_NewNode> AddOpAction = UEdGraphSchema_DialogBuilder::AddNewNodeAction(PlayerOptionsBuilder, FText::GetEmpty(), LOCTEXT("DialogSystemGraphNodeAction", "Add Player Option..."), LOCTEXT("DialogSystemGraphNodeTooltip", "Add Player Option..."));
		UDialogBuilderEdNode* OpNode = NewObject<UDialogBuilderEdNode>(ContextMenuBuilder.OwnerOfTemporaries, PlayerOptionEdNodeClass);
		FGraphNodeClassData PlayerOptionClassData = FGraphNodeClassData(PlayerOptionNodeClass, "PlayerOptionClassData");
		OpNode->ClassData = PlayerOptionClassData;
		AddOpAction->NodeTemplate = OpNode;
		OpNode->NodeInstance = NewObject<UObject>(DialogGraph, PlayerOptionNodeClass);
		OpNode->NodeInstance->SetFlags(RF_Transactional);
	}

	ContextMenuBuilder.Append(PlayerOptionsBuilder);
	
	// Add the ability to create a comment to the context menu too for discoverability
	{
		TSharedPtr<FDialogSchemaAction_AddComment> Action = TSharedPtr<FDialogSchemaAction_AddComment>(
			new FDialogSchemaAction_AddComment(LOCTEXT("AddComment", "Add Comment"), LOCTEXT("AddComment_Tooltip", "Adds a comment node to the graph."))
		);

		ContextMenuBuilder.AddAction(Action);
	}
}



void UEdGraphSchema_DialogBuilder::GetGraphNodeContextActions(FGraphContextMenuBuilder& ContextMenuBuilder, int32 SubNodeFlags) const
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

			UDialogBuilderEdNode* OpNode = NewObject<UDialogBuilderEdNode>(Graph, GraphNodeClass);
			OpNode->ClassData = NodeClass;

			TSharedPtr<FAssetSchemaAction_DialogSystem_NewSubNode> AddOpAction = UEdGraphSchema_DialogBuilder::AddNewSubNodeAction(ContextMenuBuilder, NodeClass.GetCategory(), NodeTypeName, NodeClass.GetTooltip());
			AddOpAction->ParentNode = Cast<UDialogBuilderEdNode>(ContextMenuBuilder.SelectedObjects[0]);
			AddOpAction->NodeTemplate = OpNode;
		}
	}

}

void UEdGraphSchema_DialogBuilder::GetSubNodeClasses(int32 SubNodeFlags, TArray<FGraphNodeClassData>& ClassData, UClass*& GraphNodeClass) const
{
	FGraphNodeClassHelper& ClassCache = GetClassCache(SubNodeFlags);

	if (SubNodeFlags == EDialogSubNode::Decorator)
	{
		ClassCache.GatherClasses(UOrionDecorator::StaticClass(), ClassData);
		GraphNodeClass = DecoratorClass;
	}
	else if (SubNodeFlags == EDialogSubNode::Event)
	{
		ClassCache.GatherClasses(UOrionEvent::StaticClass(), ClassData);
		GraphNodeClass = EventClass;
	}
}


FGraphNodeClassHelper& UEdGraphSchema_DialogBuilder::GetClassCache(int32 SubNodeFlags) const
{
	FDialog_System_EditorModule& EditorModule = FModuleManager::GetModuleChecked<FDialog_System_EditorModule>(TEXT("Dialog_System_Editor"));
	FGraphNodeClassHelper* ClassHelper = nullptr;
	if (SubNodeFlags == EDialogSubNode::Decorator)
	{
		ClassHelper = EditorModule.GetDecoratorClassCache().Get();
	}
	else if (SubNodeFlags == EDialogSubNode::Event)
	{
		ClassHelper = EditorModule.GetEventClassCache().Get();
	}
	
	
	check(ClassHelper);
	return *ClassHelper;
}

void UEdGraphSchema_DialogBuilder::GetContextMenuActions(UToolMenu* Menu, UGraphNodeContextMenuContext* Context) const
{
	if (Context->Pin) {
		FToolMenuSection& Section = Menu->AddSection("DialogSystemGraphAssetGraphSchemaNodeActions", LOCTEXT("PinActionsMenuHeader", "Pin Actions"));
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
						FNewToolMenuDelegate::CreateUObject((UEdGraphSchema_DialogBuilder* const)this, &UEdGraphSchema_DialogBuilder::GetBreakLinkToSubMenuActions, const_cast<UEdGraphPin*>(Context->Pin)));
				}
				else
				{
					((UEdGraphSchema_DialogBuilder* const)this)->GetBreakLinkToSubMenuActions(Menu, const_cast<UEdGraphPin*>(Context->Pin));
				}
			}
	}
	else if (Context->Node)
	{
		{
			FToolMenuSection& Section = Menu->AddSection("DialogBuilderAssetGraphSchemaNodeActions", LOCTEXT("ClassActionsMenuHeader", "Node Actions"));
			Section.AddMenuEntry(FGenericCommands::Get().Rename);
			Section.AddMenuEntry(FGenericCommands::Get().Delete);
			Section.AddMenuEntry(FGenericCommands::Get().Cut);
			Section.AddMenuEntry(FGenericCommands::Get().Copy);
			Section.AddMenuEntry(FGenericCommands::Get().Duplicate);

			Section.AddMenuEntry(FGraphEditorCommands::Get().BreakNodeLinks);
		}
	}

	Super::GetContextMenuActions(Menu, Context);
}

const FPinConnectionResponse UEdGraphSchema_DialogBuilder::CanCreateConnection(const UEdGraphPin* A, const UEdGraphPin* B) const
{
	
	// Make sure the pins are not on the same node
	if (A->GetOwningNode() == B->GetOwningNode())
	{
		return FPinConnectionResponse(CONNECT_RESPONSE_DISALLOW, LOCTEXT("PinErrorSameNode", "Can't connect node to itself"));
	}

	const UEdGraphPin* Out = A;
	const UEdGraphPin* In = B;

	UDialogBuilderEdNode* EdNode_Out = Cast<UDialogBuilderEdNode>(Out->GetOwningNode());
	UDialogBuilderEdNode* EdNode_In = Cast<UDialogBuilderEdNode>(In->GetOwningNode());

	const bool bNodeAIsSpeaker = Cast<UDialogBuilderEdNode_DialogLine>(A->GetOwningNode()) ? true : false;
	const bool bNodeAIsOptionSelector = Cast<UDialogBuilderEdNode_OptionSelector>(A->GetOwningNode()) ? true : false;
	const bool bNodeAIsPlayerOption = Cast<UDialogBuilderEdNode_PlayerOption>(A->GetOwningNode()) ? true : false;

	const bool bNodeBIsSpeaker = Cast<UDialogBuilderEdNode_DialogLine>(B->GetOwningNode()) ? true : false;
	const bool bNodeBIsOptionSelector = Cast<UDialogBuilderEdNode_OptionSelector>(B->GetOwningNode()) ? true : false;
	const bool bNodeBIsPlayerOption = Cast<UDialogBuilderEdNode_PlayerOption>(B->GetOwningNode()) ? true : false;

	if (EdNode_Out == nullptr || EdNode_In == nullptr)
	{
		return FPinConnectionResponse(CONNECT_RESPONSE_DISALLOW, LOCTEXT("PinError", "Not a valid UDialogGraphEdNode"));
	}

	//Determine if we can have cycles or not
	bool bAllowCycles = false;
	auto EdGraph = Cast<UDialogBuilderEdGraph>(Out->GetOwningNode()->GetGraph());
	if (EdGraph != nullptr)
	{
		bAllowCycles = GetDefault<UDialogBuilderSetting>()->bCanBeCyclical;
	}



	// check for cycles
	FNodeVisitorCycleChecker CycleChecker;
	if (!bAllowCycles && !CycleChecker.CheckForLoop(Out->GetOwningNode(), In->GetOwningNode()))
	{
		return FPinConnectionResponse(CONNECT_RESPONSE_DISALLOW, LOCTEXT("PinErrorCycle", "Can't create a graph cycle"));
	}

	if (bNodeAIsPlayerOption && bNodeBIsPlayerOption)
	{
		return FPinConnectionResponse(CONNECT_RESPONSE_DISALLOW, LOCTEXT("PinErrorCycle", "Invalid Node Connections"));
	}

	if (bNodeAIsOptionSelector && bNodeBIsOptionSelector)
	{
		return FPinConnectionResponse(CONNECT_RESPONSE_DISALLOW, LOCTEXT("PinErrorCycle", "Invalid Node Connections"));
	}

	if (bNodeAIsSpeaker && bNodeBIsPlayerOption)
	{
		return FPinConnectionResponse(CONNECT_RESPONSE_DISALLOW, LOCTEXT("PinErrorCycle", "Invalid Node Connections"));
	}

	if (In->Direction == EGPD_Input && Out->Direction == EGPD_Input)
	{
		return FPinConnectionResponse(CONNECT_RESPONSE_DISALLOW, LOCTEXT("PinErrorCycle", "Invalid Node Connections"));
	}

	if (In->Direction == EGPD_Output && Out->Direction == EGPD_Input)
	{
		return FPinConnectionResponse(CONNECT_RESPONSE_DISALLOW, LOCTEXT("PinErrorCycle", "Invalid Node Connections"));
	}

	if (GetDefault<UDialogBuilderSetting>()->bEdgeEnabled)
	{
		return FPinConnectionResponse(CONNECT_RESPONSE_MAKE_WITH_CONVERSION_NODE, LOCTEXT("PinConnect", "Connect nodes with edge"));
	}
	else
	{
		return FPinConnectionResponse(CONNECT_RESPONSE_MAKE, LOCTEXT("PinConnect", "Connect nodes"));
	}
}

const FPinConnectionResponse UEdGraphSchema_DialogBuilder::CanMergeNodes(const UEdGraphNode* NodeA, const UEdGraphNode* NodeB) const
{
	// Make sure the nodes are not the same 
	if (NodeA == NodeB)
	{
		return FPinConnectionResponse(CONNECT_RESPONSE_DISALLOW, TEXT("Both are the same node"));
	}

	const UDialogBuilderEdNode* DialogEdNodeA = Cast<UDialogBuilderEdNode>(NodeA);
	const UDialogBuilderEdNode* DialogEdNodeB = Cast<UDialogBuilderEdNode>(NodeB);

	const bool bNodeAIsDecorator = NodeA->IsA(UDialogBuilderEdSubNode_Decorator::StaticClass());
	const bool bNodeAIsEvent = NodeA->IsA(UDialogBuilderEdSubNode_Event::StaticClass());
	const bool bNodeBIsObjective = NodeB->IsA(UDialogBuilderEdNode_DialogLine::StaticClass());
	const bool bNodeBIsState = NodeB->IsA(UDialogBuilderEdNode_OptionSelector::StaticClass());
	const bool bNodeBIsDecorator = NodeB->IsA(UDialogBuilderEdSubNode_Decorator::StaticClass());
	const bool bNodeBIsEvent = NodeB->IsA(UDialogBuilderEdSubNode_Event::StaticClass());

	if ((bNodeAIsDecorator && (bNodeBIsDecorator || bNodeBIsState || bNodeBIsObjective))
		|| (bNodeAIsEvent && (bNodeBIsEvent || bNodeBIsState || bNodeBIsObjective)))
	{
		return FPinConnectionResponse(CONNECT_RESPONSE_MAKE, TEXT(""));
	}
	return FPinConnectionResponse(CONNECT_RESPONSE_DISALLOW, TEXT(""));
}

bool UEdGraphSchema_DialogBuilder::TryCreateConnection(UEdGraphPin* A, UEdGraphPin* B) const
{
	// We don't actually care about the pin, we want the node that is being dragged between
	UDialogBuilderEdNode* NodeA = Cast<UDialogBuilderEdNode>(A->GetOwningNode());
	UDialogBuilderEdNode* NodeB = Cast<UDialogBuilderEdNode>(B->GetOwningNode());

	if(NodeA == NodeB)
	{
		return false;
	}

	// Check that this edge doesn't already exist
	if (NodeA->GetOutputPin())
	{
		for (UEdGraphPin* TestPin : NodeA->GetOutputPin()->LinkedTo)
		{
			UEdGraphNode* ChildNode = TestPin->GetOwningNode();
			if (UDialogBuilderEdNode_Edge* EdNode_Edge = Cast<UDialogBuilderEdNode_Edge>(ChildNode))
			{
				ChildNode = EdNode_Edge->GetEndNode();
			}
			if (ChildNode == NodeB)
				return false;
		}
	}
	bool bSuccesful = false;
	if (UDialogBuilderEdNode_Root* RootNode = Cast<UDialogBuilderEdNode_Root>(NodeB))
	{
		Super::TryCreateConnection(NodeB->GetOutputPin(), NodeA->GetInputPin());
		bSuccesful =  true;
	}
	else if (NodeA && NodeB)
	{
		if (A->Direction == EGPD_Output && B->Direction == EGPD_Input)
		{
			// Always create connections from node A to B, don't allow adding in reverse
			Super::TryCreateConnection(NodeA->GetOutputPin(), NodeB->GetInputPin());
			bSuccesful =  true;
		}
	}

	if (UDialogBuilderEdGraph* DialogEdGraph = Cast<UDialogBuilderEdGraph>(NodeA->GetGraph()))
	{
		DialogEdGraph->UpdateAsset(true);
	}

	
	return bSuccesful;
}

bool UEdGraphSchema_DialogBuilder::CreateAutomaticConversionNodeAndConnections(UEdGraphPin* A, UEdGraphPin* B) const
{
	UDialogBuilderEdNode* NodeA = Cast<UDialogBuilderEdNode>(A->GetOwningNode());
	UDialogBuilderEdNode* NodeB = Cast<UDialogBuilderEdNode>(B->GetOwningNode());

	// Are nodes and pins all valid?
	if (!NodeA || !NodeA->GetOutputPin() || !NodeB || !NodeB->GetInputPin())
		return false;

	UDialogBuilderNode* DialogNodeA = NodeA ? Cast<UDialogBuilderNode>(NodeA->NodeInstance) : nullptr;
	UDialogBuilderGraph* Graph = NodeA && DialogNodeA ? DialogNodeA->GetOwningDialogGraph() : nullptr;

	FVector2D InitPos((NodeA->NodePosX + NodeB->NodePosX) / 2, (NodeA->NodePosY + NodeB->NodePosY) / 2);

	FAssetSchemaAction_DialogSystem_NewEdge Action;
	Action.NodeTemplate = NewObject<UDialogBuilderEdNode_Edge>(NodeA->GetGraph());
	Action.NodeTemplate->SetEdge(NewObject<UDialogBuilderEdge>(Action.NodeTemplate, UDialogBuilderEdge::StaticClass()));
	UDialogBuilderEdNode_Edge* EdgeNode = Cast<UDialogBuilderEdNode_Edge>(Action.PerformAction(NodeA->GetGraph(), nullptr, InitPos, false));

	// Always create connections from node A to B, don't allow adding in reverse
	EdgeNode->CreateConnections(NodeA, NodeB);

	return true;
}

FConnectionDrawingPolicy* UEdGraphSchema_DialogBuilder::CreateConnectionDrawingPolicy(int32 InBackLayerID, int32 InFrontLayerID, float InZoomFactor, const FSlateRect& InClippingRect, FSlateWindowElementList& InDrawElements, UEdGraph* InGraphObj) const
{
	return new FDialogBuilder_ConnectionDrawingPolicy(InBackLayerID, InFrontLayerID, InZoomFactor, InClippingRect, InDrawElements, InGraphObj);
}

FLinearColor UEdGraphSchema_DialogBuilder::GetPinTypeColor(const FEdGraphPinType& PinType) const
{
	return FColor::Cyan;
}

void UEdGraphSchema_DialogBuilder::BreakNodeLinks(UEdGraphNode& TargetNode) const
{
	const FScopedTransaction Transaction(NSLOCTEXT("UnrealEd", "GraphEd_BreakNodeLinks", "Break Node Links"));

	Super::BreakNodeLinks(TargetNode);
	if (UDialogBuilderEdGraph* DialogEdGraph = Cast<UDialogBuilderEdGraph>(TargetNode.GetGraph()))
	{
		DialogEdGraph->UpdateAsset(true);
	}
}

void UEdGraphSchema_DialogBuilder::BreakPinLinks(UEdGraphPin& TargetPin, bool bSendsNodeNotifcation) const
{
	const FScopedTransaction Transaction(NSLOCTEXT("UnrealEd", "GraphEd_BreakPinLinks", "Break Pin Links"));

	Super::BreakPinLinks(TargetPin, bSendsNodeNotifcation);
}

void UEdGraphSchema_DialogBuilder::BreakSinglePinLink(UEdGraphPin* SourcePin, UEdGraphPin* TargetPin) const
{
	const FScopedTransaction Transaction(NSLOCTEXT("UnrealEd", "GraphEd_BreakSinglePinLink", "Break Pin Link"));

	Super::BreakSinglePinLink(SourcePin, TargetPin);
}

UEdGraphPin* UEdGraphSchema_DialogBuilder::DropPinOnNode(UEdGraphNode* InTargetNode, const FName& InSourcePinName, const FEdGraphPinType& InSourcePinType, EEdGraphPinDirection InSourcePinDirection) const
{
	UDialogBuilderEdNode* EdNode = Cast<UDialogBuilderEdNode>(InTargetNode);
	switch (InSourcePinDirection)
	{
	case EGPD_Input:
		return EdNode->GetOutputPin();
	case EGPD_Output:
		return EdNode->GetInputPin();
	default:
		return nullptr;
	}
}

bool UEdGraphSchema_DialogBuilder::SupportsDropPinOnNode(UEdGraphNode* InTargetNode, const FEdGraphPinType& InSourcePinType, EEdGraphPinDirection InSourcePinDirection, FText& OutErrorMessage) const
{
	return Cast<UDialogBuilderEdNode>(InTargetNode) != nullptr;
}

bool UEdGraphSchema_DialogBuilder::IsCacheVisualizationOutOfDate(int32 InVisualizationCacheID) const
{
	return CurrentCacheRefreshID != InVisualizationCacheID;
}

int32 UEdGraphSchema_DialogBuilder::GetCurrentVisualizationCacheID() const
{
	return CurrentCacheRefreshID;
}

void UEdGraphSchema_DialogBuilder::ForceVisualizationCacheClear() const
{
	++CurrentCacheRefreshID;
}

TSharedPtr<FEdGraphSchemaAction> UEdGraphSchema_DialogBuilder::GetCreateCommentAction() const
{
	return TSharedPtr<FEdGraphSchemaAction>(static_cast<FEdGraphSchemaAction*>(new FDialogSchemaAction_AddComment));
}

TSharedPtr<FAssetSchemaAction_DialogSystem_NewNode> UEdGraphSchema_DialogBuilder::AddNewNodeAction(FGraphActionListBuilderBase& ContextMenuBuilder, const FText& Category, const FText& MenuDesc, const FText& Tooltip)
{
	TSharedPtr<FAssetSchemaAction_DialogSystem_NewNode> NewAction = TSharedPtr<FAssetSchemaAction_DialogSystem_NewNode>(new FAssetSchemaAction_DialogSystem_NewNode(Category, MenuDesc, Tooltip, 0));
	ContextMenuBuilder.AddAction(NewAction);

	return NewAction;
}

TSharedPtr<FAssetSchemaAction_DialogSystem_NewSubNode> UEdGraphSchema_DialogBuilder::AddNewSubNodeAction(FGraphActionListBuilderBase& ContextMenuBuilder, const FText& Category, const FText& MenuDesc, const FText& Tooltip)
{
	TSharedPtr<FAssetSchemaAction_DialogSystem_NewSubNode> NewAction = TSharedPtr<FAssetSchemaAction_DialogSystem_NewSubNode>(new FAssetSchemaAction_DialogSystem_NewSubNode(Category, MenuDesc, Tooltip, 0));
	ContextMenuBuilder.AddAction(NewAction);
	return NewAction;
}


UEdGraphNode* FAssetSchemaAction_DialogSystem_NewSubNode::PerformAction(UEdGraph* ParentGraph, UEdGraphPin* FromPin, const FVector2D Location, bool bSelectNewNode)
{
	ParentNode->AddSubNode(NodeTemplate, ParentGraph);
	return NULL;
}

UEdGraphNode* FAssetSchemaAction_DialogSystem_NewSubNode::PerformAction(UEdGraph* ParentGraph, TArray<UEdGraphPin*>& FromPins, const FVector2D Location, bool bSelectNewNode)
{
	return PerformAction(ParentGraph, NULL, Location, bSelectNewNode);
}

void FAssetSchemaAction_DialogSystem_NewSubNode::AddReferencedObjects(FReferenceCollector& Collector)
{
	FEdGraphSchemaAction::AddReferencedObjects(Collector);

	// These don't get saved to disk, but we want to make sure the objects don't get GC'd while the action array is around
	Collector.AddReferencedObject(NodeTemplate);
	Collector.AddReferencedObject(ParentNode);
}

UEdGraphNode* FDialogSchemaAction_AddComment::PerformAction(UEdGraph* ParentGraph, UEdGraphPin* FromPin, const FVector2D Location, bool bSelectNewNode)
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

#undef LOCTEXT_NAMESPACE

