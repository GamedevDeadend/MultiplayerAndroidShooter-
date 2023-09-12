// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameMode.h"
#include "DeathMatch_GM.generated.h"

/**
 * 
 */
UCLASS()
class MULTIPLAYERTPP_API ADeathMatch_GM : public AGameMode
{
	GENERATED_BODY()

public:

	virtual void PlayerEliminated(class AMPPlayer* EliminatedCharacter, class AMPPlayerController* EliminatedPlayerController, class AMPPlayerController* AttackingPlayerController);
	
};
