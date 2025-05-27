// Fill out your copyright notice in the Description page of Project Settings.


#include "Multiplayer_GI.h"
#include "Net/UnrealNetwork.h"

void UMultiplayer_GI::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	//DOREPLIFETIME(UMultiplayer_GI, CurrentGameModeType);
}
