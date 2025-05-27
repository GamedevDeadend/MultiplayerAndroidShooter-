// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "MultiplayerTPP/GameMode/DeathMatch_GM.h"
#include "TeamDeathMatch_GM.generated.h"

/**
 * 
 */
UCLASS()
class MULTIPLAYERTPP_API ATeamDeathMatch_GM : public ADeathMatch_GM
{
	GENERATED_BODY()

public:

	virtual void PostLogin(APlayerController* NewPlayer)override;
	virtual void Logout(AController* Exiting)override;
	virtual bool CheckIsFriendlyFire(AMPPlayerState* AttackerState, AMPPlayerState* VictimPlayerState)override;
	virtual void UpdateTeamScore(class AMPPlayerState* AttackerState, class AMPPlayerState* VictimPlayerState)override;



private:

	UPROPERTY()
		class ATeamDeathMatch_GS* TDM_GS = nullptr;

protected: 

	virtual void HandleMatchHasStarted()override;

};
