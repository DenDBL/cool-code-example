// Fill out your copyright notice in the Description page of Project Settings.


#include "Lockstep/Actors/LockstepSyncer.h"
#include "Lockstep/Subsystems/LockstepTickSubsystem.h"
#include "Lockstep/Interfaces/LockstepProcedureCallInterface.h"
#include "GameFramework/PlayerState.h"
#include "GameFramework/PlayerController.h"
#include "Lockstep/Actors/LockstepScheduler.h"
#include "Kismet/GameplayStatics.h"
#include "Logging/StructuredLog.h"

ALockstepSyncer::ALockstepSyncer():Super()
{
	bReplicates = true;
	bAlwaysRelevant = true;

	NetPriority = 10.f;

	PrimaryActorTick.bTickEvenWhenPaused = true;
	PrimaryActorTick.bCanEverTick = true;
}

void ALockstepSyncer::BeginPlay()
{
	LockstepTickSubsystem = GetWorld()->GetSubsystem<ULockstepTickSubsystem>();
	LockstepTickSubsystem->LockstepTickDelegate.AddUObject(this, &ThisClass::LockstepTick);

}

void ALockstepSyncer::CreateSyncTimer()// The looped timer.  
{
	GetWorldTimerManager().SetTimer(FireSyncTimer, this, &ThisClass::BroadcastMaxDelay, SYNC_RATE, true);
}

void ALockstepSyncer::Tick(float DeltaTime)
{
}

//We sending client's delay every lockstep-tick
void ALockstepSyncer::LockstepTick(uint64 TickNumber, float DeltaTime)
{
	double Delay = LockstepTickSubsystem->GetDelay();
	
	APlayerState* PS = GetWorld()->GetFirstPlayerController()->GetPlayerState<APlayerState>();

	ILockstepProcedureCallInterface* Interface = Cast<ILockstepProcedureCallInterface>(PS);
	if (Interface)
	{
		Interface->SendClientLockstepDelay(Delay, TickNumber);
	}
}

//Called on server. We sending the max delay of every client and every client calculates by itself how long pause of tick has to ber
void ALockstepSyncer::BroadcastMaxDelay()
{
	if (GetWorld()->GetNetMode() != NM_ListenServer) return;

	double MaxDelay = 0.f;

	for (FConstPlayerControllerIterator PCIter = GetWorld()->GetPlayerControllerIterator(); PCIter; PCIter++) //Gets max delay of every client
	{
		APlayerController* PC = Cast<APlayerController>(*PCIter);
		APlayerState* PS = PC->GetPlayerState<APlayerState>();
		
		if (PS->Implements<ULockstepProcedureCallInterface>())
		{

			if (ILockstepProcedureCallInterface::Execute_GetIsPlayerDelaying(PS)) //Checks if noone is delaying right now
			{
				return;
			}

			double Delay = ILockstepProcedureCallInterface::Execute_GetDelay(PS);

			if (Delay > MaxDelay)
			{
				MaxDelay = Delay;
			}
		}
	}

	CREATE_MULTICAST_LOCKSTEP_ACTION(MulticastSendMaxDelayRPC, MaxDelay); //Sending delay to instances

	for (FConstPlayerControllerIterator PCIter = GetWorld()->GetPlayerControllerIterator(); PCIter; PCIter++) //Mark all player states as delayig
	{
		APlayerController* PC = Cast<APlayerController>(*PCIter);
		APlayerState* PS = PC->GetPlayerState<APlayerState>();

		if (PS->Implements<ULockstepProcedureCallInterface>())
		{
			ILockstepProcedureCallInterface::Execute_SetPlayerDelaying(PS, true);
		}
	}
}

void ALockstepSyncer::MakeDelay(double Delay)
{
	//GEngine->AddOnScreenDebugMessage(-1, 3.f, FColor::Green, "Tried to Delay " + FString::SanitizeFloat(Delay));
	if (FMath::IsNearlyZero(Delay, 0.00001f))
	{
		return;
	}

	StartDelay(Delay);
}

//Function to start delay
void ALockstepSyncer::StartDelay(double Delay)
{
	LockstepTickSubsystem->MakeDelay(Delay);
}

void ALockstepSyncer::UpdateTickDelayNum(uint32 NewTickDelayNum)
{
	LockstepTickSubsystem->UpdateTickNumDelay(NewTickDelayNum);
}

void ALockstepSyncer::MulticastSendMaxDelayRPC_Implementation(const FGuid& Guid, double MaxDelay)
{
	double DeltaDelay = FMath::Clamp(MaxDelay - LockstepTickSubsystem->GetDelay(), 0, 15);

	FScheduledAction* Action = new FScheduledAction(Guid, [this, DeltaDelay]()
		{
			

			MakeDelay(DeltaDelay); //Delay

			FTimerHandle UnusedHandle;
			GetWorldTimerManager().SetTimer( //Disable delaying state on players state 
				UnusedHandle, [this]()
				{
					APlayerController* PC = GetWorld()->GetFirstPlayerController();
					APlayerState* PS = PC->GetPlayerState<APlayerState>();

					if (PS->Implements<ULockstepProcedureCallInterface>())
					{
						ILockstepProcedureCallInterface::Execute_SetPlayerDelaying(PS, false);
					}
				}, LockstepTickSubsystem->GetTickInterval() + DeltaDelay, false);
			
			return true;
		});

	ADD_AND_APPROVE_LOCKSTEP_ACTION(Action)
}

void ALockstepSyncer::MulticastSendNewTickDelayNumRPC_Implementation(const FGuid& Guid, uint32 NewTickDelayNum)
{
	FScheduledAction* Action = new FScheduledAction(Guid, [this, NewTickDelayNum]()
		{
			UpdateTickDelayNum(NewTickDelayNum);

			return true;
		});

	ADD_AND_APPROVE_LOCKSTEP_ACTION(Action)
}