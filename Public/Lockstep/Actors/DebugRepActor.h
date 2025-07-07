// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "DebugRepActor.generated.h"

//EXAMPLE OF USING LOCKSTEP TICK EXECUTION. OPEN DebugRepActor.cpp FOR MORE INFO

UCLASS()
class ESNETWORK_API ADebugRepActor : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ADebugRepActor();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	UFUNCTION(CallInEditor, BlueprintCallable, Server, Reliable)
	void CallTestOnServerRPC();

	UFUNCTION(CallInEditor,BlueprintCallable, NetMulticast, Reliable)
	void ExecuteOnEachInst(const FGuid& Guid);
};
