// Fill out your copyright notice in the Description page of Project Settings.


#include "MPPlayerState.h"
#include "MultiplayerTPP/Character/MPPlayer.h"
#include "MultiplayerTPP/Controllers/MPPlayerController.h"

void AMPPlayerState::OnRep_Score()
{
	Super::OnRep_Score();

	Character = Character == nullptr ? Cast<AMPPlayer>(GetPawn()) : Character;
	Controller = Controller == nullptr ? Cast<AMPPlayerController>(Character->Controller) : Controller;
	if (Character)
	{
		if (Controller)
		{
			Controller->SetHUDScore(Score);
		}
	}
}

void AMPPlayerState::AddToScore(float Amt)
{
	Score += Amt;
	Character = Character == nullptr ? Cast<AMPPlayer>(GetPawn()) : Character;
	Controller = Controller == nullptr ? Cast<AMPPlayerController>(Character->Controller) : Controller;
	if (Character)
	{
		if (Controller)
		{
			Controller->SetHUDScore(Score);
		}
	}
}
