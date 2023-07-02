// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "MPPlayerController.generated.h"

/**
 * 
 */
UCLASS()
class MULTIPLAYERTPP_API AMPPlayerController : public APlayerController
{
	GENERATED_BODY()

protected:

	virtual void BeginPlay() override;

private:

	class AMPPlayerHUD* PlayerHUD;

public:

	void SetHUDHealth(float MaxHealth, float CurrentHealth);

};
