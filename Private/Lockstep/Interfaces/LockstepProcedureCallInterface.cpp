// Fill out your copyright notice in the Description page of Project Settings.


#include "Lockstep/Interfaces/LockstepProcedureCallInterface.h"

// Add default functionality here for any ILockstepProcedureCallInterface functions that are not pure virtual.

bool ILockstepProcedureCallInterface::SendClientLockstepDelay(double Delay, uint64 TickNum)
{
    return false;
}

void ILockstepProcedureCallInterface::AddLockstepDebugDataOnServer(const int64 LockstepSecond, const uint8 PlayerIndex, const FString DebugID,
    const FLockstepDebugItem& DebugData)
{
}
