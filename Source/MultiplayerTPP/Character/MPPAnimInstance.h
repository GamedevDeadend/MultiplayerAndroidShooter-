// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "MPPAnimInstance.generated.h"

/**
 * 
 */
UCLASS()
class MULTIPLAYERTPP_API UMPPAnimInstance : public UAnimInstance
{
	GENERATED_BODY()

private:

	UPROPERTY(BlueprintReadOnly, Category = Character, meta = (AllowPrivateAccess = "true"))
		class AMPPlayer* OurPlayer;

	UPROPERTY(BlueprintReadOnly, Category = Movement, meta = (AllowPrivateAccess = "true"))
		float Speed;

	UPROPERTY(BlueprintReadOnly, Category = Movement, meta = (AllowPrivateAccess = "true"))
		bool bIsInAir;

	UPROPERTY(BlueprintReadOnly, Category = Movement, meta = (AllowPrivateAccess = "true"))
		bool bIsAccelerating;
	

public:

	virtual void NativeInitializeAnimation() override;
	virtual void NativeUpdateAnimation(float Deltatime) override;
};