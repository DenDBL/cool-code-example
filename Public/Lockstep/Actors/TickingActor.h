// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Info.h"
#include "TickingActor.generated.h"

/**
 * 
 */
DECLARE_DELEGATE_TwoParams(FTickSignature, uint64 TickNumber , float DeltaTime)

UCLASS()
class ESNETWORK_API ATickingActor : public AInfo
{
	friend class ULockstepTickSubsystem;

	GENERATED_BODY()

	FTickSignature OnTickDelegate;

	UPROPERTY()
	uint64 TickNum;

	UPROPERTY()
	double ElapsedTime;

	UPROPERTY()
	double LastTickRealTime;

	UPROPERTY()
	bool bDelayed;

	UPROPERTY()
	float TickInterval;

public:

	ATickingActor();

	void MakeDelay(double Delay);

	void SetTickInterval(float Interval);

	virtual void BeginPlay() override;

protected:

	virtual void Tick(float DeltaSeconds) override;

	UFUNCTION()
	void TickDelegate();
};
