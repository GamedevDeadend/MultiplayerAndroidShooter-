// Fill out your copyright notice in the Description page of Project Settings.


#include "LobbyGM.h"
#include"GameFramework/GameStateBase.h"

void ALobbyGM::PostLogin(APlayerController* NewPlayer)
{
	Super::PostLogin(NewPlayer);

	int32 PlayerCount = GameState.Get()->PlayerArray.Num();

	if (PlayerCount == 1)
	{
		UWorld* WorldPt = GetWorld();
		if (WorldPt != nullptr)
		{
			bUseSeamlessTravel = true;
			WorldPt->ServerTravel(FString("/Game/Maps/StartLvl?listen"));
		}
	}
}