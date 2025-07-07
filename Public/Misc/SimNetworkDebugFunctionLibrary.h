// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "SimNetworkDebugFunctionLibrary.generated.h"

struct FLockstepDebugItem;
struct FMassEntityHandle;
class ULockstepTickSubsystem;
/**
 * 
 */
UCLASS()
class ESNETWORK_API USimNetworkDebugFunctionLibrary : public UObject
{
	GENERATED_BODY()

public:

	// @warning Currently works correctly with only two players: one on server and one on client.
	static void AddLockstepDebugItem(UWorld* World, ULockstepTickSubsystem* LockstepTickSubsystem, const FMassEntityHandle EntityHandle,
	                                 const FString FuncName, const FLockstepDebugItem DebugItem);


	static void AddLockstepCollisionClusterItem(UWorld* World, ULockstepTickSubsystem* LockstepTickSubsystem, int32 ClusterID,
		const FString FuncName, const FLockstepDebugItem DebugItem);

	static void AddLockstepDebugItem(UWorld* World, uint64 TickNum, const FMassEntityHandle EntityHandle,
		const FString FuncName, const FLockstepDebugItem DebugItem);

	static void AddLockstepDebugItemDirGridValue(UWorld* World, uint64 TickNum, int32 APID, int32 CellPositionCol, int32 CellPositionRow,
		const FString FuncName, const FLockstepDebugItem DebugItem);

	static void AddLockstepDebugAbstractItem(UWorld* World, uint64 TickNum, int32 UniqueDebugId,
		const FString FuncName, const FLockstepDebugItem DebugItem);
};
