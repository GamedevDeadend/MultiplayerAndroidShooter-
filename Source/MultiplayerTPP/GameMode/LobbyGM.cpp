// Fill out your copyright notice in the Description page of Project Settings.


#include "LobbyGM.h"
#include"GameFramework/GameStateBase.h"



void ALobbyGM::PostLogin(APlayerController* NewPlayer)
{
	Super::PostLogin(NewPlayer);

	int32 PlayerCount = GameState.Get()->PlayerArray.Num();

	if (PlayerCount > 0)
	{
		UWorld* WorldPt = GetWorld();
		GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Green, FString("Our dunction worked"));
		//if (WorldPt != nullptr)
		//{
			bUseSeamlessTravel = true;
			WorldPt->ServerTravel(FString("/Game/Maps/StartLvl?listen"));

		//}
	}
}