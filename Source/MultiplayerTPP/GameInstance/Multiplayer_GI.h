// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "MultiplayerTPP/Types/GameModeTypes.h"
#include "Multiplayer_GI.generated.h"

/**
 * 
 */
UCLASS()
class MULTIPLAYERTPP_API UMultiplayer_GI : public UGameInstance
{
	GENERATED_BODY()

public : 

	//UPROPERTY(Replicated)
	EGameModeType CurrentGameModeType = EGameModeType::EGM_TDM;

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	virtual void Init()override;
	
};
