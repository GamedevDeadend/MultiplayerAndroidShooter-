// Fill out your copyright notice in the Description page of Project Settings.


#include "EOS_Auth_Subsystem.h"
#include "IOnlineSubsystemEOS.h"
#include "VoiceChat.h"
#include "OnlineSubsystemTypes.h"
#include "Interfaces/OnlineIdentityInterface.h"
#include "OnlineSubsystem.h"


UEOS_VoiceAuth_Subsystem::UEOS_VoiceAuth_Subsystem()
{
    Subsystem = Online::GetSubsystem(GetWorld());
    OnLoginCompleteDelegate = FOnLoginCompleteDelegate::CreateUObject(this, &ThisClass::HandleLoginComplete);
    OnVoiceChatIntializationDelegate = FOnVoiceChatInitializeCompleteDelegate::CreateUObject(this, &ThisClass::OnVoiceInitalization);
    OnVoiceChatConnectDelegate = FOnVoiceChatConnectCompleteDelegate::CreateUObject(this, &ThisClass::OnVoiceConnection);
    OnVoiceChatLoginDelegate = FOnVoiceChatLoginCompleteDelegate::CreateUObject(this, &ThisClass::OnVoiceChatUserLogin);
	OnVoiceChatChannelJoinDelegate = FOnVoiceChatChannelJoinCompleteDelegate::CreateUObject(this, &ThisClass::OnVoiceChannelJoined);
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

	NetId = Identity->GetUniquePlayerId(0);

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
        SetVoiceChatUserInterface(UserId);
        GEngine->AddOnScreenDebugMessage(-1, 4.0f, FColor::Green, FString::Printf(TEXT("%s Logging into EOS Successful"), *PlayerName));
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
        IOnlineSubsystemEOS* EOSSubsystem = static_cast<IOnlineSubsystemEOS*>(Subsystem);
        if (EOSSubsystem)
        {
            VoiceChatUser = EOSSubsystem->GetVoiceChatUserInterface(UserId);
            PlayerId = VoiceChatUser->GetLoggedInPlayerName();
			CurrentChannel = VoiceChatUser->GetTransmitChannel();
        }
    }
 }

FString UEOS_VoiceAuth_Subsystem::GetPlayerId()
{
    return PlayerId.IsEmpty() ? VoiceChatUser->GetLoggedInPlayerName() : PlayerId;
}

FString UEOS_VoiceAuth_Subsystem::GetCurrentChannel() const
{

    for(auto Channel : VoiceChatUser->GetChannels())
    {
        return Channel;
	}

    return CurrentChannel;
}

/*############################## - Trusted Servers Voice Setup - ############################## */

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

void UEOS_VoiceAuth_Subsystem::JoinVoiceChannel(const FString& NewChannel)
{
    if (VoiceChatUser != nullptr && VoiceChatUser->IsLoggedIn())
    {
        if (GEngine != nullptr)
        {
            GEngine->AddOnScreenDebugMessage(-1, 4.0f, FColor::Green, FString::Printf(TEXT("Joining Channel %s"), *NewChannel));
        }

        FString Token = VoiceChatUser->InsecureGetJoinToken(NewChannel, EVoiceChatChannelType::NonPositional);
        //VoiceChatUser->JoinChannel(TEXT("Team_1"), Token, EVoiceChatChannelType::NonPositional, OnVoiceChatChannelJoinDelegate);
        //VoiceSetup();
        //VoiceChatUser->Connect();
        VoiceChatUser->JoinChannel(NewChannel,Token, EVoiceChatChannelType::NonPositional, OnVoiceChatChannelJoinDelegate);
    }
    else
    {
        if (GEngine != nullptr)
        {
            GEngine->AddOnScreenDebugMessage(-1, 4.0f, FColor::Green, FString("Voice Chat User is not logged in or null."));
        }
	}
}

void UEOS_VoiceAuth_Subsystem::VoiceSetup()
{
    if (GEngine == nullptr) return;

     if (VoiceChat != nullptr)
     {
         VoiceChat->Initialize(OnVoiceChatIntializationDelegate);
     }
}

void UEOS_VoiceAuth_Subsystem::BeginDestroy()
{
    UE_LOG(LogTemp, Warning, TEXT("Auth subsystem being destroyed!"));
    Super::BeginDestroy();
}

void UEOS_VoiceAuth_Subsystem::TestingUpdate(const FString& ChannelName, const FString& CurrPlayerName , bool bIsTalking)
{
    if (GEngine != nullptr)
    {
        GEngine->AddOnScreenDebugMessage(-1, 4.0f, FColor::Green, FString::Printf(TEXT("%s Logging into EOS Successful"), *PlayerName));
    }
}

void UEOS_VoiceAuth_Subsystem::OnVoiceInitalization(const FVoiceChatResult& ChatResult)
{
    if (VoiceChat != nullptr && ChatResult.IsSuccess())
    {
        if (GEngine != nullptr)
        {
            GEngine->AddOnScreenDebugMessage(-1, 4.0f, FColor::Green, FString::Printf(TEXT("Intialization Completed")));
        }

        if(VoiceChat->IsConnected())
        {
            if (GEngine != nullptr)
            {
                GEngine->AddOnScreenDebugMessage(-1, 4.0f, FColor::Green, FString::Printf(TEXT("Already Connected to Voice Server")));
            }
            return;
		}

        VoiceChat->Connect(OnVoiceChatConnectDelegate);
    }
    else
    {
        if (GEngine != nullptr)
        {
            GEngine->AddOnScreenDebugMessage(-1, 4.0f, FColor::Green, FString::Printf(TEXT("Intialization Failed")));
        }
    }
}

void UEOS_VoiceAuth_Subsystem::OnVoiceConnection(const FVoiceChatResult& ChatResult)
{
    if (VoiceChat != nullptr && ChatResult.IsSuccess())
    {
        if (GEngine != nullptr)
        {
            GEngine->AddOnScreenDebugMessage(-1, 4.0f, FColor::Green, FString::Printf(TEXT("Connected to %d Server"), ChatResult.ResultCode));
        }

    }
    else
    {
        if (GEngine != nullptr)
        {
            GEngine->AddOnScreenDebugMessage(-1, 4.0f, FColor::Green, FString::Printf(TEXT("Failedt to Connect to server")));
        }
    }
}

void UEOS_VoiceAuth_Subsystem::OnVoiceChatUserLogin(const FString& LoggedInPlayerName, const FVoiceChatResult& Result)
{
    if (Result.IsSuccess())
    {
        if (GEngine != nullptr)
        {
            GEngine->AddOnScreenDebugMessage(-1, 4.0f, FColor::Green, FString::Printf(TEXT("Player Is Logged In")));
        }
    }
    else
    {
        if (GEngine != nullptr)
        {
            GEngine->AddOnScreenDebugMessage(-1, 4.0f, FColor::Green, FString::Printf(TEXT("Player Is LoggedIn Failed")));
        }
    }
}

void UEOS_VoiceAuth_Subsystem::OnVoiceChannelJoined(const FString& ChannelName, const FVoiceChatResult& Result)
{
    if (Result.IsSuccess())
    {
        if (GEngine != nullptr)
        {
            GEngine->AddOnScreenDebugMessage(-1, 4.0f, FColor::Green, FString::Printf(TEXT("Player Joined Channel %s"), *ChannelName));
        }
    }
    else
    {
        if (GEngine != nullptr)
        {
            GEngine->AddOnScreenDebugMessage(
                -1, 4.0f, FColor::Red,
                FString::Printf(TEXT("Failed to Join Channel %s | Error: %s | Desc: %s"),
                    *ChannelName,
                    *Result.ErrorCode,
                    *Result.ErrorDesc)
            );
        }
	}
}
