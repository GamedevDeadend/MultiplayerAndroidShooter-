// Fill out your copyright notice in the Description page of Project Settings.


#include "MPPAnimInstance.h"
#include "MPPlayer.h"
#include "GameFramework/CharacterMovementComponent.h"

void UMPPAnimInstance::NativeInitializeAnimation()
{
	Super:: NativeInitializeAnimation();

	OurPlayer = Cast<AMPPlayer>(TryGetPawnOwner());

}

void UMPPAnimInstance::NativeUpdateAnimation(float Deltatime)
{
	Super::NativeUpdateAnimation(Deltatime);

	if (OurPlayer == nullptr)
	{
		OurPlayer = Cast<AMPPlayer>(TryGetPawnOwner());
	}

	if (OurPlayer == nullptr)
		return;

	FVector Velocity = OurPlayer->GetVelocity();
	Velocity.Z = 0.0f;

	Speed = Velocity.Size(); //Returns magnitude of vector

	bIsInAir = OurPlayer->GetCharacterMovement()->IsFalling();
	bIsAccelerating = OurPlayer->GetCharacterMovement()->GetCurrentAcceleration().Size() != 0 ? true : false;
	bWeaponEquipped = OurPlayer->IsWeaponEquipped();
	bIsCrouch = OurPlayer->bIsCrouched;
	bIsAiming = OurPlayer->IsAiming();
}

