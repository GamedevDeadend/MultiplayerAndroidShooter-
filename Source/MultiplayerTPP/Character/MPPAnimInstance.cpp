// Fill out your copyright notice in the Description page of Project Settings.


#include "MPPAnimInstance.h"
#include "MPPlayer.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/KismetMathLibrary.h"

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

	AdjustJumpVelocity();

	FVector Velocity = OurPlayer->GetVelocity();
	Velocity.Z = 0.0f;

	Speed = Velocity.Size(); //Returns magnitude of vector

	bIsInAir = OurPlayer->GetCharacterMovement()->IsFalling();
	bIsAccelerating = OurPlayer->GetCharacterMovement()->GetCurrentAcceleration().Size() != 0 ? true : false;
	bWeaponEquipped = OurPlayer->IsWeaponEquipped();
	bIsCrouch = OurPlayer->bIsCrouched;
	bIsAiming = OurPlayer->IsAiming();

	FRotator AimRotation = OurPlayer->GetBaseAimRotation();
	FRotator MovementRotation = UKismetMathLibrary::MakeRotFromX(OurPlayer->GetVelocity());
	FRotator DeltaRot = UKismetMathLibrary::NormalizedDeltaRotator(MovementRotation, AimRotation);
	DeltaRotation = FMath::RInterpTo(DeltaRotation, DeltaRot, Deltatime, 6.0f);
	YawOffset = DeltaRotation.Yaw;

	PlayerRotationLastFrame = PlayerRotation;
	PlayerRotation = OurPlayer->GetActorRotation();
	const FRotator Delta = UKismetMathLibrary::NormalizedDeltaRotator(PlayerRotation, PlayerRotationLastFrame);
	const float Target = Delta.Yaw/Deltatime;
	const float Interp = FMath::FInterpTo(Lean, Target, Deltatime, 6.0f);
	Lean = FMath::Clamp(Interp, -90.0f, 90.0f);

}

void UMPPAnimInstance::AdjustJumpVelocity()
{
	if (bWeaponEquipped)
	{
		OurPlayer->GetCharacterMovement()->JumpZVelocity = 600.0f;
	}
	else
	{
		OurPlayer->GetCharacterMovement()->JumpZVelocity = 420.0f;
	}
}

