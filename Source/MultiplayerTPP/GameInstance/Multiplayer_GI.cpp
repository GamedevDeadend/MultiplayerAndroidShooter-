// Fill out your copyright notice in the Description page of Project Settings.


#include "Multiplayer_GI.h"
#include "Net/UnrealNetwork.h"
#include "EOS_Auth_Subsystem.h"

void UMultiplayer_GI::Init()
{
	Super::Init();

	UEOS_Auth_Subsystem* Subsystem = GetSubsystem<UEOS_Auth_Subsystem>();
	if (Subsystem != nullptr)
	{
		Subsystem->Login();
	}
}

void UMultiplayer_GI::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	//DOREPLIFETIME(UMultiplayer_GI, CurrentGameModeType);
}

