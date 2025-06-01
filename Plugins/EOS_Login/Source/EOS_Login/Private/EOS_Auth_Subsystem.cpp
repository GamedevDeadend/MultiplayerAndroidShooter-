// Fill out your copyright notice in the Description page of Project Settings.


#include "EOS_Auth_Subsystem.h"
#include "OnlineSubsystemEos.h"
#include "VoiceChat.h"


UEOS_VoiceAuth_Subsystem::UEOS_VoiceAuth_Subsystem()
{
    Subsystem = Online::GetSubsystem(GetWorld());
    OnLoginCompleteDelegate = FOnLoginCompleteDelegate::CreateUObject(this, &ThisClass::HandleLoginComplete);
    OnVoiceChatIntialization = FOnVoiceChatInitializeCompleteDelegate::CreateUObject(this, &ThisClass::OnVoiceInitalization);
    OnVoiceChatConnect = FOnVoiceChatConnectCompleteDelegate::CreateUObject(this, &ThisClass::OnVoiceConnection);
}

void UEOS_VoiceAuth_Subsystem::VoiceSetup()
{
    if (GEngine == nullptr) return;

     VoiceChat = IVoiceChat::Get();

     if (VoiceChat != nullptr)
     {
         VoiceChat->Initialize(OnVoiceChatIntialization);
     }
}

void UEOS_VoiceAuth_Subsystem::Login()
{
    if (bIsLoginInProgress == true) return;
	if (GEngine == nullptr) return;

    bIsLoginInProgress = true;

	if (Subsystem == nullptr)
	{
		return;
	}

    GEngine->AddOnScreenDebugMessage(-1, 4.0f, FColor::Green, FString::Printf(TEXT("%s Fetched Online Subsystem for EOS login"), *Subsystem->GetSubsystemName().ToString() ));

	IOnlineIdentityPtr Identity = Subsystem->GetIdentityInterface();

	if (Identity == nullptr)
	{
		return;
	}


	GEngine->AddOnScreenDebugMessage(-1, 4.0f, FColor::Green, FString("Identity Interface Fetched For EOS Login"));

	FUniqueNetIdPtr NetId = Identity->GetUniquePlayerId(0);

	if (NetId != nullptr && Identity->GetLoginStatus(0) == ELoginStatus::LoggedIn)
	{
        PlayerName = Identity->GetPlayerNickname(0);
		GEngine->AddOnScreenDebugMessage(-1, 4.0f, FColor::Green, FString("Player Already Logged In"));
        GEngine->AddOnScreenDebugMessage(-1, 4.0f, FColor::Green, FString::Printf(TEXT("%s Logging into EOS Successful"), *PlayerName));
		return;
	}


    LoginDelegateHandle = Identity->AddOnLoginCompleteDelegate_Handle(0, OnLoginCompleteDelegate);


    GEngine->AddOnScreenDebugMessage(-1, 4.0f, FColor::Green, FString("HandleLogin Handle Binded"));


    FString AuthType;
    FParse::Value(FCommandLine::Get(), TEXT("AUTH_TYPE="), AuthType);

    if (!AuthType.IsEmpty()) 
    {

        GEngine->AddOnScreenDebugMessage(-1, 4.0f, FColor::Green, FString("Logging into EOS Editor"));

        if (!Identity->AutoLogin(0))
        {
            GEngine->AddOnScreenDebugMessage(-1, 4.0f, FColor::Green, FString("Logging into EOS Failed"));

            Identity->ClearOnLoginCompleteDelegate_Handle(0, LoginDelegateHandle);
            LoginDelegateHandle.Reset();
        }
    }
    else
    {
        FOnlineAccountCredentials Credentials("accountportal", "", "");

        GEngine->AddOnScreenDebugMessage(-1, 4.0f, FColor::Green, FString("Logging into EOS Dev Auth Too;"));

        if (!Identity->Login(0, Credentials))
        {
            GEngine->AddOnScreenDebugMessage(-1, 4.0f, FColor::Green, FString("Failed to Log in EOS"));
            Identity->ClearOnLoginCompleteDelegate_Handle(0, LoginDelegateHandle);
            LoginDelegateHandle.Reset();
        }
    }

}

void UEOS_VoiceAuth_Subsystem::HandleLoginComplete(int32 LocalUserNum, bool bWasSuccessful, const FUniqueNetId& UserId, const FString& Error)
{
    Subsystem = Online::GetSubsystem(GetWorld());
    IOnlineIdentityPtr Identity = Subsystem->GetIdentityInterface();


    bIsLoginInProgress = false;

    if (GEngine == nullptr) return;

    GEngine->AddOnScreenDebugMessage(-1, 4.0f, FColor::Green, FString::Printf(TEXT("%s Logging into EOS Callback Called"), *PlayerName));

    if (bWasSuccessful)
    {
    
        PlayerName = Identity->GetPlayerNickname(UserId);
        GEngine->AddOnScreenDebugMessage(-1, 4.0f, FColor::Green, FString::Printf(TEXT("%s Logging into EOS Successful"),*PlayerName));

        SetVoiceChatUserInterface(UserId);
    }
    else 
    {
       
        GEngine->AddOnScreenDebugMessage(-1, 4.0f, FColor::Green, FString("Logging into EOS Failed"));
    }

    Identity->ClearOnLoginCompleteDelegate_Handle(LocalUserNum, LoginDelegateHandle);
    LoginDelegateHandle.Reset();
}

void UEOS_VoiceAuth_Subsystem::SetVoiceChatUserInterface(const FUniqueNetId& UserId)
{
    if (Subsystem && Subsystem->GetSubsystemName() == TEXT("EOS"))
    {
        FOnlineSubsystemEOS* EOSSubsystem = static_cast<FOnlineSubsystemEOS*>(Subsystem);
        if (EOSSubsystem)
        {
            VoiceChatUser = EOSSubsystem->GetVoiceChatUserInterface(UserId);
        }
    }
 }

void UEOS_VoiceAuth_Subsystem::OnVoiceConnection(const FVoiceChatResult& ChatResult)
{
    if (VoiceChat != nullptr)
    {
        if (GEngine != nullptr)
        {
            GEngine->AddOnScreenDebugMessage(-1, 4.0f, FColor::Green, FString::Printf(TEXT("Connection Completed")));
        }
        //VoiceChat->Connect(OnVoiceChatConnect);
    }
}

void UEOS_VoiceAuth_Subsystem::OnVoiceInitalization(const FVoiceChatResult& ChatResult)
{
    if (VoiceChat != nullptr && VoiceChat->IsInitialized())
    {
        if (GEngine != nullptr)
        {
            GEngine->AddOnScreenDebugMessage(-1, 4.0f, FColor::Green, FString::Printf(TEXT("Intialization Completed")));
        }
        VoiceChat->Connect(OnVoiceChatConnect);
    }
    else
    {
        if (GEngine != nullptr)
        {
            GEngine->AddOnScreenDebugMessage(-1, 4.0f, FColor::Green, FString::Printf(TEXT("Intialization Failed")));
        }
    }
}

void UEOS_VoiceAuth_Subsystem::BeginDestroy()
{
    UE_LOG(LogTemp, Warning, TEXT("Auth subsystem being destroyed!"));
    Super::BeginDestroy();
}

void UEOS_VoiceAuth_Subsystem::CheckChannels()
{
    if (VoiceChatUser != nullptr && VoiceChatUser->IsLoggedIn())
    {
        GEngine->AddOnScreenDebugMessage(-1, 4.0f, FColor::Green, FString::Printf(TEXT("Callling Check channel")));

        //VoiceChatUser->JoinChannel("Lobby", )

        for (auto Channel : VoiceChatUser->GetChannels())
        {
            if (GEngine != nullptr)
            {
                GEngine->AddOnScreenDebugMessage(-1, 4.0f, FColor::Green, FString::Printf(TEXT("%s Channel Logging into EOS Successful"), *Channel));
            }
        }
        if (GEngine != nullptr)
        {
            //GEngine->AddOnScreenDebugMessage(-1, 4.0f, FColor::Green, FString::Printf(TEXT("%s Channel Logging into EOS Successful"), *VoiceChatUser->GetChannels();
        }

        VoiceChatUser->OnVoiceChatPlayerTalkingUpdated().AddLambda
        (
            [](const FString& Channel, const FString& Player, bool bIsTalking)
            {
                if (GEngine != nullptr)
                {
                    GEngine->AddOnScreenDebugMessage(-1, 4.0f, FColor::Green, FString::Printf(TEXT("[%s] %s is %s"), *Channel, *Player, bIsTalking ? TEXT("talking") : TEXT("silent")));
                }
            }
        );

    }
}

void UEOS_VoiceAuth_Subsystem::TestingUpdate(const FString& ChannelName, const FString& CurrPlayerName , bool bIsTalking)
{
    if (GEngine != nullptr)
    {
        GEngine->AddOnScreenDebugMessage(-1, 4.0f, FColor::Green, FString::Printf(TEXT("%s Logging into EOS Successful"), *PlayerName));
    }
}
