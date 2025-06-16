// Copyright 2025 Ivan Chandra. All Rights Reserved.


#include "QuestBuilderNode.h"
#include "QuestBuilderEdge.h"
#include "Quest.h"
#include "QuestComponent.h"
#include "Event/OrionEvent.h"
#include "Decorator/OrionDecorator.h"
#include "QuestBuilderGraph.h"
#include "QuestBuilderNode_Objective.h"

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

	OnAllEndEventFinished.AddLambda(
		[this]()
		{
			for (int i = 0; i < NextNodes.Num(); i++)
			{
				if (NextNodes.IsValidIndex(i))
				{
					//remove previous nodes from current nodes
					for (auto& ParentNode : NextNodes[i]->ParentNodes)
					{
						if (Quest->CurrentNodes.Contains(ParentNode))
							Quest->CurrentNodes.RemoveSingle(ParentNode);
					}
					Quest->BeginNode(NextNodes[i]);
					NextNodes.RemoveAt(i);
					i--;
				}
			}
		}
	);

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
	
	NextNodes.Empty();
	
	//This Node will executes their children from top to bottom in sequence
	//f none of the children succeeds, this node will stop branching until next quest update.
	for (auto& ChildNode : ChildrenNodes)
	{
		//Evaluate Decorator
		int SumConditionMet = 0;
		for (auto& Condition : ChildNode->Decorators)
		{
			bool bConditionMet = Condition && Condition->InvertCondition ? !Condition->PerformConditionCheck(OwningController, GetOwningPawn()) : Condition->PerformConditionCheck(OwningController, GetOwningPawn());
			if (bConditionMet)
			{
				SumConditionMet++;
			}
		}

		//evaluate if all the parent node is objective and has completed
		bool bCanBranch = true;
		for (auto& ParentNode : ChildNode->ParentNodes)
		{
			if (UQuestBuilderNode_Objective* ObjectiveNode = Cast<UQuestBuilderNode_Objective>(ParentNode))
			{
				if (ObjectiveNode->bOptional)
					continue;
				if(!ObjectiveNode->IsObjectiveCompleted())
					bCanBranch = false;
			}
		}
		
		if (bCanBranch && SumConditionMet >= ChildNode->Decorators.Num())
		{
			NextNodes.AddUnique(ChildNode);
		}
	}

	//Going to next node will begin when all End Events are finished
}

UQuestBuilderEdge* UQuestBuilderNode::GetEdge(UQuestBuilderNode* ChildNode)
{
	return Edges.Contains(ChildNode) ? Edges.FindChecked(ChildNode) : nullptr;
}


FText UQuestBuilderNode::GetNodeDisplayName_Implementation() const
{
	return FText();
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

	FString TypeDesc = Ob->GetClass()->GetName();

	if (Ob->GetClass()->HasAnyClassFlags(CLASS_CompiledFromBlueprint))
	{
		TypeDesc = Ob->GetClass()->GetName().LeftChop(2);
	}

	// Insert space before each capital letter (except the first character) and remove underscores
	// Do not add space if the previous character is uppercase (abbreviation)
	FString Result;
	for (int32 i = 0; i < TypeDesc.Len(); ++i)
	{
		if (TypeDesc[i] != TEXT('_'))
		{
			TCHAR Char = TypeDesc[i];
			if (i > 0 && FChar::IsUpper(Char) && !FChar::IsWhitespace(TypeDesc[i - 1]))
			{
				// Only add space if previous character is not uppercase (not abbreviation)
				if (!FChar::IsUpper(TypeDesc[i - 1]))
				{
					Result += TEXT(" ");
				}
			}
			Result += Char;
		}
	}

	return Result;
}

UWorld* UQuestBuilderNode::GetWorld() const
{
	if (HasAllFlags(RF_ClassDefaultObject))
	{
		// If we are a CDO, we must return nullptr instead of calling Outer->GetWorld() to fool UObject::ImplementsGetWorld.
		return nullptr;
	}

	UObject* Outer = GetOuter();

	while (Outer)
	{
		UWorld* World = Outer->GetWorld();
		if (World)
		{
			return World;
		}

		Outer = Outer->GetOuter();
	}

	return nullptr;
}

FString UQuestBuilderNode::GetShortTag(FGameplayTag Tag, int32 Level) const
{
	FString TagString = Tag.ToString();
	TArray<FString> Parts;
	TagString.ParseIntoArray(Parts, TEXT("."), true);

	if (Level <= 0 || Parts.Num() == 0)
	{
		return TagString;
	}

	int32 StartIndex = FMath::Max(Parts.Num() - Level, 0);
	FString Result;
	for (int32 i = StartIndex; i < Parts.Num(); ++i)
	{
		if (!Result.IsEmpty())
		{
			Result += TEXT(".");
		}
		Result += Parts[i];
	}
	return Result;
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
	return NodeName.Len() ? FText::FromString(NodeName) : FText::FromString(GetShortTypeName(this));
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


