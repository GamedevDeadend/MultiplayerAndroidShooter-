// Fill out your copyright notice in the Description page of Project Settings.


#include "HitScanWeapon.h"
#include "Engine/SkeletalMeshSocket.h"
#include "MultiplayerTPP/Character/MPPlayer.h"
#include "MultiplayerTPP/Controllers/MPPlayerController.h"
#include "Kismet/GameplayStatics.h"
#include "Sound/SoundCue.h"
#include "particles/ParticleSystemComponent.h"
#include "MultiplayerTPP/PlayerComponents/LagCompensationComponent.h"

void AHitScanWeapon::Fire(const FVector& HitTarget)
{
	Super::Fire(HitTarget);

	APawn* OwnerPawn = Cast<APawn>(GetOwner());
	if (OwnerPawn == nullptr) return;
	AController* InstigatorController = OwnerPawn->GetController();

	const USkeletalMeshSocket* MuzzleFlashSocket = GetWeaponMesh()->GetSocketByName("MuzzleFlash");
	if (MuzzleFlashSocket)
	{
		FTransform SocketTransform = MuzzleFlashSocket->GetSocketTransform(GetWeaponMesh());
		FVector Start = SocketTransform.GetLocation();

		FHitResult FireHit;
		WeaponTraceHit(Start, HitTarget, FireHit);

		AMPPlayer* MPPlayer = Cast<AMPPlayer>(FireHit.GetActor());
		if (MPPlayer && InstigatorController)
		{
			if (HasAuthority())
			{
				float DamageToBeApllied = Damage;

				if (FireHit.BoneName.ToString() == FString("head"))
				{
					DamageToBeApllied = HeadDamage;
				}

				UGameplayStatics::ApplyDamage(
					MPPlayer,
					DamageToBeApllied,
					InstigatorController,
					this,
					UDamageType::StaticClass()
				);
			}
			if (!HasAuthority() && bIsUsingSSR)
			{
				MPPlayer = MPPlayer == nullptr ? Cast<AMPPlayer>(OwnerPawn) : MPPlayer;
				MPPlayerController = MPPlayerController == nullptr ? Cast<AMPPlayerController>(InstigatorController) : MPPlayerController;
				if (MPPlayerController && MPPlayer && MPPlayer->GetLagCompensationComponent() && MPPlayer->IsLocallyControlled())
				{
					MPPlayer->GetLagCompensationComponent()->ServerScoreRequest(
						MPPlayer,
						Start,
						HitTarget,
						MPPlayerController->GetServerTime() - MPPlayerController->SingleTripTime,
						this
					);
				}
			}
		}
		if (ImpactParticles)
		{
			UGameplayStatics::SpawnEmitterAtLocation(
				GetWorld(),
				ImpactParticles,
				FireHit.ImpactPoint,
				FireHit.ImpactNormal.Rotation()
			);
		}
		if (HitSound)
		{
			UGameplayStatics::PlaySoundAtLocation(
				this,
				HitSound,
				FireHit.ImpactPoint
			);
		}

		if (MuzzleFlash)
		{
			UGameplayStatics::SpawnEmitterAtLocation(
				GetWorld(),
				MuzzleFlash,
				SocketTransform
			);
		}
		if (FireSound)
		{
			UGameplayStatics::PlaySoundAtLocation(
				this,
				FireSound,
				GetActorLocation()
			);
		}
	}
}

void AHitScanWeapon::WeaponTraceHit(const FVector& TraceStart, const FVector& HitTarget, FHitResult& OutHit)
{
	UWorld* World = GetWorld();
	if (World)
	{
		FVector End = TraceStart + (HitTarget - TraceStart) * 1.25f;

		World->LineTraceSingleByChannel(
			OutHit,
			TraceStart,
			End,
			ECollisionChannel::ECC_Visibility
		);
		FVector BeamEnd = End;
		if (OutHit.bBlockingHit)
		{
			BeamEnd = OutHit.ImpactPoint;
		}

		DrawDebugSphere(GetWorld(), BeamEnd, 16.f, 12, FColor::Orange, true);

		if (BeamParticles)
		{
			UParticleSystemComponent* Beam = UGameplayStatics::SpawnEmitterAtLocation(
				World,
				BeamParticles,
				TraceStart,
				FRotator::ZeroRotator,
				true
			);
			if (Beam)
			{
				Beam->SetVectorParameter(FName("Target"), BeamEnd);
			}
		}
	}
}