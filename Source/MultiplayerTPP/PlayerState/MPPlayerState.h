// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerState.h"
#include "MPPlayerState.generated.h"

/**
 * 
 */
UCLASS()
class MULTIPLAYERTPP_API AMPPlayerState : public APlayerState
{
	GENERATED_BODY()

public:

	virtual void OnRep_Score() override;
	virtual void AddToScore(float Amt);

private:

	TObjectPtr<class AMPPlayer> Character;
	TObjectPtr<class AMPPlayerController> Controller;
};
