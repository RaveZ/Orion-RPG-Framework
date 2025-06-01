// Copyright 2025 Ivan Chandra. All Rights Reserved.


#include "QuestBuilderNode.h"
#include "QuestBuilderEdge.h"
#include "Quest.h"
#include "QuestComponent.h"
#include "Event/OrionEvent.h"
#include "Decorator/OrionDecorator.h"
#include "QuestBuilderGraph.h"

#define LOCTEXT_NAMESPACE "QuestBuilderNode"

UQuestBuilderNode::UQuestBuilderNode()
{
	bAddToJournal = true;
#if WITH_EDITORONLY_DATA
	CompatibleGraphType = UQuestBuilderGraph::StaticClass();

	FLinearColor Default(0.05f, 0.05f, 0.05f);
	BackgroundColor = Default;
#endif
}

UQuestBuilderNode::~UQuestBuilderNode()
{
}

void UQuestBuilderNode::Initialize(bool bLaunchEventOnLoad)
{
	StartEventQueue.Empty();

	if (bLaunchEventOnLoad)
	{
		for (auto& EndEvent : Events)
		{
			if (EndEvent && EndEvent->bLaunchEventOnLoad && (EndEvent->EventLaunchType == EEventLaunchType::E_Both || EndEvent->EventLaunchType == EEventLaunchType::E_Start))
			{
				StartEventQueue.Add(EndEvent);
			}
		}
	}
	else
	{
		for (auto& EndEvent : Events)
		{
			if (EndEvent && (EndEvent->EventLaunchType == EEventLaunchType::E_Both || EndEvent->EventLaunchType == EEventLaunchType::E_Start))
			{
				StartEventQueue.Add(EndEvent);
			}
		}
	}
	OnAllStartEventFinished.AddLambda(
		[this]()
		{
			BeginNode();
		}
	);
	LaunchStartEventQueue();

	
}

void UQuestBuilderNode::Deinitialize()
{
	EndEventQueue.Empty();
	for (auto& EndEvent : Events)
	{
		if (EndEvent && (EndEvent->EventLaunchType == EEventLaunchType::E_Both || EndEvent->EventLaunchType == EEventLaunchType::E_End))
		{
			EndEventQueue.Add(EndEvent);
		}
	}
	LaunchEndEventQueue();
}

void UQuestBuilderNode::BeginNode()
{
	//empty in base 
}

void UQuestBuilderNode::Reset()
{
	//empty in base 
}

void UQuestBuilderNode::EvaluateNextNode()
{
	if (Quest->QuestState != EQuestState::E_Active)
	 return;
	
	UQuestBuilderNode* NextNode = nullptr;
	//Quest Node will executes their children from top to bottom, and will stop executing its children when one of their children decorator succeeds.
	//If none of the children succeeds, this node wiil stop evaluate branching until next quest update.
	for (auto& Node : ChildrenNodes)
	{
		int SumConditionMet = 0;
		for (auto& Condition : Node->Decorators)
		{
			bool bConditionMet = Condition && Condition->InvertCondition ? !Condition->PerformConditionCheck(OwningController, GetOwningPawn()) : Condition->PerformConditionCheck(OwningController, GetOwningPawn());
			if (bConditionMet)
			{
				SumConditionMet++;
			}
		}
		if (SumConditionMet >= Node->Decorators.Num())
		{
			NextNode = Node;
			//break loop to ensure the met condition doesn't get overrided
			break;
		}
		
	}

	UQuestBuilderNode* CachedCurrentNode = this;
	
	OnAllEndEventFinished.AddLambda(
		[this, NextNode, CachedCurrentNode]()
		{
			if (NextNode)
			{
				//Check if the current node still hasn't changed after the events. if it changed, it means we dont need to go to the next node anymore
				if (CachedCurrentNode == this)
				{
					GetOwningQuest()->BeginNode(NextNode);
				}
			}
		}
	);
	Deinitialize();

}

UQuestBuilderEdge* UQuestBuilderNode::GetEdge(UQuestBuilderNode* ChildNode)
{
	return Edges.Contains(ChildNode) ? Edges.FindChecked(ChildNode) : nullptr;
}


FText UQuestBuilderNode::GetDescription_Implementation() const
{
	return Description;
}


void UQuestBuilderNode::LaunchStartEventQueue()
{
	if (StartEventQueue.IsEmpty())
	{
		OnAllStartEventFinished.Broadcast();
		return;
	}
	StartEventQueue[0]->OnEventFinished.AddLambda(
		[this]()
		{
			StartEventQueue.RemoveAt(0);
			LaunchStartEventQueue();
		}
	);
	StartEventQueue[0]->BeginEvent(GetOwningController(), GetOwningPawn());
}

void UQuestBuilderNode::LaunchEndEventQueue()
{
	if (EndEventQueue.IsEmpty())
	{
		OnAllEndEventFinished.Broadcast();
		return;
	}
	EndEventQueue[0]->OnEventFinished.AddLambda(
		[this]()
		{
			EndEventQueue.RemoveAt(0);
			LaunchEndEventQueue();
		}
	);
	EndEventQueue[0]->BeginEvent(GetOwningController(), GetOwningPawn());
	
}

FString UQuestBuilderNode::GetShortTypeName(const UObject* Ob)
{
	if ((Ob == nullptr) || (Ob->GetClass() == nullptr))
	{
		return TEXT("None");
	}

	if (Ob->GetClass()->HasAnyClassFlags(CLASS_CompiledFromBlueprint))
	{
		return Ob->GetClass()->GetName().LeftChop(2);
	}

	FString TypeDesc = Ob->GetClass()->GetName();
	const int32 ShortNameIdx = TypeDesc.Find(TEXT("_"), ESearchCase::CaseSensitive);
	if (ShortNameIdx != INDEX_NONE)
	{
		TypeDesc.MidInline(ShortNameIdx + 1, MAX_int32, EAllowShrinking::No);
	}

	return TypeDesc;
}




#if WITH_EDITOR
bool UQuestBuilderNode::IsNameEditable() const
{
	return true;
}

FLinearColor UQuestBuilderNode::GetBackgroundColor() const
{
	return BackgroundColor;
}

FText UQuestBuilderNode::GetNodeTitle() const
{
	return FText::FromString(UQuestBuilderNode::GetShortTypeName(this));
}

FText UQuestBuilderNode::GetNodeDescription() const
{
	return FText::FromString(TEXT("This is NodeBase"));
}

FText UQuestBuilderNode::GetEventsText() const
{
	FString EventListText;

	for (const UOrionEvent* Event : Events)
	{
		if (Event)
		{
			if(!EventListText.IsEmpty()) EventListText += TEXT("\n");
			EventListText += Event->GetNodeDisplayText();
		}

	}

	return FText::FromString(EventListText);
}

FText UQuestBuilderNode::GetConditionsText() const
{
	FString ConditionListText;

	for (const UOrionDecorator* Condition : Decorators)
	{
		if (Condition)
		{
			if(!ConditionListText.IsEmpty()) ConditionListText += TEXT("\n");

			ConditionListText += Condition->GetNodeDisplayText();
		}

	}

	return FText::FromString(ConditionListText);
}

void UQuestBuilderNode::SetNodeTitle(const FText& NewTitle)
{
}

bool UQuestBuilderNode::CanCreateConnection(UQuestBuilderNode* Other, FText& ErrorMessage)
{
	return true;
}

bool UQuestBuilderNode::CanCreateConnectionTo(UQuestBuilderNode* Other, int32 NumberOfChildrenNodes, FText& ErrorMessage)
{
	if (ChildrenLimitType == EQuestNodeLimits::Limited && NumberOfChildrenNodes >= ChildrenLimit)
	{
		ErrorMessage = FText::FromString("Children limit exceeded");
		return false;
	}

	return CanCreateConnection(Other, ErrorMessage);
	
}

bool UQuestBuilderNode::CanCreateConnectionFrom(UQuestBuilderNode* Other, int32 NumberOfParentNodes, FText& ErrorMessage)
{
	if (ParentLimitType == EQuestNodeLimits::Limited && NumberOfParentNodes >= ParentLimit)
	{
		ErrorMessage = FText::FromString("Parent limit exceeded");
		return false;
	}

	return true;
}

void UQuestBuilderNode::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);

	
}


#endif

bool UQuestBuilderNode::IsLeafNode() const
{
	return ChildrenNodes.Num() == 0;
}


void UQuestBuilderNode::InitializeFromAsset(UQuestBuilderGraph& OwningQuestGraph, UQuest& OwningQuest)
{
	QuestGraph = &OwningQuestGraph;
	Quest = &OwningQuest;
	Quest->AllNodes.Add(this);
	Quest->NodeMap.Emplace(ID, this);
}


UQuestBuilderGraph* UQuestBuilderNode::GetOwningQuestGraph() const
{
	if (!QuestGraph)
		return nullptr;

	return QuestGraph;
}

UQuest* UQuestBuilderNode::GetOwningQuest() const
{
	return Quest;
}


AController* UQuestBuilderNode::GetOwningController() const
{
	return OwningController;
}

APawn* UQuestBuilderNode::GetOwningPawn() const
{
	if (QuestComponent)
		return QuestComponent->GetOwningPawn();
	return nullptr;
}

UQuestComponent* UQuestBuilderNode::GetQuestComponent() const
{
	return QuestComponent;
}

#undef LOCTEXT_NAMESPACE


