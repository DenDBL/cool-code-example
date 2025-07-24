# Пример CPP кода проекта по многопользовательской стратегии в реальном времени

Класс объекта, ответственного за исполнение отложенных действий 

h

```cpp
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
```

cpp
```cpp
#include "Lockstep/Actors/LockstepScheduler.h"
#include "Logging/StructuredLog.h"
#include "Lockstep/Subsystems/LockstepTickSubsystem.h"
#include "Lockstep/Interfaces/LockstepProcedureCallInterface.h"
#include "GameFramework/PlayerState.h"


FScheduledAction::FScheduledAction()
{
	ActionID = FGuid();
}

FScheduledAction::FScheduledAction(const FGuid& InActionID, std::function<bool()> InAction)
{
	ActionID = InActionID;
	Action = InAction;
	CreationDateTime = FDateTime::UtcNow();
}

ALockstepScheduler::ALockstepScheduler():Super()
{

}

void ALockstepScheduler::BeginPlay()
{
	Super::BeginPlay();

	LockstepTickSubsystem = GetWorld()->GetSubsystem<ULockstepTickSubsystem>();
}

//Method to add any function to delayed execution. Takes action's struct and number of lockstep ticks for execution delay.
void ALockstepScheduler::EnqueueActionInBuffer(FScheduledAction* Action)
{
	int32 InsertIndex = PlayoutDelayBuffer.Num();

	while (InsertIndex > 0 && PlayoutDelayBuffer[InsertIndex - 1]->CreationDateTime > Action->CreationDateTime)
	{
		InsertIndex--;
	}
	PlayoutDelayBuffer.Insert(Action, InsertIndex);
}

void ALockstepScheduler::ExecuteAction(FScheduledAction* Action)
{
	//if (!GetActionStatus(Action->ActionID)) //Check if this action is aproved in local storage.
	//{
	//	checkf(false, TEXT("FATAL DESYNC DETECTED! TRIED TO EXECUTE NOT APROVED ACTION"))
	//	return;
	//}

	bool result = Action->Action(); //Executes action
	Action->bActionExecuted = true;
	LockstepTickSubsystem->ClearApprovedActionID(Action->ActionID);

}

//NOT USED
FScheduledAction ALockstepScheduler::DequeueFirstActionInBuffer()
{
	FScheduledAction ActionsQueue = FScheduledAction();
	if (PlayoutDelayBuffer.Num())
	{
		ActionsQueue = *PlayoutDelayBuffer[0];
		PlayoutDelayBuffer.RemoveAt(0);
	}

	return ActionsQueue;
}

//Calles each lockstep tick. Itterrates through each action and checks its tick to executes. If tick lower or equal, then executes and removes from buffer
void ALockstepScheduler::ExecuteFirstActionInBuffer()
{
	int32 NumberOfExecutedActions = 0;

	for (int32 i = 0; i < PlayoutDelayBuffer.Num(); i++)
	{
		if (!PlayoutDelayBuffer[i]) continue;

		if (!PlayoutDelayBuffer[i]->ActionID.IsValid() || PlayoutDelayBuffer[i]->bActionExecuted) continue;

		uint64 test = LockstepTickSubsystem->GetCurrentTickNum();

		if (PlayoutDelayBuffer[i]->TickToExecute < LockstepTickSubsystem->GetCurrentTickNum())
		{
			UE_LOGFMT(LogTemp, Error, "FATAL DESYNC DETECTED! THE ACTION DIDN'T HAVE TIME TO BE COMPLETED. NETWORK LATENCY CAN BE TOO HIGH");
		}

		if (PlayoutDelayBuffer[i]->TickToExecute > LockstepTickSubsystem->GetCurrentTickNum()) break;

		ExecuteAction(PlayoutDelayBuffer[i]);
		NumberOfExecutedActions++;
	}

	for (int32 i = 0; i < NumberOfExecutedActions; i++)
	{
		PlayoutDelayBuffer.RemoveAt(0, EAllowShrinking::Yes);
	}
}

//Iterates throu all PlayersStates and checks if all Clients approved this action in ApprovedActions list
bool ALockstepScheduler::GetActionStatus(const FGuid& Guid)
{
	for (FConstPlayerControllerIterator PCIter = GetWorld()->GetPlayerControllerIterator(); PCIter; PCIter++)
	{
		APlayerController* PC = Cast<APlayerController>(*PCIter);
		APlayerState* PS = PC->GetPlayerState<APlayerState>();
		
		if (PS->Implements<ULockstepProcedureCallInterface>())
		{
			if (!ILockstepProcedureCallInterface::Execute_GetIsActionApproved(PS, Guid))
			{
				return false;
			}
		}
		else
		{
			return false;
		}
	}
	return true;
}
```

Сабсистема Lockstep механики

h
```cpp
#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "Lockstep/Actors/LockstepScheduler.h"

#include "LockstepTickSubsystem.generated.h"

#define CREATE_MULTICAST_LOCKSTEP_ACTION(MulticastFuncName, ...) \
	{	FGuid Guid = FGuid::NewGuid(); \
		MulticastFuncName(Guid, __VA_ARGS__); }

#define CREATE_MULTICAST_LOCKSTEP_ACTION_NOPARAMS(MulticastFuncName) \
	{	FGuid Guid = FGuid::NewGuid(); \
		MulticastFuncName(Guid); }

#define ADD_AND_APPROVE_LOCKSTEP_ACTION(Action) \
	{	GetWorld()->GetSubsystem<ULockstepTickSubsystem>()->AddActionToAwaitingBuffer(Action);\
		GetWorld()->GetSubsystem<ULockstepTickSubsystem>()->ApproveRequestOnServer(Action->ActionID);\
	 }

DECLARE_MULTICAST_DELEGATE_TwoParams(FLockstepTickSignature, uint64 /* TickNumber */, float /* DeltaTime */)

class ALockstepActionsExecuter;
class ACommonPlayerController;

typedef TMap<FGuid, FScheduledAction*> AwaitingActionsBufferType;

//LOCKSTEP SUBSYSTEM. 
// 
//USE LockstepTickDelegate FOR FUNCTION BINDING
// 
//VIEW DebugRepActor.cpp FOR EXAMPLE OF USING DELAYED EXECUTION THROUGH NETWORK
//VIEW TickingActor.cpp TO SEE THE BASE OF LOCKSTEP TICK
//VIEW LockstepScheduler.cpp TO SEE DELAYED ACTION EXECTUION MECHANISM
//VIEW LockstepSyncer.cpp TO SEE SYNC PROCESS

UCLASS(config = Engine, defaultconfig)
class ESNETWORK_API ULockstepTickSubsystem : public UWorldSubsystem
{
	friend  ALockstepActionsExecuter;
	friend ACommonPlayerController;

	GENERATED_BODY()
	
	UPROPERTY(globalconfig, noclear)
	uint32 DefaultLockstepTickFrequency; //Config

	UPROPERTY(globalconfig, noclear)
	uint32 DefaultLockstepTickDelayNum; //Number of tick to delay //Config

	UPROPERTY()
	uint32 LockstepTickDelayNum;

	UPROPERTY()
	class ATickingActor* TickingActor;

	UPROPERTY()
	class ALockstepScheduler* LockstepScheduler;

	UPROPERTY()
	class ALockstepSyncer* LockstepSyncer;

	UPROPERTY()
	ALockstepActionsExecuter* LockstepActionsExecuter;

	AwaitingActionsBufferType AwaitingActionsBuffer;


public:

	ULockstepTickSubsystem();

public:

	FLockstepTickSignature LockstepTickDelegate; //Delegate which called every lockstep tick

public:

	UFUNCTION()
	void SetTickFrequency(uint32 InTickFrequency);

	UFUNCTION()
	float GetTickInterval();

	UFUNCTION()
	float GetFixedTickInterval() { return (FMath::RoundToFloat((1.0f / DefaultLockstepTickFrequency) * 1000.f) / 1000.f); };

	virtual void OnWorldBeginPlay(UWorld& InWorld) override;

	UFUNCTION()
	void ApproveRequestOnServer(const FGuid& Guid);

	void EnqueueAction(FScheduledAction* Action);

	UFUNCTION()
	uint64 GetCurrentTickNum();

	UFUNCTION()
	float GetLockstepTime();

	UFUNCTION()
	double GetDelay();

	UFUNCTION()
	float GetMaxPing();

	UFUNCTION()
	uint32 CalculateTickNumDelay();

	UFUNCTION()
	uint32 GetDesiredTickNumDelay();

	uint32 CalculateTickCompensation();

	uint32 CalculateTickCompensation(FDateTime CallTime);

	void UpdateTickNumDelay();

	void UpdateTickNumDelay(uint32 InTickNumDelay);

	void SpawnExecuter(UWorld& InWorld);

	void BindObjectsToTick(UWorld& InWorld);

	UFUNCTION()
	void MakeDelay(double Delay);

	UFUNCTION()
	void ClearApprovedActionID(const FGuid& Guid);

	UFUNCTION()
	void MarkLockstepAsLoaded();

	UFUNCTION()
	void TryApproveAndCallAction(const FGuid& Guid);

	UFUNCTION()
	void CallActionEverywhere(const FGuid& Guid);

	void AddActionToAwaitingBuffer(FScheduledAction* Action);

	UFUNCTION()
	void ExecuteActionFromAwaitingBuffer(const FGuid& Guid, uint64 TickToExecute);

	UFUNCTION()
	void StartLockstep();

	UFUNCTION()
	void StartLockstepEverywhere();

protected:

	UFUNCTION()
	void Tick(uint64 TickNumber, float DeltaTime);

};
```
