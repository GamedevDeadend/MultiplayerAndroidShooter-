// Fill out your copyright notice in the Description page of Project Settings.


#include "MPPlayerState.h"
#include "MultiplayerTPP/Character/MPPlayer.h"
#include "MultiplayerTPP/Controllers/MPPlayerController.h"
#include "Net/UnrealNetwork.h"


void AMPPlayerState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AMPPlayerState, DefeatsAmt);
}

void AMPPlayerState::OnRep_Score()
{
	Super::OnRep_Score();

	Character = Character == nullptr ? Cast<AMPPlayer>(GetPawn()) : Character;
	if (Character && Character->Controller)
	{
		Controller == nullptr ? Controller = Cast<AMPPlayerController>(Character->Controller) : Controller;
		
		if (Controller)
		{
			Controller->SetHUDScore(GetScore());
		}
	}
}

void AMPPlayerState::OnRep_Defeats()
{
	Character = Character == nullptr ? Cast<AMPPlayer>(GetPawn()) : Character;
	if (Character && Character->Controller)
	{
		Controller == nullptr ? Controller = Cast<AMPPlayerController>(Character->Controller) : Controller;

		if (Controller)
		{
			Controller->SetHUDDefeats(DefeatsAmt);
		}
	}
}

void AMPPlayerState::AddToScore(float Amt)
{
	SetScore(GetScore() + Amt);
	Character = Character == nullptr ? Cast<AMPPlayer>(GetPawn()) : Character;
	
	if (Character && Character->Controller)
	{
		Controller == nullptr ? Controller = Cast<AMPPlayerController>(Character->Controller) : Controller;

		if (Controller)
		{
			Controller->SetHUDScore(GetScore());
		}
	}
}

void AMPPlayerState::AddToDefeat(int Defeats)
{
	DefeatsAmt += Defeats;

	Character = Character == nullptr ? Cast<AMPPlayer>(GetPawn()) : Character;

	if (Character && Character->Controller)
	{
		Controller == nullptr ? Controller = Cast<AMPPlayerController>(Character->Controller) : Controller;

		if (Controller)
		{
			Controller->SetHUDDefeats(DefeatsAmt);
		}
	}
}
