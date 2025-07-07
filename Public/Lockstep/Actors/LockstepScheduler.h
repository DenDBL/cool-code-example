// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Info.h"
#include <functional>

#include "LockstepScheduler.generated.h"


typedef TArray<FScheduledAction> FActionsQueue;
typedef TArray<FScheduledAction*> FPlayoutDelayBufferType;
typedef TMap<FGuid,bool> FApprovalBufferType;
typedef TMap<FGuid, TArray<bool>> FApprovementAwaitingBufferType;

USTRUCT()
struct ESNETWORK_API FScheduledAction
{
	GENERATED_BODY()

	FScheduledAction();
	FScheduledAction(const FGuid& InActionID, std::function<bool()> InAction);

	UPROPERTY()
	FGuid ActionID;

	std::function<bool()> Action;

	int32 ConstTickDelay = -1;

	uint64 TickToExecute = 0;

	FDateTime CreationDateTime;

	bool bActionExecuted = false;
};
/**
 * 
 */
UCLASS()
class ESNETWORK_API ALockstepScheduler : public AInfo
{
	GENERATED_BODY()

	FPlayoutDelayBufferType PlayoutDelayBuffer; //Buffer of delayed execution

	UPROPERTY()
	class ULockstepTickSubsystem* LockstepTickSubsystem;

public:

	ALockstepScheduler();

public:

	virtual void BeginPlay() override;

	void EnqueueActionInBuffer(FScheduledAction* Action);

	void ExecuteAction(FScheduledAction* Action);

	FScheduledAction DequeueFirstActionInBuffer();

	void ExecuteFirstActionInBuffer();

	bool GetActionStatus(const FGuid& Guid);

};
