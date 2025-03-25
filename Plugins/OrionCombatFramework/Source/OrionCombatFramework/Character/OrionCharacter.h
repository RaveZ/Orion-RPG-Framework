// Copyright 2025 Ivan Chandra. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "OrionCharacterBase.h"
#include "../Interaction/PlayerInterface.h"
#include "OrionCharacter.generated.h"

UCLASS()
class ORIONCOMBATFRAMEWORK_API AOrionCharacter : public AOrionCharacterBase, public IPlayerInterface
{
	GENERATED_BODY()

public:
	AOrionCharacter();
protected:
	virtual void BeginPlay() override;
	virtual void PossessedBy(AController* NewController) override;
	virtual void OnRep_PlayerState() override;

	/** Players Interface */
	virtual void AddToXP_Implementation(int32 InXP) override;
	virtual void LevelUp_Implementation() override;
	/** end Player Interface */

	/** Combat Interface */
	virtual int32 GetPlayerLevel_Implementation() override;
	/** end Combat Interface */
private:
	virtual void InitAbilityActorInfo() override;
};
