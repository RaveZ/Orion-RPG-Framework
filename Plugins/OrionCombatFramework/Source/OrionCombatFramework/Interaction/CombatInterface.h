#pragma once
 
#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "CombatInterface.generated.h"
 
class UAnimMontage;

UINTERFACE(MinimalAPI, BlueprintType)
class UCombatInterface : public UInterface
{
	GENERATED_BODY()
};
 
/**
 * 
 */
class ORIONCOMBATFRAMEWORK_API ICombatInterface
{
	GENERATED_BODY()
 
	
public:
	UFUNCTION(BlueprintNativeEvent)
	int32 GetPlayerLevel();

	virtual FVector GetCombatSocketLocation();

	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable)
	void UpdateFacingTarget(const FVector& Target);

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	UAnimMontage* GetHitReactMontage();

	virtual void Die() = 0;
};