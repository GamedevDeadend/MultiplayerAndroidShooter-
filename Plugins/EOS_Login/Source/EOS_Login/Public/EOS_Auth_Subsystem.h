// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "OnlineSubsystem.h"
#include "OnlineSubsystemUtils.h"
#include "OnlineSubsystemTypes.h"
#include "Interfaces/OnlineIdentityInterface.h"
#include "EOS_Auth_Subsystem.generated.h"

/**
 * 
 */
UCLASS()
class EOS_LOGIN_API UEOS_Auth_Subsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public: 

	UEOS_Auth_Subsystem();

	void Login();
	virtual void BeginDestroy()override;

private:

	class IOnlineSubsystem* Subsystem = nullptr;

	FString PlayerName = "";
	class IVoiceChatUser* VoiceChatUser = nullptr;


	FOnLoginCompleteDelegate OnLoginCompleteDelegate;
	FDelegateHandle LoginDelegateHandle;
	bool bIsLoginInProgress = false;

	void HandleLoginComplete(int32 LocalUserNum, bool bWasSuccessful, const FUniqueNetId& UserId, const FString& Error);
	void VoiceChatSetup(const FUniqueNetId& UserId);

public:

	FORCEINLINE FString GetPlayerName()const { return PlayerName.IsEmpty() ?  "Player Not Logged In" : PlayerName; };
	FORCEINLINE IVoiceChatUser* GetLocalPlayerChatInterface()const { return VoiceChatUser; };

};
