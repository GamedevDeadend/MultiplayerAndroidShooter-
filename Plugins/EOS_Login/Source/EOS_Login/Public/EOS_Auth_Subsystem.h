// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "OnlineSubsystem.h"
#include "OnlineSubsystemUtils.h"
#include "OnlineSubsystemTypes.h"
#include "Interfaces/OnlineIdentityInterface.h"
#include "VoiceChat.h"
#include "EOS_Auth_Subsystem.generated.h"

/**
 * 
 */
UCLASS()
class EOS_LOGIN_API UEOS_VoiceAuth_Subsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public: 

	UEOS_VoiceAuth_Subsystem();

	UFUNCTION(BlueprintCallable)
		void Login();
		

private:

		bool bIsLoginInProgress = false;

		FUniqueNetIdPtr NetId;
		FString PlayerName = "";
		FString PlayerId = "";
		FString CurrentChannel = "";

		class IOnlineSubsystem* Subsystem = nullptr;
		class IVoiceChatUser* VoiceChatUser = nullptr;



		FOnLoginCompleteDelegate OnLoginCompleteDelegate;
		FDelegateHandle LoginDelegateHandle;

		void HandleLoginComplete(int32 LocalUserNum, bool bWasSuccessful, const FUniqueNetId& UserId, const FString& Error);
		void SetVoiceChatUserInterface(const FUniqueNetId& UserId);

public:

	FORCEINLINE FString GetPlayerName()const { return PlayerName.IsEmpty() ?  "Player Not Logged In" : PlayerName; };
	FORCEINLINE IVoiceChatUser* GetLocalPlayerChatInterface()const { return VoiceChatUser; };
	FString GetPlayerId();
	FString GetCurrentChannel()const;


/*
* CURRENT CLASS ENDS HERE <--XXXXXXXXXXXXXXXX-->
*/



//This is Setup for trusted servers, not used for now, but can be used later for more advanced features

	IVoiceChat* VoiceChat = nullptr;

	UFUNCTION(BlueprintCallable)
	void JoinVoiceChannel(const FString& NewChannel);

	UFUNCTION(BlueprintCallable)
	void VoiceSetup();


	virtual void BeginDestroy()override;
	void TestingUpdate(const FString& ChannelName, const FString& CurrPlayerName, bool bIsTalking);

	void OnVoiceInitalization(const FVoiceChatResult& ChatResult);
	void OnVoiceConnection(const FVoiceChatResult& ChatResult);
	void OnVoiceChatUserLogin(const FString& LoggedInPlayerName, const FVoiceChatResult& Result);
	void OnVoiceChannelJoined(const FString& ChannelName, const FVoiceChatResult& Result);
	void CheckChannels();


	FOnVoiceChatInitializeCompleteDelegate OnVoiceChatIntializationDelegate;
	FOnVoiceChatConnectCompleteDelegate OnVoiceChatConnectDelegate;
	FOnVoiceChatLoginCompleteDelegate OnVoiceChatLoginDelegate;
	FOnVoiceChatChannelJoinCompleteDelegate OnVoiceChatChannelJoinDelegate;

	//-XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX->

};
