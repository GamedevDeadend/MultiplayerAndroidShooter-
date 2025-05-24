// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameState.h"
#include "MultiplayerTPP/Types/PlayerTeam.h"
#include "DeathMatch_GS.generated.h"

/*
* Player Score Information
*/
USTRUCT(BlueprintType)
struct FPlayerScoreInfo
{
	GENERATED_BODY();

public:

	int32 Priority = 0;

	UPROPERTY()
	FString PlayerName;

	UPROPERTY()
	float CurrScore;

	UPROPERTY()
	EPlayerTeam PlayerTeam = EPlayerTeam::EPT_MAX;

	bool operator<(const FPlayerScoreInfo& Other) const
	{
		return Priority < Other.Priority; // For max-heap
	}
};




/**
 * 
 */
UCLASS()
class MULTIPLAYERTPP_API ADeathMatch_GS : public AGameState
{
	GENERATED_BODY()


public:
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps)const override;
	void AddNewPlayer(class AMPPlayerState* PlayerState);


	UPROPERTY(Replicated)
		TArray<class AMPPlayerState*> TopScoringPlayers;

		void UpdateTopScore(class AMPPlayerState* ScoringPlayerState);
		void UpdatePlayersInfo(class AMPPlayerState* ScoringPlayerState);

	UPROPERTY(Replicated)
		TArray<FPlayerScoreInfo> PlayersInfo;

private:

	float TopScore = 0.0f;

	void SortPlayerInfo();

	
};
