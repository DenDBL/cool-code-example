// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "MassEntityTypes.h"
#include "Subsystems/WorldSubsystem.h"
#include "LockstepDebugSubsystem.generated.h"



namespace UE::Lockstep
{
	typedef uint8 FPlayerIndex;
	typedef FString FDebugID;
}

typedef TMap<UE::Lockstep::FPlayerIndex, TMap<UE::Lockstep::FDebugID, FLockstepDebugItem>> PlayerDebugDataMap;
typedef TMap<UE::Lockstep::FDebugID, FLockstepDebugItem> DebugDataMap;

// Returns unsynced DebugIDs (Entity index or some other useful info can be extracted from it)
DECLARE_DELEGATE_OneParam(FDetectedPlayersDataUnsyncSignature, const TArray<UE::Lockstep::FDebugID>&)


USTRUCT()
struct ESNETWORK_API FLockstepDebugItem
{
	GENERATED_BODY()

	UPROPERTY()
	FString Payload;
	
	bool operator == (const FLockstepDebugItem& Other) const
	{
		return (Payload == Other.Payload);
	}
};


UCLASS()
class ESNETWORK_API ULockstepDebugSubsystem : public UWorldSubsystem
{
	GENERATED_BODY()

public:

	// CONFIG START ------
	
	bool bStopOnFirstUnsyncDetection;

	// CONFIG END

	FCriticalSection DebugDataAdditionLock;
	bool bUnsyncDetected;

private:

	// When the debugger receives data from the next lockstep second, it compares all data from the previous second to find inconsistencies between players.
	int64 CurrentDebuggedLockstepSecond = 0;
	
	// Used to compare data from several players between each other.
	// Items with the same DebugID will be expected to have the same Payload. If they don't - we detect a data discrepancy.
	PlayerDebugDataMap CachedDebugData;

	UE::Lockstep::FDebugID FirstUnsyncedDebugID;

	FDetectedPlayersDataUnsyncSignature DetectedPlayersDataUnsyncDelegate;

	TMap<uint64, DebugDataMap> FirstPlayerDebugMaps;
	TMap<uint64, DebugDataMap> SecondPlayerDebugMaps;

public:

	ULockstepDebugSubsystem();
	
public:

	void AddDebugData(const int64 LockstepSecond, const UE::Lockstep::FPlayerIndex PlayerIndex, const UE::Lockstep::FDebugID DebugID, const FLockstepDebugItem DebugData);

private:

	void ProcessAndClearCachedDebugData();
	void DrawEntityDebug(const FMassEntityManager& EntityManager, const UE::Lockstep::FDebugID& DebugID);

	static FMassEntityHandle ExtractEntityHandleFromDebugID(const UE::Lockstep::FDebugID& DebugID);
};
