// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameMode.h"
#include "LobbyGM.generated.h"

/**
 * 
 */
UCLASS()
class MULTIPLAYERTPP_API ALobbyGM : public AGameMode
{
	GENERATED_BODY()
private:
	UPROPERTY()
		FTimerHandle LoadDelayHandler;

protected:

	virtual void PostLogin(APlayerController* NewPlayer) override;
	void StartLvlTravel();
	
};
