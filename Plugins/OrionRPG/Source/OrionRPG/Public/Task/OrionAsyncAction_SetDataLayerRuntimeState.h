// Copyright 2025 Ivan Chandra. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintAsyncActionBase.h"
#include "OrionAsyncAction_SetDataLayerRuntimeState.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnSetDataLayerComplete, bool, Success);

UCLASS(BlueprintType, meta = (ExposedAsyncProxy = "AsyncTask"))
class UOrionAsyncAction_SetDataLayerRuntimeState : public UBlueprintAsyncActionBase
{
	GENERATED_BODY()

public:
	// This function set data layer runtime and wait until streaming completed
	UFUNCTION(BlueprintCallable, meta = (DisplayName = "Set Data Layer Runtime State Async", BlueprintInternalUseOnly = "true", WorldContext = "WorldContextObject"), Category = "Utilities|DataLayer")
	static UOrionAsyncAction_SetDataLayerRuntimeState* SetDataLayerRuntimeStateAsync(UObject* WorldContextObject, const UDataLayerAsset* DataLayerAsset, EDataLayerRuntimeState InState, bool bInIsRecursive);

	// Completed delegate
	UPROPERTY(BlueprintAssignable)
	FOnSetDataLayerComplete OnCompleted;

	// Called when the async node is invoked
	virtual void Activate() override;

private:
	// params
	TWeakObjectPtr<const UObject> WorldContextObjectPtr;
	const UDataLayerAsset* DataLayerAssetPtr;
	EDataLayerRuntimeState DesiredState;
	bool bRecursive;

	FTimerHandle PollTimerHandle;

	void PollStreaming();
};
