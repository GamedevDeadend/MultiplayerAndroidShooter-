// Fill out your copyright notice in the Description page of Project Settings.


#include "MPPlayerState.h"
#include "MultiplayerTPP/Character/MPPlayer.h"
#include "MultiplayerTPP/Controllers/MPPlayerController.h"
#include "Net/UnrealNetwork.h"
#include "TimerManager.h"


void AMPPlayerState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME_CONDITION(AMPPlayerState, DefeatsAmt, COND_OwnerOnly);
		//(AMPPlayerState, DefeatsAmt);
}

void AMPPlayerState::AddToScore(float Amt)
{
	SetScore(GetScore() + Amt);
	Character = Character == nullptr ? Cast<AMPPlayer>(GetPawn()) : Character;

	if (Character && Character->Controller)
	{
		Controller = Controller == nullptr ? Cast<AMPPlayerController>(Character->Controller) : Controller;

		if (Controller)
		{
			Controller->SetHUDScore(GetScore());
		}
	}
}

void AMPPlayerState::OnRep_Score()
{
	Super::OnRep_Score();

	Character = Character == nullptr ? Cast<AMPPlayer>(GetPawn()) : Character;
	if (Character && Character->Controller)
	{
		Controller =  Controller == nullptr ? Cast<AMPPlayerController>(Character->Controller) : Controller;
		
		if (Controller)
		{
			Controller->SetHUDScore(GetScore());
		}
	}
}

void AMPPlayerState::AddToDefeat(int32 Defeats)
{
	DefeatsAmt += Defeats;
	Character = Character == nullptr ? Cast<AMPPlayer>(GetPawn()) : Character;

	if (Character && Character->Controller)
	{
		Controller = Controller == nullptr ? Cast<AMPPlayerController>(Character->Controller) : Controller;

		if (Controller)
		{
			Controller->SetHUDDefeats(DefeatsAmt);
		}
	}
}

void AMPPlayerState::OnRep_Defeats()
{
	Character = Character == nullptr ? Cast<AMPPlayer>(GetPawn()) : Character;

	if (Character)
	{
		Controller = Controller == nullptr ?  Cast<AMPPlayerController>(Character->Controller) : Controller;

		if (Controller)
		{
			Controller->SetHUDDefeats(DefeatsAmt);
		}
	}
}

/*  ##########TODO DISPLAY DEFEAT MESSAGE################
void AMPPlayerState::DisplayLosingMessage()
{
	Character = Character == nullptr ? Cast<AMPPlayer>(GetPawn()) : Character;

	if (Character)
	{
		//Controller = Controller == nullptr ?  Cast<TObjectPtr<AMPPlayerController>>(Character->Controller) : Controller;
		//UE_LOG(LogTemp, Warning, TEXT("Inside Display Message"));

		Controller = Cast<AMPPlayerController>(Character->Controller);

		if (Controller)
		{
			Controller->SetHUDDefeatMessage(DisplayMessage);
		}
	}

	FTimerHandle HideDelayTimerHanlde;
	GetWorldTimerManager().SetTimer(HideDelayTimerHanlde,this, &ThisClass::HideHUDMessage, 1.5f, false);
}
void AMPPlayerState::HideHUDMessage()
{
	if (GetPawn())
	{
		Cast<AMPPlayerController>(GetPawn()->GetController())->HideMessage();
	}
}*/

