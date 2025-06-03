// Fill out your copyright notice in the Description page of Project Settings.


#include "LobbyPlayerController.h"
#include "MultiplayerSessionsSubsystem.h"
#include "MultiplayerTPP/GameInstance/Multiplayer_GI.h"

void ALobbyPlayerController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);

	//if (InPawn != nullptr)
	//{
	//
	//	UMultiplayer_GI* Multiplayer_GI = GetGameInstance<UMultiplayer_GI>();

	//	if (Multiplayer_GI != nullptr)
	//	{
	//		UMultiplayerSessionsSubsystem* MultiplayerSubsystem = Multiplayer_GI->GetSubsystem<UMultiplayerSessionsSubsystem>();
	//		if (MultiplayerSubsystem != nullptr)
	//		{

	//			if (MultiplayerSubsystem->MatchType == "FreeForAll")
	//			{
	//				Multiplayer_GI->CurrentGameModeType = EGameModeType::EGM_SDM;
	//			}
	//			else if (MultiplayerSubsystem->MatchType == "TeamDeathMatch")
	//			{
	//				Multiplayer_GI->CurrentGameModeType = EGameModeType::EGM_TDM;
	//			}
	//		}

	//	}
	//}
}
