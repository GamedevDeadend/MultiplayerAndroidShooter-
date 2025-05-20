// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "MultiplayerTPP/Weapons/Projectile.h"
#include "ProjectileBullet.generated.h"

/**
 * 
 */
UCLASS()
class MULTIPLAYERTPP_API AProjectileBullet : public AProjectile
{
	GENERATED_BODY()

	virtual void BeginPlay()override;

protected:

	virtual void OnHit
	(
		UPrimitiveComponent* HitComponent,
		AActor* OtherActor, UPrimitiveComponent* OtherComp,
		FVector NormalImpulse, const FHitResult& Hit
	);
	
};
