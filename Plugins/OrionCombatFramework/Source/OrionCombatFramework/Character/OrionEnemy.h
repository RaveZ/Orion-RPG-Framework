// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "OrionCharacterBase.h"
#include "OrionCombatFramework/Interaction/EnemyInterface.h"
#include "OrionEnemy.generated.h"

UCLASS()
class ORIONCOMBATFRAMEWORK_API AOrionEnemy : public AOrionCharacterBase, public IEnemyInterface
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	AOrionEnemy();
	
	virtual void HighlightActor() override;
	virtual void UnHighlightActor() override;

	/** Combat Interface */
	virtual int32 GetPlayerLevel() override;
	/** end Combat Interface */

protected:
	virtual void BeginPlay() override;
	virtual void InitAbilityActorInfo() override;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Orion | Character Class Defaults")
	int32 Level = 1;
};
