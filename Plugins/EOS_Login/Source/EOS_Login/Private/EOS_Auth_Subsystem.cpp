// Fill out your copyright notice in the Description page of Project Settings.


#include "EOS_Auth_Subsystem.h"
#include "OnlineSubsystemEos.h"
#include "VoiceChat.h"


UEOS_Auth_Subsystem::UEOS_Auth_Subsystem()
{
    Subsystem = Online::GetSubsystem(GetWorld());
    OnLoginCompleteDelegate = FOnLoginCompleteDelegate::CreateUObject(this, &ThisClass::HandleLoginComplete);
}

void UEOS_Auth_Subsystem::Login()
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

void UEOS_Auth_Subsystem::HandleLoginComplete(int32 LocalUserNum, bool bWasSuccessful, const FUniqueNetId& UserId, const FString& Error)
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

        VoiceChatSetup(UserId);
    }
    else 
    {
       
        GEngine->AddOnScreenDebugMessage(-1, 4.0f, FColor::Green, FString("Logging into EOS Failed"));
    }

    Identity->ClearOnLoginCompleteDelegate_Handle(LocalUserNum, LoginDelegateHandle);
    LoginDelegateHandle.Reset();
}

void UEOS_Auth_Subsystem::VoiceChatSetup(const FUniqueNetId& UserId)
{
    if (Subsystem && Subsystem->GetSubsystemName() == TEXT("EOS"))
    {
        FOnlineSubsystemEOS* EOSSubsystem = static_cast<FOnlineSubsystemEOS*>(Subsystem);
        if (EOSSubsystem)
        {
            VoiceChatUser = EOSSubsystem->GetVoiceChatUserInterface(UserId);
            if (VoiceChatUser != nullptr && VoiceChatUser->IsLoggedIn())
            {
                VoiceChatUser->GetChannels();
            }
        }
    }
}

void UEOS_Auth_Subsystem::BeginDestroy()
{
    UE_LOG(LogTemp, Warning, TEXT("Auth subsystem being destroyed!"));
    Super::BeginDestroy();
}
