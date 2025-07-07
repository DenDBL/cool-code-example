// Fill out your copyright notice in the Description page of Project Settings.


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
