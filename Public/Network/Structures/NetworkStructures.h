// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "NetworkStructures.generated.h"

class APlayerController;

UENUM(BlueprintType)
enum class EOnlineScenarioType : uint8
{
	SimpleScenario = 0	UMETA(DisplayName = "SimpleScenario"),
	AdvancedScenario	UMETA(DisplayName = "AdvancedScenario"),

	Max UMETA(Hidden)
};

UENUM(BlueprintType)
enum class EOnlinePlayerRole : uint8
{
	CrowdManipulator = 0	UMETA(DisplayName = "CrowdManipulator"),
	PoliceManipulator		UMETA(DisplayName = "PoliceManipulator"),

	Max UMETA(Hidden)
};

USTRUCT(BlueprintType, Blueprintable)
struct FRolesStruct : public FTableRowBase
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	FString Name = "";
};

USTRUCT(BlueprintType, Blueprintable)
struct FPlayerSectorStruct
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	int32 SectorID = 0;

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	FString SectorName = "";
};

USTRUCT(BlueprintType, Blueprintable)
struct FPlayerSectorsContainer
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	TArray<FPlayerSectorStruct> PlayerSectorsData;
};

USTRUCT(BlueprintType, Blueprintable)
struct FSimPlayerMinInfo 
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	FString Name = "";

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	APlayerController* PlayerController = nullptr;

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	FUniqueNetIdRepl UniqueNetID = FUniqueNetIdRepl();

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	FText LocationRole = FText::FromString("");

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	FText LocationSectors = FText::FromString("");

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	bool bPlayerReady = false;
};

