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
	float LevelStartTime = 0.0f;

	UPROPERTY(EditDefaultsOnly)
		float WarmupTime = 10.0f;

	UPROPERTY(EditDefaultsOnly)
		float MatchTime = 20.0f;

	UPROPERTY(EditDefaultsOnly)
		float CooldownTime = 10.0f;


		float CountDownTime = 0.0f;


private : 


public:
	ADeathMatch_GM();
	virtual void BeginPlay()override;
	virtual void Tick(float DeltaTime)override;
	virtual void PlayerEliminated(class AMPPlayer* EliminatedCharacter, class AMPPlayerController* EliminatedPlayerController, class AMPPlayerController* AttackingPlayerController);
	virtual void RequestRespawn(ACharacter* ElimCharacter, AController* ElimPlayerController);
	virtual void OnMatchStateSet()override;

};
