// Fill out your copyright notice in the Description page of Project Settings.


#include "DeathMatch_GS.h"
#include "MultiplayerTPP/PlayerState/MPPlayerState.h"
#include "Net/UnrealNetwork.h"

void ADeathMatch_GS::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ADeathMatch_GS, TopScoringPlayers);
}

void ADeathMatch_GS::UpdateTopScore(AMPPlayerState* ScoringPlayerState)
{
	auto CurrScore = ScoringPlayerState->GetScore();

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
