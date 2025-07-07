// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Info.h"
#include "LockstepActionsExecuter.generated.h"

/**
 * 
 */

UCLASS()
class ESNETWORK_API ALockstepActionsExecuter : public AInfo
{
	GENERATED_BODY()
	
	UPROPERTY()
	class ULockstepTickSubsystem* LockstepTickSubsystem;

public:

	ALockstepActionsExecuter();

public:

	UFUNCTION()
	void ConfirmActionOnServer(FGuid Guid);

	virtual void BeginPlay() override;

	UFUNCTION(NetMulticast,Reliable)
	void CallActionRPC(FGuid Guid, uint64 TickToExecute);

	UFUNCTION(NetMulticast, Reliable)
	void StartLockstepRPC(FDateTime CallTime);

protected:

	virtual void Tick(float DeltaTime) override;
};
