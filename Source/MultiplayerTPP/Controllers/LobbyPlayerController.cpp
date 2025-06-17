// Fill out your copyright notice in the Description page of Project Settings.


#include "LobbyPlayerController.h"
#include "MultiplayerSessionsSubsystem.h"
#include "MultiplayerSessionsSubsystem.h"
#include "MultiplayerTPP/GameInstance/Multiplayer_GI.h"

void ALobbyPlayerController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);

	/*if (InPawn != nullptr)
	{
	
		UMultiplayer_GI* Multiplayer_GI = GetGameInstance<UMultiplayer_GI>();

		if (Multiplayer_GI != nullptr)
		{
			UMultiplayerSessionsSubsystem* MultiplayerSubsystem = Multiplayer_GI->GetSubsystem<UMultiplayerSessionsSubsystem>();
			if (MultiplayerSubsystem != nullptr)
			{
				GEngine->AddOnScreenDebugMessage(-1, 4.0f, FColor::Green, FString::Printf(TEXT("%s Match Type Set"), *MultiplayerSubsystem->MatchType));

				if (MultiplayerSubsystem->MatchType == "FreeForAll")
				{
					Multiplayer_GI->CurrentGameModeType = EGameModeType::EGM_SDM;
					GEngine->AddOnScreenDebugMessage(-1, 4.0f, FColor::Green, FString::Printf(TEXT("Match Type Set to %s"), *MultiplayerSubsystem->MatchType));
				}
				else if (MultiplayerSubsystem->MatchType == "TeamDeathMatch")
				{
					Multiplayer_GI->CurrentGameModeType = EGameModeType::EGM_TDM;
					GEngine->AddOnScreenDebugMessage(-1, 4.0f, FColor::Green, FString::Printf(TEXT("Match Type Set to %s"), *MultiplayerSubsystem->MatchType));
				}
			}

		}
	}*/
}

void ALobbyPlayerController::BeginPlay()
{
	auto Multiplayer_GI = GetGameInstance<UMultiplayer_GI>();
	auto MultiplayerSubsystem = Multiplayer_GI->GetSubsystem<UMultiplayerSessionsSubsystem>();

	if(Multiplayer_GI != nullptr)
	{
		if(MultiplayerSubsystem != nullptr && MultiplayerSubsystem->MatchType == "FreeForAll")
		{
			Multiplayer_GI->CurrentGameModeType = EGameModeType::EGM_SDM;
			GEngine->AddOnScreenDebugMessage(-1, 4.0f, FColor::Green, FString::Printf(TEXT("Match Type Set to %s"), *MultiplayerSubsystem->MatchType));
		}
		else if(MultiplayerSubsystem != nullptr && MultiplayerSubsystem->MatchType == "TeamDeathMatch")
		{
			Multiplayer_GI->CurrentGameModeType = EGameModeType::EGM_TDM;
			GEngine->AddOnScreenDebugMessage(-1, 4.0f, FColor::Green, FString::Printf(TEXT("Match Type Set to %s"), *MultiplayerSubsystem->MatchType));
		}
	}
}
