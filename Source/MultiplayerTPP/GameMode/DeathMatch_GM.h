// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameMode.h"
#include "DeathMatch_GM.generated.h"

/**
 * 
 */
namespace MatchState
{
	/// <summary>
	/// Called When Match is finished and waiting for new Match
	/// </summary>
	extern MULTIPLAYERTPP_API const FName Cooldown;
}

UCLASS()
class MULTIPLAYERTPP_API ADeathMatch_GM : public AGameMode
{
	GENERATED_BODY()

public:

	ADeathMatch_GM();

	float LevelStartTime = 0.0f;

	UPROPERTY(EditDefaultsOnly)
		float WarmupTime = 10.0f;

	UPROPERTY(EditDefaultsOnly)
		float MatchTime = 20.0f;

	UPROPERTY(EditDefaultsOnly)
		float CooldownTime = 10.0f;


		float CountDownTime = 0.0f;


	virtual bool CheckIsFriendlyFire(class AMPPlayerState* AttackerState, class AMPPlayerState* VictimPlayerState);
	void KickPlayer(class AMPPlayerState* PlayerState);
	virtual void BeginPlay()override;
	virtual void Tick(float DeltaTime)override;
	virtual void PlayerEliminated(class AMPPlayer* EliminatedCharacter, class AMPPlayerController* EliminatedPlayerController, class AMPPlayerController* AttackingPlayerController);
	void CheckForGainLead(AMPPlayerState*& AttackerPlayerState, AMPPlayerController* AttackingPlayerController);
	void CheckForLossLeads(AMPPlayerState* AttackerPlayerState);
	virtual void RequestRespawn(ACharacter* ElimCharacter, AController* ElimPlayerController);
	virtual void UpdateTeamScore(class AMPPlayerState* AttackerState, class AMPPlayerState* VictimPlayerState);
	virtual void OnMatchStateSet()override;

private : 

	UPROPERTY(EditAnywhere)
		FString LobbyLvl = "Lobby";

	UPROPERTY()
		class ADeathMatch_GS* Curr_GameState = nullptr;


protected:

	virtual void RegisterAllPlayers();
	virtual void HandleMatchHasStarted()override;

public:

};
