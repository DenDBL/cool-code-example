// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "LockstepProcedureCallInterface.generated.h"

struct FLockstepDebugItem;
// This class does not need to be modified.
UINTERFACE(MinimalAPI)
class ULockstepProcedureCallInterface : public UInterface
{
	GENERATED_BODY()
};

/**
 * 
 */
class ESNETWORK_API ILockstepProcedureCallInterface
{
	GENERATED_BODY()

	// Add interface functions to this class. This is the class that will be inherited to implement this interface.
public:

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Lockstep")
	bool ConfirmActionOnServer(const FGuid& Guid);

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Lockstep")
	bool ClearConfirmedActionOnServer(const FGuid& Guid);

	UFUNCTION(Category = "Lockstep")
	virtual bool SendClientLockstepDelay(double Delay, uint64 TickNum);

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Lockstep")
	double GetDelay();

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Lockstep")
	FDateTime GetLastDelayUpdateTime();

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Lockstep")
	bool GetIsPlayerDelaying();

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Lockstep")
	void SetPlayerDelaying(bool IsDelaying);

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Lockstep")
	bool GetIsActionApproved(const FGuid& Guid);

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Lockstep")
	bool MarkLockstepAsLoaded();

	// DEBUG ------

	UFUNCTION(Category = "Lockstep|Debug")
	virtual void AddLockstepDebugDataOnServer(const int64 LockstepSecond, const uint8 PlayerIndex,
									const FString DebugID, const FLockstepDebugItem& DebugData);
};
