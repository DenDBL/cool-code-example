// Fill out your copyright notice in the Description page of Project Settings.


#include "Lockstep/Subsystems/LockstepTickSubsystem.h"
#include "Lockstep/Actors/TickingActor.h"
#include "Lockstep/Actors/LockstepScheduler.h"
#include "Lockstep/Actors/LockstepActionsExecuter.h"
#include "Lockstep/Actors/LockstepSyncer.h"
#include "Logging/StructuredLog.h"
#include "GameFramework/PlayerState.h"
#include "Lockstep/Interfaces/LockstepProcedureCallInterface.h"
#include "Kismet/KismetSystemLibrary.h"
#include "MassEntitySubsystem.h"

ULockstepTickSubsystem::ULockstepTickSubsystem():Super()
{
	DefaultLockstepTickFrequency = 15;

	DefaultLockstepTickDelayNum = 1;
	LockstepTickDelayNum = DefaultLockstepTickDelayNum;
}

void ULockstepTickSubsystem::SetTickFrequency(uint32 InTickFrequency)
{
	TickingActor->SetTickInterval(1.0f / InTickFrequency);
}

float ULockstepTickSubsystem::GetTickInterval()
{
	return TickingActor->TickInterval;
}

void ULockstepTickSubsystem::OnWorldBeginPlay(UWorld& InWorld)
{
	TickingActor = InWorld.SpawnActor<ATickingActor>(ATickingActor::StaticClass(), FTransform());
	check(TickingActor);

	LockstepTickDelayNum = DefaultLockstepTickDelayNum;
	SetTickFrequency(DefaultLockstepTickFrequency);

	TickingActor->OnTickDelegate.BindUObject(this, &ThisClass::Tick);

	LockstepScheduler = InWorld.SpawnActor<ALockstepScheduler>(ALockstepScheduler::StaticClass(), FTransform());
	check(LockstepScheduler);

	LockstepSyncer = InWorld.SpawnActor<ALockstepSyncer>(ALockstepSyncer::StaticClass(), FTransform());
	check(LockstepSyncer);

	SpawnExecuter(InWorld);
	BindObjectsToTick(InWorld);
}

void ULockstepTickSubsystem::SpawnExecuter(UWorld& InWorld)
{
	FActorSpawnParameters SpawnParameters = FActorSpawnParameters();

	if (GetWorld()->GetNetMode() == ENetMode::NM_ListenServer || GetWorld()->GetNetMode() == ENetMode::NM_DedicatedServer)
	{
		LockstepActionsExecuter = GetWorld()->SpawnActor<ALockstepActionsExecuter>(ALockstepActionsExecuter::StaticClass(), FTransform(), SpawnParameters);
		check(LockstepActionsExecuter);
	}
}

void ULockstepTickSubsystem::BindObjectsToTick(UWorld& InWorld)
{
	FMassEntityManager* EM = &GetWorld()->GetSubsystem<UMassEntitySubsystem>()->GetMutableEntityManager();
	LockstepTickDelegate.AddRaw(EM, &FMassEntityManager::OnLockstepTick);
}

//Delays execution of next tick
void ULockstepTickSubsystem::MakeDelay(double Delay)
{
	TickingActor->MakeDelay(Delay);
}

//Clears approved action
void ULockstepTickSubsystem::ClearApprovedActionID(const FGuid& Guid)
{
	APlayerController* PC = GetWorld()->GetFirstPlayerController();
	APlayerState* PS = PC->GetPlayerState<APlayerState>();

	if (PS->Implements<ULockstepProcedureCallInterface>())
	{
		ILockstepProcedureCallInterface::Execute_ClearConfirmedActionOnServer(PS, Guid);
	}
}


void ULockstepTickSubsystem::MarkLockstepAsLoaded()
{
	APlayerController* PC = GetWorld()->GetFirstPlayerController();

	if (PC->Implements<ULockstepProcedureCallInterface>())
	{
		ILockstepProcedureCallInterface::Execute_MarkLockstepAsLoaded(PC);
	}
}

// if all instances and server confirm action, server call's this action on every instance from awaiting buffer by adding to execution queue.
void ULockstepTickSubsystem::TryApproveAndCallAction(const FGuid& Guid)
{
	if (LockstepScheduler->GetActionStatus(Guid))
	{
		CallActionEverywhere(Guid);
	}
}

//Adds action by guid to Queue on every intance
void ULockstepTickSubsystem::CallActionEverywhere(const FGuid& Guid)
{
	//UE_LOG(LogTemp, Log, TEXT("ULockstepTickSubsystem::CallActionEverywhere: %s"), *Guid.ToString());

	FDateTime CallTime = FDateTime::UtcNow();
	UpdateTickNumDelay();

	if (GetWorld()->GetNetMode() == ENetMode::NM_ListenServer) //Add to queue directly on server
		ExecuteActionFromAwaitingBuffer(Guid, GetCurrentTickNum() + GetDesiredTickNumDelay());

	LockstepActionsExecuter->CallActionRPC(Guid,GetCurrentTickNum() + GetDesiredTickNumDelay());
}

//Add action to array and make this action await approvement
void ULockstepTickSubsystem::AddActionToAwaitingBuffer(FScheduledAction* Action)
{
	AwaitingActionsBuffer.Add(Action->ActionID, Action);
}

void ULockstepTickSubsystem::ExecuteActionFromAwaitingBuffer(const FGuid& Guid,uint64 TickToExecute)
{
	FScheduledAction* Action;
	AwaitingActionsBuffer.RemoveAndCopyValue(Guid, Action);
	Action->TickToExecute = TickToExecute;

	EnqueueAction(Action);
	
}

//Start lockstep locally
void ULockstepTickSubsystem::StartLockstep()
{
	//GEngine->AddOnScreenDebugMessage(-1, 20.f, FColor::Emerald,FString::FromInt(FDateTime::UtcNow().GetMillisecond()));
	TickingActor->SetActorTickEnabled(true);
	LockstepSyncer->CreateSyncTimer();
}

//start lockstep everywhere. Executes on server
void ULockstepTickSubsystem::StartLockstepEverywhere()
{
	LockstepActionsExecuter->StartLockstepRPC(FDateTime::UtcNow());
}

//Calls request approvement on server. See ConfirmActionOnServer
void ULockstepTickSubsystem::ApproveRequestOnServer(const FGuid& Guid)
{
	LockstepActionsExecuter->ConfirmActionOnServer(Guid);
}

void ULockstepTickSubsystem::EnqueueAction(FScheduledAction* Action)
{
	//int32 DeltaTickNum = int32(PositionInQueue) - int32(CalculateTickCompensation());
	//PositionInQueue = FMath::Max(1, DeltaTickNum); TODO: Disabled because of wrong compensation's calculation
	LockstepScheduler->EnqueueActionInBuffer(Action);
}

//Number of current tick
uint64 ULockstepTickSubsystem::GetCurrentTickNum()
{
	return TickingActor->TickNum ;
}

float ULockstepTickSubsystem::GetLockstepTime()
{
	return (float)GetCurrentTickNum() * GetFixedTickInterval();
}

//Delay of lockstep tick, which was occurred by lags or anything else 
double ULockstepTickSubsystem::GetDelay()
{
	//UE_LOGFMT(LogTemp, Warning, "Returned delay: {0}. Server: {1}", TickingActor->ElapsedTime - GetCurrentTickNum() * TickingActor->TickInterval, GetWorld()->GetNetMode()==NM_ListenServer);
	return	FMath::Max(0,TickingActor->ElapsedTime - (GetCurrentTickNum()-1) * TickingActor->TickInterval);
}

//Max Ping of all clients
float ULockstepTickSubsystem::GetMaxPing()
{
	float MaxPing = 0.f;

	for (FConstPlayerControllerIterator PCIter = GetWorld()->GetPlayerControllerIterator(); PCIter; PCIter++)
	{
		APlayerController* PC = Cast<APlayerController>(*PCIter);
		APlayerState* PS = PC->GetPlayerState<APlayerState>();

		float Ping = PS->GetPingInMilliseconds();

		if (Ping > MaxPing)
		{
			MaxPing = Ping;
		}
	}

	return MaxPing;
}

uint32 ULockstepTickSubsystem::CalculateTickNumDelay()
{
	uint32 DelayNum = DefaultLockstepTickDelayNum;
	uint32 AdditionalDelayNum = FMath::CeilToInt32(GetMaxPing() / (1.f / TickingActor->TickInterval));

	return DelayNum+AdditionalDelayNum;
}

uint32 ULockstepTickSubsystem::GetDesiredTickNumDelay()
{
	return LockstepTickDelayNum;
}

//NOT USED
//Tick compensation based on ping. TODO: Needs a new way of finding tick compensation.
uint32 ULockstepTickSubsystem::CalculateTickCompensation()
{
	if (GetWorld()->GetNetMode() == NM_ListenServer) return 0;


	APlayerController* PC = GetWorld()->GetFirstPlayerController();
	APlayerState* PS = PC->GetPlayerState<APlayerState>();

	float Ping = PS->GetPingInMilliseconds();
	uint32 Compensation = FMath::CeilToInt32(Ping / 2.f / TickingActor->TickInterval);

	return Compensation;
}

//NOT USED
//Tick compensation based on call time
uint32 ULockstepTickSubsystem::CalculateTickCompensation(FDateTime CallTime)
{
	if(GetWorld()->GetNetMode() == NM_ListenServer) return 0;

	double Latency = (FDateTime::UtcNow() - CallTime).GetTotalSeconds();
	uint32 Compensation = FMath::FloorToInt32(Latency / TickingActor->TickInterval);

	return Compensation;
}

//Updates action delay to execute
void ULockstepTickSubsystem::UpdateTickNumDelay()
{

	uint32 NewTickNumDelay = CalculateTickNumDelay();
	if (NewTickNumDelay > LockstepTickDelayNum)
	{
		UpdateTickNumDelay(NewTickNumDelay);
	}
}

//Updates number of ticks to pass until action execution
void ULockstepTickSubsystem::UpdateTickNumDelay(uint32 InTickNumDelay)
{
	UE_LOGFMT(LogTemp, Warning, "NewTickNumDelay: {0}", InTickNumDelay);
	LockstepTickDelayNum = InTickNumDelay;
}

//LOCKSTEP TICK
void ULockstepTickSubsystem::Tick(uint64 TickNumber, float DeltaTime)
{
	TRACE_CPUPROFILER_EVENT_SCOPE_TEXT(*FString::Printf(TEXT("ULockstepTickSubsystem::Tick %s"), *FString::FromInt(TickNumber)));
	//UE_LOGFMT(LogTemp, Warning, "Tick: {0}", TickNumber);
	//GEngine->AddOnScreenDebugMessage(-1, 1.f / DefaultLockstepTickFrequency, FColor::Green, "Tick " + FString::FromInt(TickNumber));

	//UKismetSystemLibrary::PrintString(GetWorld(), "Tick " + FString::FromInt(TickNumber), true, true, FColor::Green, GetTickInterval());

	LockstepScheduler->ExecuteFirstActionInBuffer();

	LockstepTickDelegate.Broadcast(TickNumber, DeltaTime);
}

	