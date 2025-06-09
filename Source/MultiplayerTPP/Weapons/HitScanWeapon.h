// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Weapons.h"
#include "HitScanWeapon.generated.h"

/**
 * 
 */
UCLASS()
class MULTIPLAYERTPP_API AHitScanWeapon : public AWeapons
{
	GENERATED_BODY()

public :

	virtual void Fire(const FVector& HitTarget) override;

private :

	UPROPERTY()
	AMPPlayerController* MPPlayerController = nullptr;

	void WeaponTraceHit(const FVector& TraceStart, const FVector& HitTarget, FHitResult& OutHit);


protected : 

	UPROPERTY(EditAnywhere)
	class UParticleSystem* ImpactParticles;

	UPROPERTY(EditAnywhere)
	USoundCue* HitSound;
private:



	UPROPERTY(EditAnywhere)
	UParticleSystem* BeamParticles;

	UPROPERTY(EditAnywhere)
	UParticleSystem* MuzzleFlash;

	UPROPERTY(EditAnywhere)
	USoundCue* FireSound;
	
};
