// Fill out your copyright notice in the Description page of Project Settings.


#include "Lockstep/Actors/LockstepActionsExecuter.h"
#include "Lockstep/Subsystems/LockstepTickSubsystem.h"
#include "Logging/StructuredLog.h"
#include "Engine/EngineBaseTypes.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/PlayerState.h"
#include "Lockstep/Interfaces/LockstepProcedureCallInterface.h"
#include "Net/UnrealNetwork.h"

ALockstepActionsExecuter::ALockstepActionsExecuter(): Super()
{
	bReplicates = true;
	SetReplicateMovement(true);
	NetUpdateFrequency = 30.0f;

	bOnlyRelevantToOwner = false;
	bAlwaysRelevant = true;
	PrimaryActorTick.bCanEverTick = false;
}

void ALockstepActionsExecuter::BeginPlay()
{
	Super::BeginPlay();

	LockstepTickSubsystem = GetWorld()->GetSubsystem<ULockstepTickSubsystem>();

	if (GetWorld()->GetNetMode() == ENetMode::NM_Client || GetWorld()->GetNetMode() == ENetMode::NM_Standalone)
	{
		LockstepTickSubsystem->LockstepActionsExecuter = this;
		//SetRole(ROLE_AutonomousProxy);
		UE_LOGFMT(LogTemp, Warning, "Client {0} {1}", AActor::GetDebugName(GetOwner()), GetLocalRole());
	}
	else
	{
		//SetOwner(GetWorld()->GetFirstPlayerController());
		UE_LOGFMT(LogTemp, Warning, "Server {0} {1}", AActor::GetDebugName(GetOwner()), GetLocalRole());
	}
}

void ALockstepActionsExecuter::CallActionRPC_Implementation(FGuid Guid, uint64 TickToExecute)
{
	if (GetWorld()->GetNetMode() == ENetMode::NM_ListenServer) return;

	LockstepTickSubsystem->ExecuteActionFromAwaitingBuffer(Guid, TickToExecute);
}

void ALockstepActionsExecuter::StartLockstepRPC_Implementation(FDateTime CallTime)
{
	const double Delay = 0.5f ;

	FTimerHandle UnusedHandle;
	GetWorldTimerManager().SetTimer(
		UnusedHandle, [this, Delay](){
			LockstepTickSubsystem->StartLockstep();
			//GEngine->AddOnScreenDebugMessage(-1, 10, FColor::Green, FString::SanitizeFloat(FDateTime::Now().GetMillisecond()));
		}, Delay - (FDateTime::UtcNow() - CallTime).GetTotalSeconds(), false);
}

void ALockstepActionsExecuter::Tick(float DeltaTime)
{

}

void ALockstepActionsExecuter::ConfirmActionOnServer(FGuid Guid)
{
	APlayerController* PC = GetWorld()->GetFirstPlayerController();
	APlayerState* PS = PC->GetPlayerState<APlayerState>();
	if (PS->Implements<ULockstepProcedureCallInterface>())
	{
		ILockstepProcedureCallInterface::Execute_ConfirmActionOnServer(PS, Guid);
	}

}
