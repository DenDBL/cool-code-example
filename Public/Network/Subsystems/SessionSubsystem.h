// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Interfaces/OnlineSessionInterface.h"	
#include "ESNetwork/Public/Network/Structures/NetworkStructures.h"
#include "OnlineSubsystemUtils/Classes/FindSessionsCallbackProxy.h"
#include "SessionSubsystem.generated.h"

class FOnlineSessionSettings;
class APlayerController;
class AGameModeBase;

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnAllPlayersConnected);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FESOnCreateSessionComplete, bool, Successful);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FESOnUpdateSessionComplete, bool, Successful);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FESOnStartSessionComplete, bool, Successful);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FESOnEndSessionComplete, bool, Successful);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FESOnDestroySessionComplete, bool, Successful);
DECLARE_MULTICAST_DELEGATE_TwoParams(FESOnFindSessionsComplete, const TArray<FOnlineSessionSearchResult>& SessionResults, bool Successful);
DECLARE_MULTICAST_DELEGATE_OneParam(FESOnJoinSessionComplete, EOnJoinSessionCompleteResult::Type Result);


USTRUCT(BlueprintType)
struct FSimSessionParameters
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Sesssion Parameters")
	FString ScenarioName = FString("");

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Sesssion Parameters")
	EOnlineScenarioType ScenarioType = EOnlineScenarioType::SimpleScenario;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Sesssion Parameters")
	int32 TotalPlannedUnits = 0;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Sesssion Parameters")
	int32 CountSectors = 0;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Sesssion Parameters")
	FString ScenarioVersion = FString("");

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Sesssion Parameters")
	FString ScenarioCreatorUserName = FString("");
};


/**
 * 
 */
UCLASS()
class ESNETWORK_API USessionSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()
	
public:
	USessionSubsystem();

	//Session creation return handle!!!!
	UFUNCTION(BlueprintCallable)
	void CreateSession(
		int32 NumPublicConnections, bool IsLANMatch, //Standart Params
		FSimSessionParameters SimSessionParameters); // Event sim Params

	//Session settings update
	void UpdateSession(FOnlineSessionSettings* Settings);
	//Mark session as started
	void StartSession();
	//Mark session as ended
	void EndSession();
	//Destroy or leave session
	UFUNCTION(BlueprintCallable)
	void DestroySession();
	//Find Available sessions
	TArray<FOnlineSessionSearchResult> FindSessions(int32 MaxSearchResults, bool IsLANQuery);
	//Join Game session
	void JoinGameSession(const FOnlineSessionSearchResult& SessionResult);

	//Open Level with ?Listen parameter
	UFUNCTION(BlueprintCallable)
	void OpenLevelAsListen(FName LevelName);


	//Get / Set Sesion standart parametrs
	UFUNCTION(BlueprintPure)
	int32 GetCurrentPlayersInSession();

	UFUNCTION(BlueprintPure)
	int32 GetMaxPlayersInSession();
	//End Get / Set Sesion standart parametrs


	//Get / Set Session custom parametrs
	UFUNCTION(BlueprintPure)
	FString GetOnlineScenarioStringParam(const FBlueprintSessionResult& SessionHandle, const FName ParamName);

	UFUNCTION(BlueprintPure)
	FString GetOnlineScenarioStringParamInSession(const FName ParamName);

	UFUNCTION(BlueprintCallable)
	void SetOnlineScenarioStringParam(const FName ParamName, FString NewValue);

	UFUNCTION(BlueprintPure)
	int32 GetOnlineScenarioIntParam(const FBlueprintSessionResult& SessionHandle, const FName ParamName);

	UFUNCTION(BlueprintPure)
	int32 GetOnlineScenarioIntParamInSession(const FName ParamName);

	UFUNCTION(BlueprintCallable)
	void SetOnlineScenarioIntParam(const FName ParamName, int32 NewValue);

	UFUNCTION(BlueprintPure)
	bool GetOnlineScenarioBoolParam(const FBlueprintSessionResult& SessionHandle, const FName ParamName);

	UFUNCTION(BlueprintPure)
	bool GetOnlineScenarioBoolParamInSession(const FName ParamName);

	UFUNCTION(BlueprintCallable)
	void SetOnlineScenarioBoolParam(const FName ParamName, bool NewValue);
	//End Get / Set Session custom parametrs


	//Delegates
	FESOnCreateSessionComplete OnCreateSessionCompleteEvent;
	FESOnUpdateSessionComplete OnUpdateSessionCompleteEvent;
	FESOnStartSessionComplete OnStartSessionCompleteEvent;
	FESOnEndSessionComplete OnEndSessionCompleteEvent;
	FESOnDestroySessionComplete OnDestroySessionCompleteEvent;
	FESOnFindSessionsComplete OnFindSessionsCompleteEvent;
	FESOnJoinSessionComplete OnJoinGameSessionCompleteEvent;

	UPROPERTY(BlueprintAssignable)
	FOnAllPlayersConnected OnAllPlayersConnectedEvent;

	//Client travel to session URL(level)
	bool TryTravelToCurrentSession();

	UFUNCTION(BlueprintCallable)
	void ServerTravel(const FString& LevelName, const FString& Options, bool bAbsolute = false);

	//Save current number of players connected to session (server only)
	UFUNCTION()
	void CacheNumOfConnectedPlayers();

	UFUNCTION(BlueprintPure)
	bool IsServer() const;

	UFUNCTION(BlueprintPure)
	bool IsClient() const;

	UFUNCTION(BlueprintPure)
	bool IsSinglePlayer() const;

	UFUNCTION(BlueprintPure)
	bool NowInSession();

	UFUNCTION(BlueprintPure)
	bool IsUniqueNetIdEqual(FUniqueNetIdRepl First, FUniqueNetIdRepl Second) const;

protected:

	void OnCreateSessionCompleted(FName SessionName, bool Successful);
	void OnUpdateSessionCompleted(FName SessionName, bool Successful);
	void OnStartSessionCompleted(FName SessionName, bool Successful);
	void OnEndSessionCompleted(FName SessionName, bool Successful);
	void OnDestroySessionCompleted(FName SessionName, bool Successful);
	void OnFindSessionsCompleted(bool Successful);
	void OnJoinSessionCompleted(FName SessionName, EOnJoinSessionCompleteResult::Type Result);
	void OnPlayerConnected(AGameModeBase* GameMode, APlayerController* PlayerController);

	void CheckAndBroadcastIfAllPlayersConnected();

private:

	//Get / Set Session custom parametrs Internal
	FOnlineSessionSettings* GetSessionSettingsForHost() const;

	FString GetOnlineScenarioStringParamInternal(const FOnlineSessionSettings& SessionSettings, const FName ParamName) const;
	int32 GetOnlineScenarioIntParamInternal(const FOnlineSessionSettings& SessionSettings, const FName ParamName) const;
	bool GetOnlineScenarioBoolParamInternal(const FOnlineSessionSettings& SessionSettings, const FName ParamName) const;
	//End Get / Set Session custom parametrs Internal

protected:

	UPROPERTY(BlueprintReadOnly, meta=(AllowPrivateAccess="true"))
	int32 NumOfPrevConnectedPlayers;
	
private:

	FOnCreateSessionCompleteDelegate CreateSessionCompleteDelegate;
	FDelegateHandle CreateSessionCompleteDelegateHandle;
	TSharedPtr<FOnlineSessionSettings> LastSessionSettings;

	FOnUpdateSessionCompleteDelegate UpdateSessionCompleteDelegate;
	FDelegateHandle UpdateSessionCompleteDelegateHandle;

	FOnStartSessionCompleteDelegate StartSessionCompleteDelegate;
	FDelegateHandle StartSessionCompleteDelegateHandle;

	FOnEndSessionCompleteDelegate EndSessionCompleteDelegate;
	FDelegateHandle EndSessionCompleteDelegateHandle;

	FOnDestroySessionCompleteDelegate DestroySessionCompleteDelegate;
	FDelegateHandle DestroySessionCompleteDelegateHandle;

	FOnFindSessionsCompleteDelegate FindSessionsCompleteDelegate;
	FDelegateHandle FindSessionsCompleteDelegateHandle;
	TSharedPtr<FOnlineSessionSearch> LastSessionSearch;

	FOnJoinSessionCompleteDelegate JoinSessionCompleteDelegate;
	FDelegateHandle JoinSessionCompleteDelegateHandle;

public:

	//Host Info
	UPROPERTY(BlueprintReadWrite)
	FName CurrentCreatedSessionName;

	//Clients Info
	UPROPERTY(BlueprintReadWrite)
	FBlueprintSessionResult CurrentPlayerSession;

	UPROPERTY(BlueprintReadWrite)
	FString PlayerName;

	UPROPERTY(BlueprintReadWrite)
	EOnlinePlayerRole PlayerRole;

	UPROPERTY(BlueprintReadWrite)
	FPlayerSectorsContainer PlayerSectorsContainer;
};
