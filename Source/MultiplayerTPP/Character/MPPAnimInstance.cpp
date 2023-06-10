// Fill out your copyright notice in the Description page of Project Settings.


#include "MPPAnimInstance.h"
#include "MPPlayer.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/KismetMathLibrary.h"
#include "MultiplayerTPP/Weapons/Weapons.h"

void UMPPAnimInstance::NativeInitializeAnimation()
{
	Super:: NativeInitializeAnimation();

	MPPlayer = Cast<AMPPlayer>(TryGetPawnOwner());

}

void UMPPAnimInstance::NativeUpdateAnimation(float Deltatime)
{


	Super::NativeUpdateAnimation(Deltatime);

	if (MPPlayer == nullptr)
	{
		MPPlayer = Cast<AMPPlayer>(TryGetPawnOwner());
	}

	if (MPPlayer == nullptr)
		return;

	FVector Velocity = MPPlayer->GetVelocity();
	Velocity.Z = 0.0f;
	Speed = Velocity.Size(); //Returns magnitude of vector

	EquippedWeapon = MPPlayer->GetEquippedWeapon();
	bIsInAir = MPPlayer->GetCharacterMovement()->IsFalling();
	bIsAccelerating = MPPlayer->GetCharacterMovement()->GetCurrentAcceleration().Size() != 0 ? true : false;
	bWeaponEquipped = MPPlayer->IsWeaponEquipped();
	bIsCrouch = MPPlayer->bIsCrouched;
	bIsPlayerAiming = MPPlayer->IsAiming();
	TurningInplace = MPPlayer->GetTurningState();

	FRotator AimRotation = MPPlayer->GetBaseAimRotation();
	FRotator MovementRotation = UKismetMathLibrary::MakeRotFromX(MPPlayer->GetVelocity());
	FRotator DeltaRot = UKismetMathLibrary::NormalizedDeltaRotator(MovementRotation, AimRotation);
	DeltaRotation = FMath::RInterpTo(DeltaRotation, DeltaRot, Deltatime, 6.0f);
	YawOffset = DeltaRotation.Yaw;

	//Lean Calculation
	PlayerRotationLastFrame = PlayerRotation;
	PlayerRotation = MPPlayer->GetActorRotation();
	const FRotator Delta = UKismetMathLibrary::NormalizedDeltaRotator(PlayerRotation, PlayerRotationLastFrame);
	const float Target = Delta.Yaw/Deltatime;
	const float Interp = FMath::FInterpTo(Lean, Target, Deltatime, 6.0f);
	Lean = FMath::Clamp(Interp, -90.0f, 90.0f);

	AO_Yaw = MPPlayer->GetAimYaw();
	AO_Pitch = MPPlayer->GetAimPitch();

	if (bWeaponEquipped && EquippedWeapon && EquippedWeapon->GetWeaponMesh() && MPPlayer->GetMesh())
	{

		//Setting LeftHandTransform for FABRIK
		LeftHandTransform = EquippedWeapon->GetWeaponMesh()->GetSocketTransform(FName("LeftHandSocket"), ERelativeTransformSpace::RTS_World);
		FVector OutPosition;
		FRotator OutRotation;
		MPPlayer->GetMesh()->TransformToBoneSpace(FName("Hand_R"), LeftHandTransform.GetLocation(), FRotator::ZeroRotator, OutPosition, OutRotation);
		LeftHandTransform.SetLocation(OutPosition);
		LeftHandTransform.SetRotation(FQuat(OutRotation));

		//Calculating difference between gun pointing and projectile travel(Also RHand rotation to fix that)

		if (MPPlayer->IsLocallyControlled())
		{
			bIsLocallyControlled = true;
			FTransform RightHandTransform = MPPlayer->GetMesh()->GetSocketTransform(FName("Hand_R"), ERelativeTransformSpace::RTS_World);
			FRotator LookAtRotation = UKismetMathLibrary::FindLookAtRotation(RightHandTransform.GetLocation(), RightHandTransform.GetLocation() + (RightHandTransform.GetLocation() - MPPlayer->GetHitTarget()));
			RightHandRotation = FMath::RInterpTo(RightHandRotation, LookAtRotation, Deltatime, 20.0f);
		}
	}
}

//***********DEBUG**********************************
		//FTransform MuzzleFlashTransform = EquippedWeapon->GetWeaponMesh()->GetSocketTransform(FName("MuzzleFlash"), ERelativeTransformSpace::RTS_World);
		//FVector MuzzleX(FRotationMatrix(MuzzleFlashTransform.GetRotation().Rotator()).GetUnitAxis(EAxis::X));
		//DrawDebugLine(GetWorld(), MuzzleFlashTransform.GetLocation(), MuzzleFlashTransform.GetLocation() + MuzzleX * 10000.0f, FColor::Green);
		//DrawDebugLine(GetWorld(), MuzzleFlashTransform.GetLocation(), MPPlayer->GetHitTarget(), FColor::Orange);
