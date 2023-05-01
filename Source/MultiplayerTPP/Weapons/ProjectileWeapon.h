// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Weapons.h"
#include "ProjectileWeapon.generated.h"

/**
 *
 */
UCLASS()
class MULTIPLAYERTPP_API AProjectileWeapon : public AWeapons
{
	GENERATED_BODY()

public:

	virtual void Fire(const FVector& HitTarget)override;

private:

	UPROPERTY(EditAnywhere)
		TSubclassOf<class AProjectile> ProjectileClass;


};
