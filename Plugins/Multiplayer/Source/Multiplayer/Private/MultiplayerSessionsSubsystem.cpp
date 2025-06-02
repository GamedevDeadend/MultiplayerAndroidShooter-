// Fill out your copyright notice in the Description page of Project Settings.

#include "MultiplayerSessionsSubsystem.h"
#include"OnlineSubSystem.h"
#include "OnlineSessionSettings.h"

UMultiplayerSessionsSubsystem::UMultiplayerSessionsSubsystem():

	CreateSessionCompleteDelegate(FOnCreateSessionCompleteDelegate::CreateUObject(this,&ThisClass::OnCreateSessionComplete)),
	FindSessionsCompleteDelegate(FOnFindSessionsCompleteDelegate::CreateUObject(this,&ThisClass::OnFindSessionsComplete)),
	JoinSessionCompleteDelegate(FOnJoinSessionCompleteDelegate::CreateUObject(this,&ThisClass::OnJoinSessionComplete)),
	DestroySessionCompleteDelegate(FOnDestroySessionCompleteDelegate::CreateUObject(this,&ThisClass::OnDestroySessionComplete)),
	StartSessionCompleteDelegate(FOnStartSessionCompleteDelegate::CreateUObject(this,&ThisClass::OnStartSessionComplete))
{
	IOnlineSubsystem *Subsystem = IOnlineSubsystem::Get();

	if (Subsystem)
	{
		SessionInterface = Subsystem->GetSessionInterface();
	}
}


//Session Functions

void UMultiplayerSessionsSubsystem::CreateSession(int32 NumPublicConnections, FString MatchType)
{

	if (!SessionInterface)
	{
		return;
	}

	auto ExistingSessions = SessionInterface->GetNamedSession(NAME_GameSession);
	if (ExistingSessions)
	{
		bCreateSessionOnDestroy = true;
		LastNumPublicConnections = NumPublicConnections;
		LastMatchType = MatchType;
		DestroySessions();
	}

	CreateSessionCompleteDelegateHandle = SessionInterface->AddOnCreateSessionCompleteDelegate_Handle(CreateSessionCompleteDelegate);

	LastSessionSettings = MakeShareable(new FOnlineSessionSettings());
	LastSessionSettings->bIsLANMatch = false;
	LastSessionSettings->NumPublicConnections = NumPublicConnections;
	LastSessionSettings->bAllowJoinInProgress = true;
	LastSessionSettings->bAllowJoinViaPresence = true;
	LastSessionSettings->bShouldAdvertise = true;
	LastSessionSettings->bUsesPresence = true;
	LastSessionSettings->Set(FName("MatchType"), MatchType, EOnlineDataAdvertisementType::ViaOnlineServiceAndPing);
	LastSessionSettings->BuildUniqueId = 1;
	LastSessionSettings->bUseLobbiesIfAvailable = false;
	LastSessionSettings->bUseLobbiesVoiceChatIfAvailable = true;

	const ULocalPlayer *LocalPlayer= GetWorld()->GetFirstLocalPlayerFromController();
	bool bIsCreated = SessionInterface->CreateSession(*LocalPlayer->GetPreferredUniqueNetId(), NAME_GameSession, *LastSessionSettings);
	if (bIsCreated)
	{
		GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Green, FString("Session is Created"));
		//GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Yellow, FString(printf("Session has been created 1007 SessionName: %s")));
	}
	if (bIsCreated == false)
	{
		GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Green, FString("Session is Not Created"));
		SessionInterface->ClearOnCreateSessionCompleteDelegate_Handle(CreateSessionCompleteDelegateHandle);
		MultiplayerOnCreateSessionDelegate.Broadcast(false);
	}
}

void UMultiplayerSessionsSubsystem::FindSessions(int32 MaxSearchResults)
{

	if (!SessionInterface.IsValid())
	{
		return;
	}

	FindSessionsCompleteDelegateHandle = SessionInterface->AddOnFindSessionsCompleteDelegate_Handle(FindSessionsCompleteDelegate);
	LastSessionSearch = MakeShareable(new FOnlineSessionSearch());
	LastSessionSearch->QuerySettings.SearchParams.Empty();

	if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Green, FString::Printf(TEXT("JoinButtonClicked1 %d sessions found"), LastSessionSearch->SearchResults.Num()));
	}

	LastSessionSearch->MaxSearchResults = MaxSearchResults;
	LastSessionSearch->bIsLanQuery = IOnlineSubsystem::Get()->GetSubsystemName() == "NULL" ? true : false;

	LastSessionSearch->QuerySettings.Set(SEARCH_PRESENCE, true, EOnlineComparisonOp::Equals);
#if P2PMODE
	LastSessionSearch->QuerySettings.Set(SEARCH_LOBBIES, true, EOnlineComparisonOp::Equals);
#endif

	if (GEngine != nullptr)
	{

#if P2PMODE
		GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Green, FString("Finding lobby"));
#else 
		GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Green, FString("Finding Sessions"));
#endif 

	}

	//Local Player To Get Net PLayer Id
	const ULocalPlayer* LocalPlayer = GetWorld()->GetFirstLocalPlayerFromController();
	if (!SessionInterface->FindSessions(*LocalPlayer->GetPreferredUniqueNetId(), LastSessionSearch.ToSharedRef()))
	{

#if P2PMODE
		GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Green, FString("Finding lobby Failed"));
#else 
		GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Green, FString("Finding Sessions Failed"));
#endif 

		SessionInterface->ClearOnFindSessionsCompleteDelegate_Handle(FindSessionsCompleteDelegateHandle);
		MultiplayerOnFindSessionDelegate.Broadcast(TArray<FOnlineSessionSearchResult>(), false);
	}
}

void UMultiplayerSessionsSubsystem::JoinSessions(const FOnlineSessionSearchResult& SearchResult)
{
	if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Green, FString("JoinButtonClicked5"));
	}

	if (!SessionInterface.IsValid())
	{
		MultiplayerOnJoinSessionDelegate.Broadcast(EOnJoinSessionCompleteResult::UnknownError);
		return;
	}

	//Adding join delegate to interface delegate list
	JoinSessionCompleteDelegateHandle = SessionInterface->AddOnJoinSessionCompleteDelegate_Handle(JoinSessionCompleteDelegate);

	if (GEngine != nullptr)
	{
#if P2PMODE
		GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Green, FString("Joining Lobby"));
#else 
		GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Green, FString("Joining Sessions"));
#endif 

	}

	//Local Player To Get Net PLayer Id
	const ULocalPlayer* LocalPlayer = GetWorld()->GetFirstLocalPlayerFromController();
	if (!SessionInterface->JoinSession(*LocalPlayer->GetPreferredUniqueNetId(), NAME_GameSession, SearchResult) )
	{
		SessionInterface->ClearOnJoinSessionCompleteDelegate_Handle(JoinSessionCompleteDelegateHandle);
		MultiplayerOnJoinSessionDelegate.Broadcast(EOnJoinSessionCompleteResult::UnknownError);
	}
}

void UMultiplayerSessionsSubsystem::DestroySessions()
{
	if (!SessionInterface.IsValid())
	{ 
		MultiplayerOnDestroySessionDelegate.Broadcast(false);
		return; 
	}

	DestroySessionCompleteDelegateHandle =  SessionInterface->AddOnDestroySessionCompleteDelegate_Handle(DestroySessionCompleteDelegate);

	if (!SessionInterface->DestroySession(NAME_GameSession))
	{
		SessionInterface->ClearOnDestroySessionCompleteDelegate_Handle(DestroySessionCompleteDelegateHandle);
		MultiplayerOnDestroySessionDelegate.Broadcast(false);
	}

}

void UMultiplayerSessionsSubsystem::StartSession()
{
	if (!SessionInterface.IsValid())
	{
		MultiplayerOnStartSessionDelegate.Broadcast(false);
		return;
	}
	
	StartSessionCompleteDelegateHandle = SessionInterface->AddOnStartSessionCompleteDelegate_Handle(StartSessionCompleteDelegate);
	bool bSessionStarted = SessionInterface->StartSession(NAME_GameSession);
	if (!bSessionStarted)
	{
		SessionInterface->ClearOnStartSessionCompleteDelegate_Handle(StartSessionCompleteDelegateHandle);
		MultiplayerOnStartSessionDelegate.Broadcast(false);
	}
}

/*
* Lobby Code
*/
// Tutorial 7: This code will only be included if P2PMode is enabled 

#if P2PMODE

void UMultiplayerSessionsSubsystem::CreateLobby(FName KeyName, FString KeyValue)
{
	if (GEngine == nullptr)
	{
		return;
	}

	IOnlineSubsystem* Subsystem = IOnlineSubsystem::Get();
	IOnlineSessionPtr Session = Subsystem->GetSessionInterface();

	CreateLobbyDelegateHandle =
		Session->AddOnCreateSessionCompleteDelegate_Handle(FOnCreateSessionCompleteDelegate::CreateUObject(
			this,
			&ThisClass::HandleCreateLobbyCompleted));

	TSharedRef<FOnlineSessionSettings> SessionSettings = MakeShared<FOnlineSessionSettings>();
	SessionSettings->NumPublicConnections = 2; //We will test our sessions with 2 players to keep things simple
	SessionSettings->bShouldAdvertise = true; //This creates a public match and will be searchable.
	SessionSettings->bUsesPresence = false;   //No presence on dedicated server. This requires a local user.
	SessionSettings->bAllowJoinViaPresence = false;
	SessionSettings->bAllowJoinViaPresenceFriendsOnly = false;
	SessionSettings->bAllowInvites = false;    //Allow inviting players into session. This requires presence and a local user. 
	SessionSettings->bAllowJoinInProgress = false; //Once the session is started, no one can join.
	SessionSettings->bIsDedicated = false; //Session created on dedicated server.
	SessionSettings->bUseLobbiesIfAvailable = true; //For P2P we will use a lobby instead of a session
	SessionSettings->bUseLobbiesVoiceChatIfAvailable = true; //We will also enable voice
	SessionSettings->bUsesStats = true; //Needed to keep track of player stats.
	SessionSettings->Set(FName("MatchType"), FString("FreeForAll") , EOnlineDataAdvertisementType::ViaOnlineService);
	SessionSettings->Settings.Add(KeyName, FOnlineSessionSetting((KeyValue), EOnlineDataAdvertisementType::ViaOnlineService));

	GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Green, FString("Creating Lobby"));

	UE_LOG(LogTemp, Log, TEXT("Creating Lobby..."));

	if (!Session->CreateSession(0, LobbyName, *SessionSettings))
	{
		GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Green, FString("Failed To Create Lobby"));

		UE_LOG(LogTemp, Warning, TEXT("Failed to create Lobby!"));
	}
}

void UMultiplayerSessionsSubsystem::HandleCreateLobbyCompleted(FName EOSLobbyName, bool bWasSuccessful)
{
	// Tutorial 7: Callback function: This is called once our lobby is created

	IOnlineSubsystem* Subsystem = IOnlineSubsystem::Get();
	IOnlineSessionPtr Session = Subsystem->GetSessionInterface();
	if (bWasSuccessful)
	{
		UE_LOG(LogTemp, Log, TEXT("Lobby: %s Created!"), *EOSLobbyName.ToString());
		//FString Map = "Game/Content/ThirdPerson/Maps/ThirdPersonMap?listen"; //Hardcoding map name here, should be passed by parameter
		//FURL TravelURL;
		//TravelURL.Map = Map;
		//GetWorld()->Listen(TravelURL);
		MultiplayerOnCreateSessionDelegate.Broadcast(true);
		SetupNotifications(); // Setup our listeners for lobby notification events 
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("Failed to create lobby!"));
	}

	// Clear our handle and reset the delegate. 
	Session->ClearOnCreateSessionCompleteDelegate_Handle(CreateLobbyDelegateHandle);
	CreateLobbyDelegateHandle.Reset();
}

void UMultiplayerSessionsSubsystem::SetupNotifications()
{
	// Tutorial 7: EOS Lobbies are great as there are notifications sent for our backend when there are changes to lobbies (ex: Participant Joins/Leaves, lobby or lobby member data is updated, etc...) 
	IOnlineSubsystem* Subsystem = IOnlineSubsystem::Get();
	IOnlineSessionPtr Session = Subsystem->GetSessionInterface();

	// In this tutorial we're only giving an example of a notification for when a participant joins/leaves the lobby. The approach is similar for other notifications. 
	Session->AddOnSessionParticipantsChangeDelegate_Handle(FOnSessionParticipantsChangeDelegate::CreateUObject(
		this,
		&ThisClass::HandleParticipantChanged));
}

void UMultiplayerSessionsSubsystem::HandleParticipantChanged(FName EOSLobbyName, const FUniqueNetId& NetId, bool bJoined)
{
	// Tutorial 7: Callback function called when participants join/leave. 
	if (bJoined)
	{
		UE_LOG(LogTemp, Log, TEXT("A player has joined Lobby: %s"), *LobbyName.ToString());
	}
	else
	{
		UE_LOG(LogTemp, Log, TEXT("A player has left Lobby: %s"), *LobbyName.ToString());
	}
}
#endif

//Delegates CallBack Functions

void UMultiplayerSessionsSubsystem::OnCreateSessionComplete(FName SessionName, bool bWasSuccessful)
{
	if (SessionInterface)
	{
		SessionInterface->ClearOnCreateSessionCompleteDelegate_Handle(CreateSessionCompleteDelegateHandle);
		StartSession();
	}

	MultiplayerOnCreateSessionDelegate.Broadcast(true);
}

void UMultiplayerSessionsSubsystem::OnFindSessionsComplete(bool bWasSuccessful)
{
	if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Green, FString::Printf(TEXT( "Find Search Results %d"), LastSessionSearch->SearchResults.Num() ));
	}

	if (SessionInterface.IsValid())
	{
		SessionInterface->ClearOnFindSessionsCompleteDelegate_Handle(FindSessionsCompleteDelegateHandle);
	}

	if (LastSessionSearch->SearchResults.Num() <= 0)
	{
		MultiplayerOnFindSessionDelegate.Broadcast(TArray<FOnlineSessionSearchResult>(), false);
		return;
	}

	MultiplayerOnFindSessionDelegate.Broadcast(LastSessionSearch->SearchResults, bWasSuccessful);
}

void UMultiplayerSessionsSubsystem::OnJoinSessionComplete(FName SessionName, EOnJoinSessionCompleteResult::Type Result)
{
	if (GEngine)
	{
		//GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Green, FString("JoinButtonClicked6"));
	}

	if (SessionInterface)
	{
		SessionInterface->ClearOnJoinSessionCompleteDelegate_Handle(JoinSessionCompleteDelegateHandle);
	}

	MultiplayerOnJoinSessionDelegate.Broadcast(Result);
}

void UMultiplayerSessionsSubsystem::OnDestroySessionComplete(FName SessionName, bool bWasSuccessful)
{
	if (SessionInterface)
	{
		SessionInterface->ClearOnDestroySessionCompleteDelegate_Handle(DestroySessionCompleteDelegateHandle);
	}

	if (bWasSuccessful && bCreateSessionOnDestroy)
	{
		bCreateSessionOnDestroy = false;
		CreateSession(LastNumPublicConnections, LastMatchType);
	}

	MultiplayerOnDestroySessionDelegate.Broadcast(true);
}

void UMultiplayerSessionsSubsystem::OnStartSessionComplete(FName SessionName, bool bWasSuccessful)
{
	if (SessionInterface)
	{
		SessionInterface->ClearOnStartSessionCompleteDelegate_Handle(StartSessionCompleteDelegateHandle);
	}

	if (bWasSuccessful)
	{
		MultiplayerOnStartSessionDelegate.Broadcast(true);
	}
}
