// Fill out your copyright notice in the Description page of Project Settings.


#include "LobbyGM.h"
#include"GameFramework/GameStateBase.h"



void ALobbyGM::PostLogin(APlayerController* NewPlayer)
{
	Super::PostLogin(NewPlayer);

	int32 PlayerCount = GameState.Get()->PlayerArray.Num();

	if (PlayerCount >= MaxPlayerCount)
	{
		
		GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Green, FString("Our dunction worked"));
		//RestartGame();

		UWorld* World = GetWorld();
		if (World != nullptr)
		{
			bUseSeamlessTravel = true;
			World->ServerTravel(FString::Printf(TEXT("/Game/Maps/%s?listen"), *DeatMatchLvl));
			GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Cyan, FString("Our dunction worked 2"));
		}

	}
}

void ALobbyGM::StartLvlTravel()
{
	UWorld* World = GetWorld();
	if (World != nullptr)
	{
		bUseSeamlessTravel = true;
		World->ServerTravel(FString::Printf(TEXT("/Game/Maps/%s?listen"), *DeatMatchLvl));
		GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Cyan, FString("Our dunction worked 2"));
	}

}