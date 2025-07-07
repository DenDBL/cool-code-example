// Fill out your copyright notice in the Description page of Project Settings.


#include "Lockstep/Actors/DebugRepActor.h"
#include "Lockstep/Subsystems/LockstepTickSubsystem.h"
#include "Lockstep/Actors/LockstepScheduler.h"
// Sets default values
ADebugRepActor::ADebugRepActor()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	bReplicates = true;
	NetUpdateFrequency = 30.0f;
	bAlwaysRelevant = true;
}

// Called when the game starts or when spawned
void ADebugRepActor::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void ADebugRepActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

//1. Entry point of calling any action. The RPC executed on server from the Client/Listen Server.
void ADebugRepActor::CallTestOnServerRPC_Implementation()
{
	CREATE_MULTICAST_LOCKSTEP_ACTION_NOPARAMS(ExecuteOnEachInst) //Macros to create a call with unique ID and make it await aprovement

		//Signature looks like this
	/*{
		FGuid Guid = FGuid::NewGuid(); - We create an unique ID for each action. That ID is stored on every instance until action won't be executed
		MulticastFuncName(Guid);	- Execute multicast. View step 2
	}*/
}

//2. This action executes on each Instance (Client/ Server). We dont't pass Func body through network, only any parameters for it
void ADebugRepActor::ExecuteOnEachInst_Implementation(const FGuid& Guid)
{
	ULockstepTickSubsystem* LockstepTickSubsystem = GetWorld()->GetSubsystem<ULockstepTickSubsystem>();

	FScheduledAction* Action = new FScheduledAction(Guid, [this]() // - We are creating an Action-struct which stores passed from server Guid and Our Func to execute
		{
			if (!this) return false; //CHECK RPLICATED ACTOR IF BEING DESTROYED

			FString NM = UEnum::GetValueAsString(GetLocalRole());
			if (GetWorld()->GetNetMode() == ENetMode::NM_Client)
			{
				
				GEngine->AddOnScreenDebugMessage(-1, 1, FColor::Red, FString::SanitizeFloat(FDateTime::Now().GetMillisecond()));
				GEngine->AddOnScreenDebugMessage(-1, 10, FColor::Red, NM + " Tick: " + FString::FromInt(GetWorld()->GetSubsystem<ULockstepTickSubsystem>()->GetCurrentTickNum()));
				
			}
			else
			{
				GEngine->AddOnScreenDebugMessage(-1, 1, FColor::Green, FString::SanitizeFloat(FDateTime::Now().GetMillisecond()));
				GEngine->AddOnScreenDebugMessage(-1, 10, FColor::Green, NM + " Tick: " + FString::FromInt(GetWorld()->GetSubsystem<ULockstepTickSubsystem>()->GetCurrentTickNum()));
			}


			return true;
		});

	if (true) //We can do additional checks in case actor can't do any actions because of desync
	{
		ADD_AND_APPROVE_LOCKSTEP_ACTION(Action) //Adds this action to delayed execution by lockstep. View LockstepScheduler.cpp for next steps
												//Approves this action locally and on server. 

			/* Signature
				GetWorld()->GetSubsystem<ULockstepTickSubsystem>()->AddActionToAwaitingBuffer(Action); - Adds action to buffer, that awaits command from server
				GetWorld()->GetSubsystem<ULockstepTickSubsystem>()->ApproveRequestOnServer(Action->ActionID); - Confirm action on server
			*/
	}
}