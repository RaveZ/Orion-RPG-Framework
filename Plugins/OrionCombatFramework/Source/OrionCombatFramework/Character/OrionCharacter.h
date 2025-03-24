// Copyright 2025 Ivan Chandra. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "OrionCharacterBase.h"
#include "OrionCharacter.generated.h"

UCLASS()
class ORIONCOMBATFRAMEWORK_API AOrionCharacter : public AOrionCharacterBase
{
	GENERATED_BODY()

public:
	AOrionCharacter();
protected:
	virtual void BeginPlay() override;
	virtual void PossessedBy(AController* NewController) override;
	virtual void OnRep_PlayerState() override;

	/** Combat Interface */
	virtual int32 GetPlayerLevel() override;
	/** end Combat Interface */
private:
	virtual void InitAbilityActorInfo() override;
};
