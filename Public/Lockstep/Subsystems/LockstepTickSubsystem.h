// Fill out your copyright notice in the Description page of Project Settings.

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
