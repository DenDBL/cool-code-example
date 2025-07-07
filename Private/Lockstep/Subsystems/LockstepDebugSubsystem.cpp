// Fill out your copyright notice in the Description page of Project Settings.


#include "Lockstep/Subsystems/LockstepDebugSubsystem.h"

#include "MassCommonFragments.h"
#include "MassEntityManager.h"
#include "MassEntitySubsystem.h"

#pragma optimize ("", off)

ULockstepDebugSubsystem::ULockstepDebugSubsystem()
{
	// CONFIG START ------
	bStopOnFirstUnsyncDetection = false;
	// CONFIG END
	
	bUnsyncDetected = false;
}

void ULockstepDebugSubsystem::AddDebugData(const int64 LockstepSecond, const UE::Lockstep::FPlayerIndex PlayerIndex, const UE::Lockstep::FDebugID DebugID,
                                           const FLockstepDebugItem DebugData)
{
#if WITH_EDITORONLY_DATA
	DebugDataAdditionLock.Lock();
	if (bStopOnFirstUnsyncDetection && bUnsyncDetected)
	{
		return;
	}
	
	CurrentDebuggedLockstepSecond = LockstepSecond;

	//if (CurrentDebuggedLockstepSecond != LockstepSecond)
	//{
	//	ProcessAndClearCachedDebugData();
	//	
	//}
	if (PlayerIndex == 0)
	{
		if (!FirstPlayerDebugMaps.FindOrAdd(LockstepSecond).Contains(DebugID))
		{
			FirstPlayerDebugMaps.FindOrAdd(LockstepSecond).Add(DebugID, DebugData);
		}

	}
	else
	{
		if (!SecondPlayerDebugMaps.FindOrAdd(LockstepSecond).Contains(DebugID))
		{
			SecondPlayerDebugMaps.FindOrAdd(LockstepSecond).Add(DebugID, DebugData);
		}
	}

	ProcessAndClearCachedDebugData();

	DebugDataAdditionLock.Unlock();
#endif
}

void ULockstepDebugSubsystem::ProcessAndClearCachedDebugData()
{
	using UE::Lockstep::FDebugID;
	
	bool bDiscrepancyFound = false;
	bool bDifferentCallsAmount = false;
	TArray<FDebugID> InvalidDebugIDs;
	
	TArray<uint64> ProcessedTicks;

	//for (uint8 PlayerIdx = 1; PlayerIdx < CachedDebugData.Num(); PlayerIdx++)
	//{
	//	const TMap<FDebugID, FLockstepDebugItem>& DebugItems1 = CachedDebugData[PlayerIdx - 1];
	//	const TMap<FDebugID, FLockstepDebugItem>& DebugItems2 = CachedDebugData[PlayerIdx];

	//	if (DebugItems1.Num() != DebugItems2.Num())
	//	{
	//		bDiscrepancyFound = true;
	//		bDifferentCallsAmount = true;
	//	}

	//	for (auto& [DebugID, LockstepDebugItem] : DebugItems1)
	//	{
	//		const FLockstepDebugItem* DebugItem2 = DebugItems2.Find(DebugID);
	//		if (!DebugItem2 || LockstepDebugItem != *DebugItem2)
	//		{
	//			bDiscrepancyFound = true;
	//			InvalidDebugIDs.Add(DebugID);

	//			if (FirstUnsyncedDebugID.IsEmpty())
	//			{
	//				FirstUnsyncedDebugID = DebugID;
	//			}
	//		}
	//	}
	//}
	
	TArray<uint64> SortedTickKeys;
	FirstPlayerDebugMaps.GetKeys(SortedTickKeys);
	SortedTickKeys.Sort();

	for (auto& FirstPlayerDebugPairKey : SortedTickKeys)
	{
		const DebugDataMap& DataMap = FirstPlayerDebugMaps[FirstPlayerDebugPairKey];


		if (FirstPlayerDebugMaps.Num() <= 1) break;
		if (SecondPlayerDebugMaps.Num() <= 1) break;

		const TMap<FDebugID, FLockstepDebugItem>& FirstPlayerDebugItems = DataMap;
		if (!SecondPlayerDebugMaps.Contains(FirstPlayerDebugPairKey) || !SecondPlayerDebugMaps.Contains(FirstPlayerDebugPairKey + 1))
		{
			//UE_LOG(LogTemp, Error, TEXT("Player is lagging far behind"));
			break;
		}

		const TMap<FDebugID, FLockstepDebugItem>& SecondPlayerDebugItems = SecondPlayerDebugMaps[FirstPlayerDebugPairKey];

		if (FirstPlayerDebugItems.Num() != SecondPlayerDebugItems.Num())
		{
			bDiscrepancyFound = true;
			bDifferentCallsAmount = true;
		}

		for (auto& [DebugID, LockstepDebugItem] : FirstPlayerDebugItems)
		{
			const FLockstepDebugItem* DebugItem2 = SecondPlayerDebugItems.Find(DebugID);
			if (!DebugItem2 || LockstepDebugItem != *DebugItem2)
			{
				bDiscrepancyFound = true;
				InvalidDebugIDs.Add(DebugID);

				if (FirstUnsyncedDebugID.IsEmpty())
				{
					FirstUnsyncedDebugID = DebugID;
				}
			}
		}

		ProcessedTicks.Add(FirstPlayerDebugPairKey);

	}

	if (bDiscrepancyFound)
	{
		const FMassEntityManager& EntityManager = GetWorld()->GetSubsystem<UMassEntitySubsystem>()->GetMutableEntityManager();
		
		UE_LOG(LogTemp, Error, TEXT("Lockstep Debug: Data discrepancy has been found!"));
		if (bDifferentCallsAmount)
		{
			UE_LOG(LogTemp, Error, TEXT("Different amount of debug calls on players."));
		}

		FString InvalidDebugIDsString;
		for (const FDebugID& DebugID : InvalidDebugIDs)
		{
			InvalidDebugIDsString += DebugID + ", ";
			DrawEntityDebug(EntityManager, DebugID);
		}
		UE_LOG(LogTemp, Error, TEXT("Inconsistent DebugIDs: %s"), *InvalidDebugIDsString);

		DetectedPlayersDataUnsyncDelegate.ExecuteIfBound(InvalidDebugIDs);
	}

	if (!bStopOnFirstUnsyncDetection)
	{
		CachedDebugData.Empty();
	}
	
	for (auto& ProcessedTick : ProcessedTicks)
	{
		FirstPlayerDebugMaps.Remove(ProcessedTick);
		SecondPlayerDebugMaps.Remove(ProcessedTick);
	}

	bUnsyncDetected = true;
}

void ULockstepDebugSubsystem::DrawEntityDebug(const FMassEntityManager& EntityManager, const UE::Lockstep::FDebugID& DebugID)
{
	const FMassEntityHandle Entity          = ExtractEntityHandleFromDebugID(DebugID);
	if (!EntityManager.IsEntityActive(Entity)) return;
	
	const FTransform& Transform             = EntityManager.GetFragmentDataChecked<FTransformFragment>(Entity).GetTransform();
	const FVector EntityLocation            = Transform.GetLocation();
	DrawDebugLine(GetWorld(), EntityLocation, EntityLocation + FVector::UpVector * 500.f, FColor::Red, 0, .25f, 0, 2.f);
}

FMassEntityHandle ULockstepDebugSubsystem::ExtractEntityHandleFromDebugID(const UE::Lockstep::FDebugID& DebugID)
{
	// We consider that DebugID is formated like this: "{ActionName}:{EntityIndex}:{EntitySerialNumber}"

	FString EntityAsString = DebugID.RightChop(DebugID.Find(":") + 1);
	FString Index          = EntityAsString.LeftChop(EntityAsString.Find(":") - 1);
	FString SerialNumber   = EntityAsString.RightChop(EntityAsString.Find(":") + 1);
	return FMassEntityHandle{FCString::Atoi(*Index), FCString::Atoi(*SerialNumber)};
}

#pragma optimize ("", on)
