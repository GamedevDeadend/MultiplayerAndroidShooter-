// Fill out your copyright notice in the Description page of Project Settings.


#include "DeathMatch_GS.h"
#include "MultiplayerTPP/PlayerState/MPPlayerState.h"
#include "Net/UnrealNetwork.h"

void ADeathMatch_GS::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ADeathMatch_GS, TopScoringPlayers);
	DOREPLIFETIME(ADeathMatch_GS, PlayersInfo);
}

void ADeathMatch_GS::AddNewPlayer(AMPPlayerState* PlayerState)
{
	if (PlayerState != nullptr)
	{
		FPlayerScoreInfo CurrPlayer{};

		CurrPlayer.CurrScore = PlayerState->GetScore();
		CurrPlayer.PlayerName = PlayerState->GetPlayerName();
		CurrPlayer.PlayerTeam = PlayerState->GetPlayerTeam();

		//UE_LOG(LogTemp, Warning, TEXT("New Player %s"), *PlayerState->GetPlayerName());

		PlayersInfo.Add(CurrPlayer);
	}
}

void ADeathMatch_GS::UpdateTopScore(AMPPlayerState* ScoringPlayerState)
{
	auto CurrScore = ScoringPlayerState->GetScore();
	//UE_LOG(LogTemp, Warning, TEXT("Curr Score %f"), CurrScore);


	if (TopScoringPlayers.Num() == 0)
	{
		TopScoringPlayers.Add(ScoringPlayerState);
		return;
	}
	else if (CurrScore == TopScore)
	{
		TopScoringPlayers.AddUnique(ScoringPlayerState);
	}
	else if (CurrScore > TopScore)
	{
		TopScoringPlayers.Empty();
		TopScoringPlayers.AddUnique(ScoringPlayerState);
		TopScore = CurrScore;
	}
}

void ADeathMatch_GS::UpdatePlayersInfo(AMPPlayerState* ScoringPlayerState)
{
	auto CurrScore = ScoringPlayerState->GetScore();
	//UE_LOG(LogTemp, Warning, TEXT("Score updated for %s: %f"), *ScoringPlayerState->GetPlayerName(), CurrScore);

	for (auto& Player : PlayersInfo)
	{
		//UE_LOG(LogTemp, Warning, TEXT("Name Check Passed Score updated for %s:  PlayerState %s"), *Player.PlayerName,*ScoringPlayerState->GetPlayerName());
		if (Player.PlayerName == ScoringPlayerState->GetPlayerName())
		{
			Player.CurrScore = CurrScore;
			//UE_LOG(LogTemp, Warning, TEXT("Name Check Passed Score updated for %s: %f"), *ScoringPlayerState->GetPlayerName(), CurrScore);
		}
	}

	SortPlayerInfo();
}

void ADeathMatch_GS::SortPlayerInfo()
{
	if (PlayersInfo.IsEmpty() == false)
	{
		PlayersInfo.Sort
		(
			[](const FPlayerScoreInfo& A, const FPlayerScoreInfo& B)
			{
				if (A.PlayerTeam < B.PlayerTeam)
				{
					return A.Priority > B.Priority;
				}
				else if (A.PlayerTeam == B.PlayerTeam)
				{
					return A.CurrScore > B.CurrScore;
				}
				else
				{
					return A.Priority < B.Priority;
				}
			}
		);
	}
}
