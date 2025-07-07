// Fill out your copyright notice in the Description page of Project Settings.


#include "Misc/SimNetworkDebugFunctionLibrary.h"

#include "Lockstep/Interfaces/LockstepProcedureCallInterface.h"
#include "Lockstep/Subsystems/LockstepDebugSubsystem.h"
#include "Lockstep/Subsystems/LockstepTickSubsystem.h"


void USimNetworkDebugFunctionLibrary::AddLockstepDebugItem(UWorld* World, ULockstepTickSubsystem* LockstepTickSubsystem, const FMassEntityHandle EntityHandle,
                                                           const FString FuncName, const FLockstepDebugItem DebugItem)
{
#if WITH_EDITORONLY_DATA
	check(World);
	
	APlayerController* PC = World->GetFirstPlayerController();
	ILockstepProcedureCallInterface* LockstepInterface = Cast<ILockstepProcedureCallInterface>(PC);
	if (LockstepInterface)
	{
		ULockstepDebugSubsystem* LockstepDebugSubsystem = World->GetSubsystem<ULockstepDebugSubsystem>();
		LockstepDebugSubsystem->DebugDataAdditionLock.Lock();
		
		uint8 PlayerIndex = 1;
		if (World->GetNetMode() != ENetMode::NM_Client)
		{
			PlayerIndex = 0;
		}

		// DebugID is expected to be formated like this: "{ActionName}:{EntityIndex}:{EntitySerialNumber}"
		FString DebugID = FuncName + ":" + FString::FromInt(EntityHandle.Index) + ":" + FString::FromInt(EntityHandle.SerialNumber);
		LockstepInterface->AddLockstepDebugDataOnServer(LockstepTickSubsystem->GetCurrentTickNum(), PlayerIndex, DebugID, DebugItem);

		LockstepDebugSubsystem->DebugDataAdditionLock.Unlock();
	}
#endif
}


void USimNetworkDebugFunctionLibrary::AddLockstepCollisionClusterItem(UWorld* World, ULockstepTickSubsystem* LockstepTickSubsystem, int32 ClusterID,
	const FString FuncName, const FLockstepDebugItem DebugItem)
{
#if WITH_EDITORONLY_DATA
	check(World);

	APlayerController* PC = World->GetFirstPlayerController();
	ILockstepProcedureCallInterface* LockstepInterface = Cast<ILockstepProcedureCallInterface>(PC);
	if (LockstepInterface)
	{
		ULockstepDebugSubsystem* LockstepDebugSubsystem = World->GetSubsystem<ULockstepDebugSubsystem>();
		LockstepDebugSubsystem->DebugDataAdditionLock.Lock();

		uint8 PlayerIndex = 1;
		if (World->GetNetMode() != ENetMode::NM_Client)
		{
			PlayerIndex = 0;
		}

		// DebugID is expected to be formated like this: "{ActionName}:{EntityIndex}:{EntitySerialNumber}"
		FString DebugID = FuncName + ":" + FString::FromInt(ClusterID);
		LockstepInterface->AddLockstepDebugDataOnServer(LockstepTickSubsystem->GetCurrentTickNum(), PlayerIndex, DebugID, DebugItem);

		LockstepDebugSubsystem->DebugDataAdditionLock.Unlock();
	}
#endif
}

void USimNetworkDebugFunctionLibrary::AddLockstepDebugItem(UWorld* World, uint64 TickNum, const FMassEntityHandle EntityHandle, const FString FuncName, const FLockstepDebugItem DebugItem)
{
#if WITH_EDITORONLY_DATA
	check(World);

	APlayerController* PC = World->GetFirstPlayerController();
	ILockstepProcedureCallInterface* LockstepInterface = Cast<ILockstepProcedureCallInterface>(PC);
	if (LockstepInterface)
	{
		ULockstepDebugSubsystem* LockstepDebugSubsystem = World->GetSubsystem<ULockstepDebugSubsystem>();
		LockstepDebugSubsystem->DebugDataAdditionLock.Lock();

		uint8 PlayerIndex = 1;
		if (World->GetNetMode() != ENetMode::NM_Client)
		{
			PlayerIndex = 0;
		}

		// DebugID is expected to be formated like this: "{ActionName}:{EntityIndex}:{EntitySerialNumber}"
		FString DebugID = FuncName + ":" + FString::FromInt(EntityHandle.Index) + ":" + FString::FromInt(EntityHandle.SerialNumber);
		LockstepInterface->AddLockstepDebugDataOnServer(TickNum, PlayerIndex, DebugID, DebugItem);

		LockstepDebugSubsystem->DebugDataAdditionLock.Unlock();
	}
#endif
}

void USimNetworkDebugFunctionLibrary::AddLockstepDebugItemDirGridValue(UWorld* World, uint64 TickNum, int32 APID, int32 CellPositionCol, int32 CellPositionRow,
	const FString FuncName, const FLockstepDebugItem DebugItem)
{
#if WITH_EDITORONLY_DATA
	check(World);

	APlayerController* PC = World->GetFirstPlayerController();
	ILockstepProcedureCallInterface* LockstepInterface = Cast<ILockstepProcedureCallInterface>(PC);
	if (LockstepInterface)
	{
		ULockstepDebugSubsystem* LockstepDebugSubsystem = World->GetSubsystem<ULockstepDebugSubsystem>();
		LockstepDebugSubsystem->DebugDataAdditionLock.Lock();

		uint8 PlayerIndex = 1;
		if (World->GetNetMode() != ENetMode::NM_Client)
		{
			PlayerIndex = 0;
		}

		// DebugID is expected to be formated like this: "{ActionName}:{EntityIndex}:{EntitySerialNumber}"
		FString DebugID = FuncName + ":" + FString::FromInt(APID) + ":" + FString::FromInt(CellPositionCol) + ":" + FString::FromInt(CellPositionRow);
		LockstepInterface->AddLockstepDebugDataOnServer(TickNum, PlayerIndex, DebugID, DebugItem);

		LockstepDebugSubsystem->DebugDataAdditionLock.Unlock();
	}
#endif
}

void USimNetworkDebugFunctionLibrary::AddLockstepDebugAbstractItem(UWorld* World, uint64 TickNum, int32 UniqueDebugId,
	const FString FuncName, const FLockstepDebugItem DebugItem)
{
#if WITH_EDITORONLY_DATA
	check(World);

	APlayerController* PC = World->GetFirstPlayerController();
	ILockstepProcedureCallInterface* LockstepInterface = Cast<ILockstepProcedureCallInterface>(PC);
	if (LockstepInterface)
	{
		ULockstepDebugSubsystem* LockstepDebugSubsystem = World->GetSubsystem<ULockstepDebugSubsystem>();
		LockstepDebugSubsystem->DebugDataAdditionLock.Lock();

		uint8 PlayerIndex = 1;
		if (World->GetNetMode() != ENetMode::NM_Client)
		{
			PlayerIndex = 0;
		}

		// DebugID is expected to be formated like this: "{ActionName}:{EntityIndex}:{EntitySerialNumber}"
		FString DebugID = FuncName + ":" + FString::FromInt(UniqueDebugId);
		LockstepInterface->AddLockstepDebugDataOnServer(TickNum, PlayerIndex, DebugID, DebugItem);

		LockstepDebugSubsystem->DebugDataAdditionLock.Unlock();
	}
#endif
}