// Fill out your copyright notice in the Description page of Project Settings.


#include "Multiplayer_GI.h"
#include "Net/UnrealNetwork.h"
#include "EOS_Auth_Subsystem.h"


void UMultiplayer_GI::Init()
{
	Super::Init();

#if !WITH_EDITOR

	UEOS_VoiceAuth_Subsystem* EOS_Subsystem = GetSubsystem<UEOS_VoiceAuth_Subsystem>();
	if (EOS_Subsystem != nullptr)
	{
		EOS_Subsystem->Login();
	}

#endif

}

void UMultiplayer_GI::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	//DOREPLIFETIME(UMultiplayer_GI, CurrentGameModeType);
}

