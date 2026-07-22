// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "MassProcessor.h"
#include "MassEntityQuery.h" 
#include "DeathProcessor.generated.h"

/**
 * 
 */
UCLASS()
class ORIONRPGFRAMEWORK_API UDeathProcessor : public UMassProcessor
{
	GENERATED_BODY()

public:
	UDeathProcessor();

protected:
	virtual void ConfigureQueries(const TSharedRef<FMassEntityManager>& EntityManager) override;
	virtual void Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context) override;

private:
	FMassEntityQuery EntityQuery;
};
