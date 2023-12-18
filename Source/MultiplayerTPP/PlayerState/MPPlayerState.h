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
	virtual void AddToDefeat(int DefeatAmt);

private:

	virtual void GetLifetimeReplicatedProps(TArray< FLifetimeProperty >& OutLifetimeProps) const override;

	UPROPERTY()
	TObjectPtr<class AMPPlayer> Character;

	UPROPERTY()
	TObjectPtr<class AMPPlayerController> Controller;

	UPROPERTY(ReplicatedUsing = OnRep_Defeats)
	int32 DefeatsAmt;
};
 