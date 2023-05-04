// Fill out your copyright notice in the Description page of Project Settings.


#include "MPPAnimInstance.h"
#include "MPPlayer.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/KismetMathLibrary.h"
#include "MultiplayerTPP/Weapons/Weapons.h"

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

	EquippedWeapon = OurPlayer->GetEquippedWeapon();
	bIsInAir = OurPlayer->GetCharacterMovement()->IsFalling();
	bIsAccelerating = OurPlayer->GetCharacterMovement()->GetCurrentAcceleration().Size() != 0 ? true : false;
	bWeaponEquipped = OurPlayer->IsWeaponEquipped();
	bIsCrouch = OurPlayer->bIsCrouched;
	bIsPlayerAiming = OurPlayer->IsAiming();
	TurningInplace = OurPlayer->GetTurningState();

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

	AO_Yaw = OurPlayer->GetAimYaw();
	AO_Pitch = OurPlayer->GetAimPitch();

	if (bWeaponEquipped && EquippedWeapon && EquippedWeapon->GetWeaponMesh() && OurPlayer->GetMesh())
	{

		//Setting LeftHandTransform for FABRIK
		LeftHandTransform = EquippedWeapon->GetWeaponMesh()->GetSocketTransform(FName("LeftHandSocket"), ERelativeTransformSpace::RTS_World);
		FVector OutPosition;
		FRotator OutRotation;
		OurPlayer->GetMesh()->TransformToBoneSpace(FName("Hand_R"), LeftHandTransform.GetLocation(), FRotator::ZeroRotator, OutPosition, OutRotation);
		LeftHandTransform.SetLocation(OutPosition);
		LeftHandTransform.SetRotation(FQuat(OutRotation));

		//Calculating difference between gun pointing and projectile travel(Also RHand rotation to fix that)

		if (OurPlayer->IsLocallyControlled())
		{
			bIsLocallyControlled = true;
			FTransform RightHandTransform = OurPlayer->GetMesh()->GetSocketTransform(FName("Hand_R"), ERelativeTransformSpace::RTS_World);
			RightHandRotation = UKismetMathLibrary::FindLookAtRotation(RightHandTransform.GetLocation(), RightHandTransform.GetLocation() + ( RightHandTransform.GetLocation() - OurPlayer->GetHitTarget()) );
		}
	}
}

//***********DEBUG**********************************
		//FTransform MuzzleFlashTransform = EquippedWeapon->GetWeaponMesh()->GetSocketTransform(FName("MuzzleFlash"), ERelativeTransformSpace::RTS_World);
		//FVector MuzzleX(FRotationMatrix(MuzzleFlashTransform.GetRotation().Rotator()).GetUnitAxis(EAxis::X));
		//DrawDebugLine(GetWorld(), MuzzleFlashTransform.GetLocation(), MuzzleFlashTransform.GetLocation() + MuzzleX * 10000.0f, FColor::Green);
		//DrawDebugLine(GetWorld(), MuzzleFlashTransform.GetLocation(), OurPlayer->GetHitTarget(), FColor::Orange);
