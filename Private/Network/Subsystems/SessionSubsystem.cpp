// Fill out your copyright notice in the Description page of Project Settings.


#include "Network/Subsystems/SessionSubsystem.h"
#include "OnlineSubsystemUtils.h"
#include "OnlineSessionSettings.h"
#include "Online/OnlineSessionNames.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/GameStateBase.h"
#include "GameFramework/GameModeBase.h"
#include "GameFramework/PLayerController.h"

#if ESNETWORK_WITH_EDITOR
#include "EventSimEditor/Public/Network/EditorNetworkBFL.h"
#endif

USessionSubsystem::USessionSubsystem()
	: CreateSessionCompleteDelegate(FOnCreateSessionCompleteDelegate::CreateUObject(this, &ThisClass::OnCreateSessionCompleted))
	, UpdateSessionCompleteDelegate(FOnUpdateSessionCompleteDelegate::CreateUObject(this, &ThisClass::OnUpdateSessionCompleted))
	, StartSessionCompleteDelegate(FOnStartSessionCompleteDelegate::CreateUObject(this, &ThisClass::OnStartSessionCompleted))
	, EndSessionCompleteDelegate(FOnEndSessionCompleteDelegate::CreateUObject(this, &ThisClass::OnEndSessionCompleted))
	, DestroySessionCompleteDelegate(FOnDestroySessionCompleteDelegate::CreateUObject(this, &ThisClass::OnDestroySessionCompleted))
	, FindSessionsCompleteDelegate(FOnFindSessionsCompleteDelegate::CreateUObject(this, &ThisClass::OnFindSessionsCompleted))
	, JoinSessionCompleteDelegate(FOnJoinSessionCompleteDelegate::CreateUObject(this, &ThisClass::OnJoinSessionCompleted))
{
	FGameModeEvents::GameModePostLoginEvent.AddUObject(this, &ThisClass::OnPlayerConnected);
	NumOfPrevConnectedPlayers = 0;

	CurrentCreatedSessionName = FName("");
	CurrentPlayerSession = FBlueprintSessionResult();
	PlayerName = "";
	PlayerRole = EOnlinePlayerRole::PoliceManipulator;
}

void USessionSubsystem::CreateSession(
	int32 NumPublicConnections, bool IsLANMatch, //Standart Params
	FSimSessionParameters SimSessionParameters) // Event sim Params
{
	const IOnlineSessionPtr sessionInterface = Online::GetSessionInterface(GetWorld());
	if (!sessionInterface.IsValid())
	{
		OnCreateSessionCompleteEvent.Broadcast(false);
		return;
	}

	LastSessionSettings = MakeShareable(new FOnlineSessionSettings());
	LastSessionSettings->Settings.Add(FName("OnlineScenarioName"), FOnlineSessionSetting(SimSessionParameters.ScenarioName, EOnlineDataAdvertisementType::ViaOnlineService));
	LastSessionSettings->Settings.Add(FName("OnlineScenarioType"), FOnlineSessionSetting(static_cast<int32>(SimSessionParameters.ScenarioType), EOnlineDataAdvertisementType::ViaOnlineService));
	LastSessionSettings->Settings.Add(FName("OnlineScenarioStarted"), FOnlineSessionSetting(false, EOnlineDataAdvertisementType::ViaOnlineService));
	LastSessionSettings->Settings.Add(FName("TotalPlannedUnits"), FOnlineSessionSetting(SimSessionParameters.TotalPlannedUnits, EOnlineDataAdvertisementType::ViaOnlineService));
	LastSessionSettings->Settings.Add(FName("CountSectors"), FOnlineSessionSetting(SimSessionParameters.CountSectors, EOnlineDataAdvertisementType::ViaOnlineService));
	LastSessionSettings->Settings.Add(FName("ScenarioVersion"), FOnlineSessionSetting(SimSessionParameters.ScenarioVersion, EOnlineDataAdvertisementType::ViaOnlineService));
	LastSessionSettings->Settings.Add(FName("ScenarioCreatorUserName"), FOnlineSessionSetting(SimSessionParameters.ScenarioCreatorUserName, EOnlineDataAdvertisementType::ViaOnlineService));

	LastSessionSettings->NumPrivateConnections = 0;
	LastSessionSettings->NumPublicConnections = NumPublicConnections;
	LastSessionSettings->bAllowInvites = true;
	LastSessionSettings->bAllowJoinInProgress = true;
	LastSessionSettings->bAllowJoinViaPresence = true;
	LastSessionSettings->bAllowJoinViaPresenceFriendsOnly = true;
	LastSessionSettings->bIsDedicated = false;
	LastSessionSettings->bUsesPresence = true;
	LastSessionSettings->bIsLANMatch = IsLANMatch;
	LastSessionSettings->bShouldAdvertise = true;

	CreateSessionCompleteDelegateHandle = sessionInterface->AddOnCreateSessionCompleteDelegate_Handle(CreateSessionCompleteDelegate);

	const ULocalPlayer* localPlayer = GetWorld()->GetFirstLocalPlayerFromController();
	if (!sessionInterface->CreateSession(*localPlayer->GetPreferredUniqueNetId(), NAME_GameSession, *LastSessionSettings))
	{
		sessionInterface->ClearOnCreateSessionCompleteDelegate_Handle(CreateSessionCompleteDelegateHandle);

		OnCreateSessionCompleteEvent.Broadcast(false);
	}

}

void USessionSubsystem::OnCreateSessionCompleted(FName SessionName, bool Successful)
{
	const IOnlineSessionPtr sessionInterface = Online::GetSessionInterface(GetWorld());
	if (sessionInterface)
	{
		sessionInterface->ClearOnCreateSessionCompleteDelegate_Handle(CreateSessionCompleteDelegateHandle);
	}

	CurrentCreatedSessionName = SessionName;

	GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Green, "SESSION CREATED");
	OnCreateSessionCompleteEvent.Broadcast(Successful);
}

void USessionSubsystem::UpdateSession(FOnlineSessionSettings* Settings)
{
	const IOnlineSessionPtr sessionInterface = Online::GetSessionInterface(GetWorld());
	if (!sessionInterface.IsValid())
	{
		OnUpdateSessionCompleteEvent.Broadcast(false);
		return;
	}

	TSharedPtr<FOnlineSessionSettings> updatedSessionSettings = MakeShareable(new FOnlineSessionSettings(*Settings));

	UpdateSessionCompleteDelegateHandle =
		sessionInterface->AddOnUpdateSessionCompleteDelegate_Handle(UpdateSessionCompleteDelegate);

	if (!sessionInterface->UpdateSession(NAME_GameSession, *updatedSessionSettings))
	{
		sessionInterface->ClearOnUpdateSessionCompleteDelegate_Handle(UpdateSessionCompleteDelegateHandle);

		OnUpdateSessionCompleteEvent.Broadcast(false);
	}
	else
	{
		LastSessionSettings = updatedSessionSettings;
	}
}

void USessionSubsystem::OnUpdateSessionCompleted(FName SessionName, bool Successful)
{
	const IOnlineSessionPtr sessionInterface = Online::GetSessionInterface(GetWorld());
	if (sessionInterface)
	{
		sessionInterface->ClearOnUpdateSessionCompleteDelegate_Handle(UpdateSessionCompleteDelegateHandle);
	}

	OnUpdateSessionCompleteEvent.Broadcast(Successful);
}

void USessionSubsystem::StartSession()
{
	const IOnlineSessionPtr sessionInterface = Online::GetSessionInterface(GetWorld());
	if (!sessionInterface.IsValid())
	{
		OnStartSessionCompleteEvent.Broadcast(false);
		return;
	}

	StartSessionCompleteDelegateHandle =
		sessionInterface->AddOnStartSessionCompleteDelegate_Handle(StartSessionCompleteDelegate);

	if (!sessionInterface->StartSession(NAME_GameSession))
	{
		sessionInterface->ClearOnStartSessionCompleteDelegate_Handle(StartSessionCompleteDelegateHandle);

		OnStartSessionCompleteEvent.Broadcast(false);
	}
}

void USessionSubsystem::OnStartSessionCompleted(FName SessionName, bool Successful)
{
	const IOnlineSessionPtr sessionInterface = Online::GetSessionInterface(GetWorld());
	if (sessionInterface)
	{
		sessionInterface->ClearOnStartSessionCompleteDelegate_Handle(StartSessionCompleteDelegateHandle);
	}

	GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Green, TEXT("SESSION STARTED"));
	OnStartSessionCompleteEvent.Broadcast(Successful);
}

void USessionSubsystem::EndSession()
{
	const IOnlineSessionPtr sessionInterface = Online::GetSessionInterface(GetWorld());
	if (!sessionInterface.IsValid())
	{
		OnEndSessionCompleteEvent.Broadcast(false);
		return;
	}

	EndSessionCompleteDelegateHandle =
		sessionInterface->AddOnEndSessionCompleteDelegate_Handle(EndSessionCompleteDelegate);

	if (!sessionInterface->EndSession(NAME_GameSession))
	{
		sessionInterface->ClearOnEndSessionCompleteDelegate_Handle(EndSessionCompleteDelegateHandle);

		OnEndSessionCompleteEvent.Broadcast(false);
	}
}

void USessionSubsystem::OnEndSessionCompleted(FName SessionName, bool Successful)
{
	const IOnlineSessionPtr sessionInterface = Online::GetSessionInterface(GetWorld());
	if (sessionInterface)
	{
		sessionInterface->ClearOnEndSessionCompleteDelegate_Handle(EndSessionCompleteDelegateHandle);
	}

	GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Green, TEXT("SESSION ENDED"));
	OnEndSessionCompleteEvent.Broadcast(Successful);
}

void USessionSubsystem::DestroySession()
{
	const IOnlineSessionPtr sessionInterface = Online::GetSessionInterface(GetWorld());
	if (!sessionInterface.IsValid())
	{
		OnDestroySessionCompleteEvent.Broadcast(false);
		return;
	}

	DestroySessionCompleteDelegateHandle =
		sessionInterface->AddOnDestroySessionCompleteDelegate_Handle(DestroySessionCompleteDelegate);

	if (!sessionInterface->DestroySession(NAME_GameSession))
	{
		sessionInterface->ClearOnDestroySessionCompleteDelegate_Handle(DestroySessionCompleteDelegateHandle);

		OnDestroySessionCompleteEvent.Broadcast(false);
	}
}

void USessionSubsystem::OnDestroySessionCompleted(FName SessionName, bool Successful)
{
	const IOnlineSessionPtr sessionInterface = Online::GetSessionInterface(GetWorld());
	if (sessionInterface)
	{
		sessionInterface->ClearOnDestroySessionCompleteDelegate_Handle(DestroySessionCompleteDelegateHandle);
	}

	GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Green, TEXT("SESSION DESTROYED"));
	OnDestroySessionCompleteEvent.Broadcast(Successful);
}

TArray<FOnlineSessionSearchResult> USessionSubsystem::FindSessions(int32 MaxSearchResults, bool IsLANQuery)
{
	const IOnlineSessionPtr sessionInterface = Online::GetSessionInterface(GetWorld());
	if (!sessionInterface.IsValid())
	{
		OnFindSessionsCompleteEvent.Broadcast(TArray<FOnlineSessionSearchResult>(), false);
		return TArray<FOnlineSessionSearchResult>();
	}

	FindSessionsCompleteDelegateHandle =
		sessionInterface->AddOnFindSessionsCompleteDelegate_Handle(FindSessionsCompleteDelegate);

	LastSessionSearch = MakeShareable(new FOnlineSessionSearch());
	LastSessionSearch->MaxSearchResults = MaxSearchResults;
	LastSessionSearch->bIsLanQuery = IsLANQuery;

	LastSessionSearch->QuerySettings.Set(SEARCH_PRESENCE, true, EOnlineComparisonOp::Equals);

	const ULocalPlayer* localPlayer = GetWorld()->GetFirstLocalPlayerFromController();
	if (!sessionInterface->FindSessions(*localPlayer->GetPreferredUniqueNetId(), LastSessionSearch.ToSharedRef()))
	{
		sessionInterface->ClearOnFindSessionsCompleteDelegate_Handle(FindSessionsCompleteDelegateHandle);

		OnFindSessionsCompleteEvent.Broadcast(TArray<FOnlineSessionSearchResult>(), false);
		return TArray<FOnlineSessionSearchResult>();
	}

	return LastSessionSearch->SearchResults;
}

void USessionSubsystem::OnFindSessionsCompleted(bool Successful)
{
	const IOnlineSessionPtr sessionInterface = Online::GetSessionInterface(GetWorld());
	if (sessionInterface)
	{
		sessionInterface->ClearOnFindSessionsCompleteDelegate_Handle(FindSessionsCompleteDelegateHandle);
	}

	if (LastSessionSearch->SearchResults.Num() <= 0)
	{
		OnFindSessionsCompleteEvent.Broadcast(TArray<FOnlineSessionSearchResult>(), Successful);
		return;
	}

	OnFindSessionsCompleteEvent.Broadcast(LastSessionSearch->SearchResults, Successful);
}

void USessionSubsystem::JoinGameSession(const FOnlineSessionSearchResult& SessionResult)
{
	const IOnlineSessionPtr sessionInterface = Online::GetSessionInterface(GetWorld());
	if (!sessionInterface.IsValid())
	{
		OnJoinGameSessionCompleteEvent.Broadcast(EOnJoinSessionCompleteResult::UnknownError);
		return;
	}

	JoinSessionCompleteDelegateHandle =
		sessionInterface->AddOnJoinSessionCompleteDelegate_Handle(JoinSessionCompleteDelegate);

	const ULocalPlayer* localPlayer = GetWorld()->GetFirstLocalPlayerFromController();
	if (!sessionInterface->JoinSession(*localPlayer->GetPreferredUniqueNetId(), NAME_GameSession, SessionResult))
	{
		sessionInterface->ClearOnJoinSessionCompleteDelegate_Handle(JoinSessionCompleteDelegateHandle);

		OnJoinGameSessionCompleteEvent.Broadcast(EOnJoinSessionCompleteResult::UnknownError);
	}
}

void USessionSubsystem::OpenLevelAsListen(FName LevelName)
{
	UGameplayStatics::OpenLevel(GetWorld(), LevelName, true, "?listen");
}

void USessionSubsystem::OnJoinSessionCompleted(FName SessionName, EOnJoinSessionCompleteResult::Type Result)
{
	const IOnlineSessionPtr sessionInterface = Online::GetSessionInterface(GetWorld());
	if (sessionInterface)
	{
		sessionInterface->ClearOnJoinSessionCompleteDelegate_Handle(JoinSessionCompleteDelegateHandle);
	}

	GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Green, TEXT("JOINED SESSION"));
	OnJoinGameSessionCompleteEvent.Broadcast(Result);
	TryTravelToCurrentSession();
}

void USessionSubsystem::OnPlayerConnected(AGameModeBase* GameMode,APlayerController* PlayerController)
{
	CheckAndBroadcastIfAllPlayersConnected();
}

void USessionSubsystem::CheckAndBroadcastIfAllPlayersConnected()
{

	if (!IsValid(GetWorld()) || !IsValid(GetWorld()->GetGameState())) {
		return;
	}

	AGameStateBase* GameStateBase = GetWorld()->GetGameState();
	if (NumOfPrevConnectedPlayers == GameStateBase->PlayerArray.Num() && NumOfPrevConnectedPlayers > 0) {
		GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Green, TEXT("ALL PLAYERS CONNECTED"));
		OnAllPlayersConnectedEvent.Broadcast();

		return;
	}
	
#if ESNETWORK_WITH_EDITOR
	int32 EditorPlayers = UEditorNetworkBFL::GetNumOfPlayersFromEditorSettings();
	if (EditorPlayers == GameStateBase->PlayerArray.Num() && EditorPlayers > 0 && !NowInSession())
	{
		GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Green, TEXT("Editor players count equals connected"));
		NumOfPrevConnectedPlayers = EditorPlayers;
		OnAllPlayersConnectedEvent.Broadcast();
	}
	
#endif
}

bool USessionSubsystem::TryTravelToCurrentSession()
{

	const IOnlineSessionPtr sessionInterface = Online::GetSessionInterface(GetWorld());
	if (!sessionInterface.IsValid())
	{
		return false;
	}

	FString connectString;
	if (!sessionInterface->GetResolvedConnectString(NAME_GameSession, connectString))
	{
		return false;
	}

	APlayerController* playerController = GetWorld()->GetFirstPlayerController();
	playerController->ClientTravel(connectString, TRAVEL_Absolute);
	return true;
}

void USessionSubsystem::ServerTravel(const FString& LevelName, const FString& Options, bool bAbsolute)
{
	CacheNumOfConnectedPlayers();
	//UGameplayStatics::GetPlayerController(GetWorld(), 0)->ConsoleCommand("ServerTravel "+ LevelName.TrimStartAndEnd()+Options.TrimStartAndEnd());
	GetWorld()->ServerTravel(LevelName, bAbsolute);
}

void USessionSubsystem::CacheNumOfConnectedPlayers()
{
	if (!IsValid(GetWorld()) || !IsValid(GetWorld()->GetGameState())) {
		NumOfPrevConnectedPlayers = 0;
		return;
	}

	AGameStateBase* GameStateBase = GetWorld()->GetGameState();
	NumOfPrevConnectedPlayers = GameStateBase->PlayerArray.Num();

}


//Get / Set Sesion standart parametrs
int32 USessionSubsystem::GetCurrentPlayersInSession()
{
	if (IsServer())
	{
		const IOnlineSessionPtr sessionInterface = Online::GetSessionInterface(GetWorld());
		if (!sessionInterface)
		{
			return 0;
		}

		auto FoundServerSession = sessionInterface->GetNamedSession(CurrentCreatedSessionName);
		if (!FoundServerSession)
		{
			return 0;
		}

		return FoundServerSession->SessionSettings.NumPublicConnections - FoundServerSession->NumOpenPublicConnections;
	}


	// For Clients
	if (CurrentPlayerSession.OnlineResult.IsValid())
	{
		return CurrentPlayerSession.OnlineResult.Session.SessionSettings.NumPublicConnections - CurrentPlayerSession.OnlineResult.Session.NumOpenPublicConnections;
	}

	return 0;
}

int32 USessionSubsystem::GetMaxPlayersInSession()
{
	if (IsServer())
	{
		auto SessionSettings = GetSessionSettingsForHost();
		if (!SessionSettings)
		{
			return 0;
		}

		return SessionSettings->NumPublicConnections;
	}


	// For Clients
	if (CurrentPlayerSession.OnlineResult.IsValid())
	{
		return CurrentPlayerSession.OnlineResult.Session.SessionSettings.NumPublicConnections;
	}

	return 0;
}
//End Get / Set Sesion standart parametrs


//Get / Set Session custom parametrs
FString USessionSubsystem::GetOnlineScenarioStringParam(const FBlueprintSessionResult& SessionHandle, const FName ParamName)
{
	if (SessionHandle.OnlineResult.IsValid())
	{
		return GetOnlineScenarioStringParamInternal(SessionHandle.OnlineResult.Session.SessionSettings, ParamName);
	}

	return FString("");
}

FString USessionSubsystem::GetOnlineScenarioStringParamInSession(const FName ParamName)
{
	if (IsServer())
	{
		auto SessionSettings = GetSessionSettingsForHost();
		if (!SessionSettings)
		{
			return FString("");
		}

		return GetOnlineScenarioStringParamInternal(*SessionSettings, ParamName);
	}


	// For Clients
	return GetOnlineScenarioStringParam(CurrentPlayerSession, ParamName);
}

void USessionSubsystem::SetOnlineScenarioStringParam(const FName ParamName, FString NewValue)
{
	auto SessionSettings = GetSessionSettingsForHost();
	if (!SessionSettings)
	{
		return;
	}

	if (FOnlineSessionSetting* ParamValue = SessionSettings->Settings.Find(ParamName))
	{
		ParamValue->Data.SetValue(NewValue);
	}
}

int32 USessionSubsystem::GetOnlineScenarioIntParam(const FBlueprintSessionResult& SessionHandle, const FName ParamName)
{
	if (SessionHandle.OnlineResult.IsValid())
	{
		return GetOnlineScenarioIntParamInternal(SessionHandle.OnlineResult.Session.SessionSettings, ParamName);
	}

	return 0;
}

int32 USessionSubsystem::GetOnlineScenarioIntParamInSession(const FName ParamName)
{
	if (IsServer())
	{
		auto SessionSettings = GetSessionSettingsForHost();
		if (!SessionSettings)
		{
			return 0;
		}

		return GetOnlineScenarioIntParamInternal(*SessionSettings, ParamName);
	}

	// For Clients
	return GetOnlineScenarioIntParam(CurrentPlayerSession, ParamName);

}

void USessionSubsystem::SetOnlineScenarioIntParam(const FName ParamName, int32 NewValue)
{
	auto SessionSettings = GetSessionSettingsForHost();
	if (!SessionSettings)
	{
		return;
	}

	if (FOnlineSessionSetting* ParamValue = SessionSettings->Settings.Find(ParamName))
	{
		ParamValue->Data.SetValue(NewValue);
	}
}

bool USessionSubsystem::GetOnlineScenarioBoolParam(const FBlueprintSessionResult& SessionHandle, const FName ParamName)
{
	if (SessionHandle.OnlineResult.IsValid())
	{
		return GetOnlineScenarioBoolParamInternal(SessionHandle.OnlineResult.Session.SessionSettings, ParamName);
	}

	return false;
}

bool USessionSubsystem::GetOnlineScenarioBoolParamInSession(const FName ParamName)
{
	if (IsServer())
	{
		auto SessionSettings = GetSessionSettingsForHost();
		if (!SessionSettings)
		{
			return false;
		}

		return GetOnlineScenarioBoolParamInternal(*SessionSettings, ParamName);
	}


	// For Clients
	return GetOnlineScenarioBoolParam(CurrentPlayerSession, ParamName);
}

void USessionSubsystem::SetOnlineScenarioBoolParam(const FName ParamName, bool NewValue)
{
	auto SessionSettings = GetSessionSettingsForHost();
	if (!SessionSettings)
	{
		return;
	}

	if (FOnlineSessionSetting* ParamValue = SessionSettings->Settings.Find(ParamName))
	{
		ParamValue->Data.SetValue(NewValue);
	}
}
//End Get / Set Session custom parametrs


//Get / Set Session custom parametrs Internal
FOnlineSessionSettings* USessionSubsystem::GetSessionSettingsForHost() const
{
	const IOnlineSessionPtr SessionInterface = Online::GetSessionInterface(GetWorld());
	if (!SessionInterface)
	{
		return nullptr;
	}

	return SessionInterface->GetSessionSettings(CurrentCreatedSessionName);
}

FString USessionSubsystem::GetOnlineScenarioStringParamInternal(const FOnlineSessionSettings& SessionSettings, const FName ParamName) const
{
	if (const FOnlineSessionSetting* ParamValue = SessionSettings.Settings.Find(ParamName))
	{
		FString Result;
		ParamValue->Data.GetValue(Result);
		return Result;
	}

	return FString("");
}

int32 USessionSubsystem::GetOnlineScenarioIntParamInternal(const FOnlineSessionSettings& SessionSettings, const FName ParamName) const
{
	if (const FOnlineSessionSetting* ParamValue = SessionSettings.Settings.Find(ParamName))
	{
		int32 Result;
		ParamValue->Data.GetValue(Result);
		return Result;
	}

	return 0;
}

bool USessionSubsystem::GetOnlineScenarioBoolParamInternal(const FOnlineSessionSettings& SessionSettings, const FName ParamName) const
{
	if (const FOnlineSessionSetting* ParamValue = SessionSettings.Settings.Find(ParamName))
	{
		bool Result;
		ParamValue->Data.GetValue(Result);
		return Result;
	}

	return false;
}
//End Get / Set Session custom parametrs Internal


bool USessionSubsystem::IsServer() const
{
	if (!GetWorld())
	{
		return false;
	}

	ENetMode CurrentNetMode = GetWorld()->GetNetMode();
	return CurrentNetMode == ENetMode::NM_ListenServer || CurrentNetMode == ENetMode::NM_DedicatedServer;
}

bool USessionSubsystem::IsClient() const
{
	if (!GetWorld())
	{
		return false;
	}

	ENetMode CurrentNetMode = GetWorld()->GetNetMode();
	return CurrentNetMode == ENetMode::NM_Client;
}

bool USessionSubsystem::IsSinglePlayer() const
{
	return !IsServer() && !IsClient();
}

bool USessionSubsystem::NowInSession()
{
	if(IsSinglePlayer())
	{
		return false;
	}
	
	//check for server
	const IOnlineSessionPtr sessionInterface = Online::GetSessionInterface(GetWorld());
	if (sessionInterface)
	{
		auto SessionSettings = sessionInterface->GetSessionSettings(CurrentCreatedSessionName);
		if (SessionSettings)
		{
			return true;
		}
	}

	//check for client
	if (CurrentPlayerSession.OnlineResult.IsValid())
	{
		return true;
	}

	return false;
}

bool USessionSubsystem::IsUniqueNetIdEqual(FUniqueNetIdRepl First, FUniqueNetIdRepl Second) const
{
	return First == Second;
}
