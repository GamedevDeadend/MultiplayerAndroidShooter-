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

	UFUNCTION()
	virtual void OnRep_Defeats();

	virtual void AddToScore(float Amt);
	virtual void AddToDefeat(int32 DefeatAmt);
	virtual void DisplayLosingMessage();

private:

	virtual void GetLifetimeReplicatedProps(TArray< FLifetimeProperty >& OutLifetimeProps) const override;
	virtual void HideHUDMessage();

	UPROPERTY()
	class AMPPlayer* Character = nullptr;

	UPROPERTY()
	class AMPPlayerController* Controller = nullptr;

	UPROPERTY(ReplicatedUsing = OnRep_Defeats)
	int32 DefeatsAmt;

	UPROPERTY(EditAnywhere, Category = "Defeated Message")
	FString DisplayMessage;

};
 