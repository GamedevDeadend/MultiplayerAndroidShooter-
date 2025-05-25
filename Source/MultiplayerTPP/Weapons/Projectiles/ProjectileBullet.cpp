// Fill out your copyright notice in the Description page of Project Settings.


#include "ProjectileBullet.h"
#include "Kismet/GameplayStatics.h"
#include "MultiplayerTPP/Character/MPPlayer.h"
#include "MultiplayerTPP/Controllers/MPPlayerController.h"
#include "MultiplayerTPP/PlayerComponents/LagCompensationComponent.h"
#include "GameFrameWork/Character.h"

void AProjectileBullet::BeginPlay()
{
	Super::BeginPlay();

	/*
		FPredictProjectilePathParams PathParams;
		PathParams.ActorsToIgnore.Add(this);
		PathParams.bTraceWithChannel = true;
		PathParams.DrawDebugTime = 5.0f;
		PathParams.DrawDebugType = EDrawDebugTrace::ForDuration;
		PathParams.LaunchVelocity = GetActorForwardVector() * InitialSpeed;
		PathParams.MaxSimTime = 4.0f;
		PathParams.ProjectileRadius = 0.0f;
		PathParams.StartLocation = GetActorLocation();
		PathParams.TraceChannel = ECollisionChannel::ECC_Visibility;

		FPredictProjectilePathResult PathResult;

		UGameplayStatics::PredictProjectilePath(this, PathParams, PathResult);
	*/
}

void AProjectileBullet::OnHit
(
	UPrimitiveComponent* HitComponent,
	AActor* OtherActor, UPrimitiveComponent* OtherComp,
	FVector NormalImpulse, const FHitResult& Hit
)
{
	AMPPlayer* OwnerCharacter = Cast<AMPPlayer>(GetOwner());
	if (OwnerCharacter != nullptr)
	{
		AMPPlayerController* DamageInstigatorController = Cast<AMPPlayerController>(OwnerCharacter->GetController());

		if (DamageInstigatorController != nullptr)
		{
			if (OwnerCharacter->HasAuthority() && !bUseSSR)
			{
				//GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Green, TEXT("On Server and SSR OFF"));
				const float DamageCaused = Hit.BoneName.ToString() == FString("head") ? HeadDamage : Damage;


				UGameplayStatics::ApplyDamage(OtherActor, DamageCaused, DamageInstigatorController, this, UGameplayStatics::StaticClass());
				Super::OnHit(HitComponent, OtherActor, OtherComp, NormalImpulse, Hit);
				return;
			}

			AMPPlayer* HitPlayer = Cast<AMPPlayer>(OtherActor);
			if (bUseSSR && OwnerCharacter->GetLagCompensationComponent() && OwnerCharacter->IsLocallyControlled() && HitPlayer)
			{
				//GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Green, TEXT("On Local Unit and SSR ON"));
				OwnerCharacter->GetLagCompensationComponent()->ServerProjectileWeaponScoreRequest
				(
					HitPlayer,
					TraceStart,
					InitialVelocity,
					DamageInstigatorController->GetServerTime() - DamageInstigatorController->SingleTripTime
				);
			}
		}
	}

	Super::OnHit(HitComponent, OtherActor, OtherComp, NormalImpulse, Hit);
}

