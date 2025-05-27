// Fill out your copyright notice in the Description page of Project Settings.


#include "TeamDeathMatch_GM.h"
#include "MultiplayerTPP/GameStates/TeamDeathMatch_GS.h"
#include "MultiplayerTPP/PlayerState/MPPlayerState.h"

void ATeamDeathMatch_GM::PostLogin(APlayerController* NewPlayer)
{
	Super::PostLogin(NewPlayer);

	if (MatchState == MatchState::InProgress)
	{
		TDM_GS = TDM_GS == nullptr ? GetGameState<ATeamDeathMatch_GS>() : TDM_GS;
		AMPPlayerState* MPPlayerState = NewPlayer->GetPlayerState<AMPPlayerState>();

		if (MPPlayerState != nullptr && MPPlayerState->GetPlayerTeam() == EPlayerTeam::EPT_NONE)
		{
			if (TDM_GS->RedTeamPlayers.Num() == TDM_GS->BlueTeamPlayers.Num())
			{
				TDM_GS->RedTeamPlayers.AddUnique(MPPlayerState);
				MPPlayerState->SetPlayerTeam(EPlayerTeam::EPT_RED);

			}
			else if (TDM_GS->RedTeamPlayers.Num() > TDM_GS->BlueTeamPlayers.Num())
			{
				TDM_GS->BlueTeamPlayers.AddUnique(MPPlayerState);
				MPPlayerState->SetPlayerTeam(EPlayerTeam::EPT_BLUE);

			}
			else
			{
				TDM_GS->RedTeamPlayers.AddUnique(MPPlayerState);
				MPPlayerState->SetPlayerTeam(EPlayerTeam::EPT_RED);
			}
		}
	}
}



void ATeamDeathMatch_GM::HandleMatchHasStarted()
{
	TDM_GS = TDM_GS == nullptr ? GetGameState<ATeamDeathMatch_GS>() : TDM_GS;

	for (auto& PlayerState : TDM_GS->PlayerArray)
	{
		AMPPlayerState* MPPlayerState = Cast<AMPPlayerState>(PlayerState);

		if (MPPlayerState != nullptr && MPPlayerState->GetPlayerTeam() == EPlayerTeam::EPT_NONE)
		{
			if (TDM_GS->RedTeamPlayers.Num() == TDM_GS->BlueTeamPlayers.Num())
			{
				GEngine->AddOnScreenDebugMessage(-1, 1.0f, FColor::Green, TEXT("Setting Team Red"));
				TDM_GS->RedTeamPlayers.AddUnique(MPPlayerState);
				MPPlayerState->SetPlayerTeam(EPlayerTeam::EPT_RED);

			}
			else if (TDM_GS->RedTeamPlayers.Num() > TDM_GS->BlueTeamPlayers.Num())
			{
				TDM_GS->BlueTeamPlayers.AddUnique(MPPlayerState);
				MPPlayerState->SetPlayerTeam(EPlayerTeam::EPT_BLUE);

			}
			else
			{
				TDM_GS->RedTeamPlayers.AddUnique(MPPlayerState);
				MPPlayerState->SetPlayerTeam(EPlayerTeam::EPT_RED);
			}
		}
	}

	Super::HandleMatchHasStarted();

}

void ATeamDeathMatch_GM::Logout(AController* Exiting)
{
	Super::Logout(Exiting);

	TDM_GS = TDM_GS == nullptr ? GetGameState<ATeamDeathMatch_GS>() : TDM_GS;
	AMPPlayerState* MPPlayerState = Exiting->GetPlayerState<AMPPlayerState>();

	if (MPPlayerState != nullptr)
	{
		if (TDM_GS->RedTeamPlayers.Contains(MPPlayerState) == true) 
		{
			TDM_GS->RedTeamPlayers.Remove(MPPlayerState);
		}
		else if (TDM_GS->BlueTeamPlayers.Contains(MPPlayerState))
		{
			TDM_GS->BlueTeamPlayers.Remove(MPPlayerState);
		}
	}
}

bool ATeamDeathMatch_GM::CheckIsFriendlyFire(AMPPlayerState* AttackerState, AMPPlayerState* VictimPlayerState)
{
	if (AttackerState == nullptr || VictimPlayerState == nullptr) return false;

	if (AttackerState->GetPlayerTeam() == VictimPlayerState->GetPlayerTeam())
	{
		return true;
	}

	return false;
}

void ATeamDeathMatch_GM::UpdateTeamScore(AMPPlayerState* AttackerState, AMPPlayerState* VictimPlayerState)
{
	if (AttackerState->GetPlayerTeam() == VictimPlayerState->GetPlayerTeam())
	{
		return;
	}


	TDM_GS = TDM_GS == nullptr ? GetGameState<ATeamDeathMatch_GS>() : TDM_GS;

	if (AttackerState->GetPlayerTeam() == EPlayerTeam::EPT_RED)
	{
		TDM_GS->SetRedTeamScore();
	}
	else
	{
		TDM_GS->SetBlueTeamScore();
	}
}
