// Copyright 2024 Ivan Chandra. All Rights Reserved.


#include "EdGraphSchema_DialogBuilder.h"
#include "ToolMenus.h"
#include "Dialog_System_Editor.h"
#include "AIGraphTypes.h"
#include "DialogBuilder_ConnectionDrawingPolicy.h"
#include "Framework/Commands/GenericCommands.h"
#include "Settings/EditorStyleSettings.h"
#include "DialogBuilderEditorUtils.h"
#include "DialogBuilderEdNode.h"
#include "DialogBuilderEdNode_Objective.h"
#include "DialogBuilderEdNode_State.h"
#include "DialogBuilderEdNode_Edge.h"
#include "DialogBuilderEdNode_Root.h"
#include "GraphEditorActions.h"
#include "QuestBuilderGraph.h"
#include "QuestBuilderEdge.h"
#include "QuestBuilderNode_Root.h"
#include "QuestBuilderNode_State.h"
#include "QuestBuilderNode_Objective.h"
#include "Task/QuestTask.h"


#define LOCTEXT_NAMESPACE "EdGraphSchema_QuestBuilder"

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

UEdGraphNode* FAssetSchemaAction_DialogBuilder_NewNode::PerformAction(UEdGraph* ParentGraph, UEdGraphPin* FromPin, const FVector2D Location, bool bSelectNewNode)
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

void FAssetSchemaAction_DialogBuilder_NewNode::AddReferencedObjects(FReferenceCollector& Collector)
{
	FEdGraphSchemaAction::AddReferencedObjects(Collector);
	Collector.AddReferencedObject(NodeTemplate);
}

UEdGraphNode* FAssetSchemaAction_DialogBuilder_NewEdge::PerformAction(UEdGraph* ParentGraph, UEdGraphPin* FromPin, const FVector2D Location, bool bSelectNewNode)
{
	UEdGraphNode* ResultNode = nullptr;

	if (NodeTemplate != nullptr)
	{
		const FScopedTransaction Transaction(LOCTEXT("QuestGraphEditorNewEdge", "Dialog Graph Editor: New Edge"));
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

		NodeTemplate->QuestSystemGraphEdge->SetFlags(RF_Transactional);
		NodeTemplate->SetFlags(RF_Transactional);

		ResultNode = NodeTemplate;
	}

	return ResultNode;
}

void FAssetSchemaAction_DialogBuilder_NewEdge::AddReferencedObjects(FReferenceCollector& Collector)
{
	FEdGraphSchemaAction::AddReferencedObjects(Collector);
}


void UEdGraphSchema_DialogBuilder::GetBreakLinkToSubMenuActions(UToolMenu* Menu, UEdGraphPin* InGraphPin)
{
	// Make sure we have a unique name for every entry in the list
	TMap< FString, uint32 > LinkTitleCount;

	FToolMenuSection& Section = Menu->FindOrAddSection("QuestBuilderAssetGraphSchemaPinActions");

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
	UQuestBuilderGraph* QuestGraph = CastChecked<UQuestBuilderGraph>(Graph.GetOuter());

	FGraphNodeCreator<UDialogBuilderEdNode_Root> NodeCreator(Graph);
	UDialogBuilderEdNode_Root* RootNode = NodeCreator.CreateNode();
	RootNode->NodeInstance = NewObject<UObject>(QuestGraph, UQuestBuilderNode_Root::StaticClass());
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
	UQuestBuilderGraph* QuestGraph = CastChecked<UQuestBuilderGraph>(ContextMenuBuilder.CurrentGraph->GetOuter());

	UEdGraph* EdGraph = (UEdGraph*)ContextMenuBuilder.CurrentGraph;

	const bool bNoParent = (ContextMenuBuilder.FromPin == NULL);

	const FText AddToolTip = LOCTEXT("NewQuestBuilderNodeTooltip", "Add node here");

	TSet<TSubclassOf<UQuestBuilderNode> > Visited;



	//Task
	FGraphNodeClassHelper& ClassCache = GetClassCache();

	FCategorizedGraphActionListBuilder TasksBuilder(TEXT("Task"));

	TArray<FGraphNodeClassData> NodeClasses;
	ClassCache.GatherClasses(UQuestTask::StaticClass(), NodeClasses);
	

	for (auto& NodeClass : NodeClasses)
	{
		if (NodeClass.GetClass(true) == UQuestTask::StaticClass())
			continue;

		const FText NodeTypeName = FText::FromString(FName::NameToDisplayString(NodeClass.ToString(), false));

		TSharedPtr<FAssetSchemaAction_DialogBuilder_NewNode> AddOpAction = UEdGraphSchema_DialogBuilder::AddNewNodeAction(TasksBuilder, NodeClass.GetCategory(), NodeTypeName, FText::GetEmpty());
		UClass* ObjectiveEdNodeClass = UDialogBuilderEdNode_Objective::StaticClass();
		UClass* ObjectiveNodeClass = UQuestBuilderNode_Objective::StaticClass();

		UDialogBuilderEdNode* OpNode = NewObject<UDialogBuilderEdNode>(ContextMenuBuilder.OwnerOfTemporaries, ObjectiveEdNodeClass);
		FGraphNodeClassData ObjectiveClassData = FGraphNodeClassData(ObjectiveNodeClass, "ObjectiveClassData");
		OpNode->ClassData = ObjectiveClassData;
		AddOpAction->NodeTemplate = OpNode;
		OpNode->NodeInstance = NewObject<UObject>(EdGraph, ObjectiveNodeClass);
		OpNode->NodeInstance->SetFlags(RF_Transactional);

		if (UQuestBuilderNode_Objective* ObjectiveNode = Cast<UQuestBuilderNode_Objective>(OpNode->NodeInstance))
		{
			if (NodeClass.GetClass(true)->IsChildOf(UQuestTask::StaticClass()))
			{
				ObjectiveNode->Tasks.Add(NewObject<UQuestTask>(AddOpAction->NodeTemplate, NodeClass.GetClass(true)));
			}
		}
	}
	ContextMenuBuilder.Append(TasksBuilder);


	FCategorizedGraphActionListBuilder StateBuilder(TEXT("Dialog State"));

	//State : Complete Dialog
	UClass* StateEdNodeClass = UDialogBuilderEdNode_State::StaticClass();
	UClass* StateNodeClass = UQuestBuilderNode_State::StaticClass();

	TSharedPtr<FAssetSchemaAction_DialogBuilder_NewNode> AddOpAction = UEdGraphSchema_DialogBuilder::AddNewNodeAction(StateBuilder, FText::GetEmpty(), LOCTEXT("QuestSystemGraphNodeAction", "Complete Dialog"), LOCTEXT("QuestSystemGraphNodeTooltip", "Complete The Dialog"));
	UDialogBuilderEdNode* OpNode = NewObject<UDialogBuilderEdNode>(ContextMenuBuilder.OwnerOfTemporaries, StateEdNodeClass);
	FGraphNodeClassData StateClassData = FGraphNodeClassData(StateNodeClass, "StateClassData");
	OpNode->ClassData = StateClassData;
	AddOpAction->NodeTemplate = OpNode;
	OpNode->NodeInstance = NewObject<UObject>(EdGraph, StateNodeClass);
	OpNode->NodeInstance->SetFlags(RF_Transactional);

	if (UQuestBuilderNode_State* NodeState = Cast< UQuestBuilderNode_State>(OpNode->NodeInstance))
	{
		NodeState->QuestState = EQuestState::E_Complete;
	}

	//State : Fail Dialog
	AddOpAction = UEdGraphSchema_DialogBuilder::AddNewNodeAction(StateBuilder, FText::GetEmpty(), LOCTEXT("QuestSystemGraphNodeAction", "Fail Dialog"), LOCTEXT("QuestSystemGraphNodeTooltip", "Fail The Dialog"));
	OpNode = NewObject<UDialogBuilderEdNode>(ContextMenuBuilder.OwnerOfTemporaries, UDialogBuilderEdNode_State::StaticClass());
	OpNode->ClassData = StateClassData;
	AddOpAction->NodeTemplate = OpNode;
	OpNode->NodeInstance = NewObject<UObject>(EdGraph, StateNodeClass);
	OpNode->NodeInstance->SetFlags(RF_Transactional);

	if (UQuestBuilderNode_State* NodeState = Cast< UQuestBuilderNode_State>(OpNode->NodeInstance))
	{
		NodeState->QuestState = EQuestState::E_Fail;
	}
	ContextMenuBuilder.Append(StateBuilder);

	//State : Custom State
	/*AddOpAction = UEdGraphSchema_DialogBuilder::AddNewNodeAction(StateBuilder, FText::GetEmpty(), LOCTEXT("QuestSystemGraphNodeAction", "Add Custom State..."), LOCTEXT("QuestSystemGraphNodeTooltip", "Add Custom State..."));
	OpNode = NewObject<UDialogBuilderEdNode>(ContextMenuBuilder.OwnerOfTemporaries, UDialogBuilderEdNode_State::StaticClass());
	AddOpAction->NodeTemplate = OpNode;
	AddOpAction->NodeTemplate->QuestBuilderNode = NewObject<UQuestBuilderNode>(AddOpAction->NodeTemplate, UQuestBuilderNode_State::StaticClass());
	AddOpAction->NodeTemplate->QuestBuilderNode->QuestGraph = Graph;
	AddOpAction->NodeTemplate->QuestBuilderNode->ID = FDialogBuilderEditorUtils::FindUniqueNodeName("StateNode");
	AddOpAction->NodeTemplate->QuestBuilderNode->QuestGraph->QuestList = Graph->QuestList;
	ContextMenuBuilder.Append(StateBuilder);*/
}


FGraphNodeClassHelper& UEdGraphSchema_DialogBuilder::GetClassCache() const
{
	FDialog_System_EditorModule& EditorModule = FModuleManager::GetModuleChecked<FDialog_System_EditorModule>(TEXT("Dialog_System_Editor"));
	FGraphNodeClassHelper* ClassHelper = EditorModule.GetClassCache().Get();
	check(ClassHelper);
	return *ClassHelper;
}

void UEdGraphSchema_DialogBuilder::GetContextMenuActions(UToolMenu* Menu, UGraphNodeContextMenuContext* Context) const
{
	if (Context->Pin) {
		FToolMenuSection& Section = Menu->AddSection("QuestSystemGraphAssetGraphSchemaNodeActions", LOCTEXT("PinActionsMenuHeader", "Pin Actions"));
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
			FToolMenuSection& Section = Menu->AddSection("QuestBuilderAssetGraphSchemaNodeActions", LOCTEXT("ClassActionsMenuHeader", "Node Actions"));
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

	if (EdNode_Out == nullptr || EdNode_In == nullptr)
	{
		return FPinConnectionResponse(CONNECT_RESPONSE_DISALLOW, LOCTEXT("PinError", "Not a valid UQuestGraphEdNode"));
	}

	//Determine if we can have cycles or not
	bool bAllowCycles = false;
	auto EdGraph = Cast<UDialogBuilderEdGraph>(Out->GetOwningNode()->GetGraph());
	if (EdGraph != nullptr)
	{
		bAllowCycles = EdGraph->GetQuestBuilderGraph()->bCanBeCyclical;
	}

	// check for cycles
	FNodeVisitorCycleChecker CycleChecker;
	if (!bAllowCycles && !CycleChecker.CheckForLoop(Out->GetOwningNode(), In->GetOwningNode()))
	{
		return FPinConnectionResponse(CONNECT_RESPONSE_DISALLOW, LOCTEXT("PinErrorCycle", "Can't create a graph cycle"));
	}

	if (In->Direction == EGPD_Input && Out->Direction == EGPD_Input)
	{
		return FPinConnectionResponse(CONNECT_RESPONSE_DISALLOW, LOCTEXT("PinErrorCycle", "Invalid Node Connections"));
	}

	if (In->Direction == EGPD_Output && Out->Direction == EGPD_Input)
	{
		return FPinConnectionResponse(CONNECT_RESPONSE_DISALLOW, LOCTEXT("PinErrorCycle", "Invalid Node Connections"));
	}

	UQuestBuilderNode* QuestNodeOut = EdNode_Out ? Cast<UQuestBuilderNode>(EdNode_Out->NodeInstance) : nullptr;
	if (QuestNodeOut && QuestNodeOut->GetOwningQuestGraph()->bEdgeEnabled)
	{
		return FPinConnectionResponse(CONNECT_RESPONSE_MAKE_WITH_CONVERSION_NODE, LOCTEXT("PinConnect", "Connect nodes with edge"));
	}
	else
	{
		return FPinConnectionResponse(CONNECT_RESPONSE_MAKE, LOCTEXT("PinConnect", "Connect nodes"));
	}
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
	if (UDialogBuilderEdNode_State* StateNode = Cast<UDialogBuilderEdNode_State>(NodeA))
	{
		Super::TryCreateConnection(NodeB->GetOutputPin(), NodeA->GetInputPin());
		bSuccesful =  true;
	}
	else if (UDialogBuilderEdNode_Root* RootNode = Cast<UDialogBuilderEdNode_Root>(NodeB))
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

	if (UDialogBuilderEdGraph* QuestEdGraph = Cast<UDialogBuilderEdGraph>(NodeA->GetGraph()))
	{
		QuestEdGraph->RebuildQuestBuilderGraph(true);
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

	UQuestBuilderNode* QuestNodeA = NodeA ? Cast<UQuestBuilderNode>(NodeA->NodeInstance) : nullptr;
	UQuestBuilderGraph* Graph = NodeA && QuestNodeA ? QuestNodeA->GetOwningQuestGraph() : nullptr;

	FVector2D InitPos((NodeA->NodePosX + NodeB->NodePosX) / 2, (NodeA->NodePosY + NodeB->NodePosY) / 2);

	FAssetSchemaAction_DialogBuilder_NewEdge Action;
	Action.NodeTemplate = NewObject<UDialogBuilderEdNode_Edge>(NodeA->GetGraph());
	Action.NodeTemplate->SetEdge(NewObject<UQuestBuilderEdge>(Action.NodeTemplate, UQuestBuilderEdge::StaticClass()));
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
	if (UDialogBuilderEdGraph* QuestEdGraph = Cast<UDialogBuilderEdGraph>(TargetNode.GetGraph()))
	{
		QuestEdGraph->RebuildQuestBuilderGraph(true);
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


TSharedPtr<FAssetSchemaAction_DialogBuilder_NewNode> UEdGraphSchema_DialogBuilder::AddNewNodeAction(FGraphActionListBuilderBase& ContextMenuBuilder, const FText& Category, const FText& MenuDesc, const FText& Tooltip)
{
	TSharedPtr<FAssetSchemaAction_DialogBuilder_NewNode> NewAction = TSharedPtr<FAssetSchemaAction_DialogBuilder_NewNode>(new FAssetSchemaAction_DialogBuilder_NewNode(Category, MenuDesc, Tooltip, 0));
	ContextMenuBuilder.AddAction(NewAction);

	return NewAction;
}


#undef LOCTEXT_NAMESPACE