// Fill out your copyright notice in the Description page of Project Settings.


#include "LobbyGM.h"
#include "MultiplayerSessionsSubsystem.h"
#include "MultiplayerTPP/GameInstance/Multiplayer_GI.h"
#include"GameFramework/GameStateBase.h"



void ALobbyGM::PostLogin(APlayerController* NewPlayer)
{
	Super::PostLogin(NewPlayer);

	UMultiplayer_GI* Multiplayer_GI = GetGameInstance<UMultiplayer_GI>();

	if (Multiplayer_GI != nullptr)
	{
		UMultiplayerSessionsSubsystem* MultiplayerSession = Multiplayer_GI->GetSubsystem<UMultiplayerSessionsSubsystem>();
		MaxPlayerCount = MultiplayerSession->NumOfConnections;

		if (MultiplayerSession->MatchType == "FreeForAll")
		{
			MapName = "SoloDeathMatch";
			Multiplayer_GI->CurrentGameModeType = EGameModeType::EGM_SDM;
		}
		else if (MultiplayerSession->MatchType == "TeamDeathMatch")
		{
			MapName = "TeamDeathMatch";
			Multiplayer_GI->CurrentGameModeType = EGameModeType::EGM_TDM;
		}


		int32 PlayerCount = GameState.Get()->PlayerArray.Num();

		if (PlayerCount >= MaxPlayerCount)
		{

			UWorld* World = GetWorld();
			if (World != nullptr)
			{
				bUseSeamlessTravel = true;
				World->ServerTravel(FString::Printf(TEXT("/Game/Maps/%s?listen"), *MapName));
			}

		}
	}
}

void ALobbyGM::StartLvlTravel()
{
	UWorld* World = GetWorld();
	if (World != nullptr)
	{
		bUseSeamlessTravel = true;
		World->ServerTravel(FString::Printf(TEXT("/Game/Maps/%s?listen"), *MapName));
	}

}