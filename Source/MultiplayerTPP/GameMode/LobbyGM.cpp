// Fill out your copyright notice in the Description page of Project Settings.


#include "LobbyGM.h"
#include"GameFramework/GameStateBase.h"



void ALobbyGM::PostLogin(APlayerController* NewPlayer)
{
	Super::PostLogin(NewPlayer);

	int32 PlayerCount = GameState.Get()->PlayerArray.Num();

	if (PlayerCount > 2)
	{
		UWorld* World = GetWorld();
		GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Green, FString("Our dunction worked"));
		if (World != nullptr)
		{
			bUseSeamlessTravel = true;
			World->ServerTravel(FString("/Game/Maps/StartLvl?listen"));
			GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Cyan, FString("Our dunction worked 2"));
		}
	}
}