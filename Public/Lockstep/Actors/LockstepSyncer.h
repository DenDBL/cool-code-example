// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Info.h"
#include "LockstepSyncer.generated.h"

#define SYNC_RATE 1.f

class ULockstepTickSubsystem;
/**
 * 
 */
UCLASS()
class ESNETWORK_API ALockstepSyncer : public AInfo
{
	GENERATED_BODY()

	UPROPERTY()
	ULockstepTickSubsystem* LockstepTickSubsystem;

	UPROPERTY()
	FTimerHandle FireSyncTimer;

public:

	ALockstepSyncer();
	
public:

	virtual void BeginPlay() override;

	void CreateSyncTimer();

protected:

	virtual void Tick(float DeltaTime) override;

	void LockstepTick(uint64 TickNumber, float DeltaTime);

	UFUNCTION()
	void BroadcastMaxDelay();

	UFUNCTION(NetMulticast, Reliable)
	void MulticastSendMaxDelayRPC(const FGuid& Guid, double MaxDelay);

	UFUNCTION(NetMulticast, Reliable)
	void MulticastSendNewTickDelayNumRPC(const FGuid& Guid, uint32 NewTickDelayNum);

	UFUNCTION()
	void MakeDelay(double Delay);

private:

	void StartDelay(double Delay);

	void UpdateTickDelayNum(uint32 NewTickDelayNum);
};
