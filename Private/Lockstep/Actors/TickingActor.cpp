// Fill out your copyright notice in the Description page of Project Settings.

#include "Lockstep/Actors/TickingActor.h"
#include "Lockstep/Subsystems/LockstepTickSubsystem.h"
#include "Kismet/KismetSystemLibrary.h"
//Actor tickrate sets in LockstepSubsystem
ATickingActor::ATickingActor():Super()
{
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.bStartWithTickEnabled = false; // Disabled by default
	PrimaryActorTick.TickGroup = ETickingGroup::TG_PrePhysics;
	bAllowTickBeforeBeginPlay = false;

	bDelayed = false;
	
	TickNum = -1;
	TickInterval = 0.f;
	ElapsedTime = 0.f;
}

//Creates delay by setting next tick's tickrate new value
void ATickingActor::MakeDelay(double Delay)
{
	SetActorTickInterval(TickInterval + float(Delay));
	bDelayed = true;

	UKismetSystemLibrary::PrintString(this,"Delayed " + FString::SanitizeFloat(Delay));
}

void ATickingActor::SetTickInterval(float Interval)
{
	TickInterval = Interval;
	SetActorTickInterval(Interval);
}

void ATickingActor::BeginPlay()
{
	Super::BeginPlay();

	GetWorld()->GetSubsystem<ULockstepTickSubsystem>()->MarkLockstepAsLoaded();
}


void ATickingActor::Tick(float DeltaSeconds)
{
	if (bDelayed)
	{
		SetActorTickInterval(TickInterval);
		bDelayed = false;
	} //Disable delay.

	Super::Tick(DeltaSeconds);

	TickDelegate();
}

void ATickingActor::TickDelegate() //MAIN LOCKSTEP TICK
{
	double TickRealTime = GetWorld()->GetRealTimeSeconds();

	TickNum++;
	if (!TickNum)
	{
		LastTickRealTime = TickRealTime;
	}

	OnTickDelegate.ExecuteIfBound(TickNum, static_cast<float>(TickRealTime - LastTickRealTime)); //Executs Tick inside Lockstep subsytem

	ElapsedTime += GetWorld()->GetRealTimeSeconds() - LastTickRealTime;
	LastTickRealTime = TickRealTime;
		
}
