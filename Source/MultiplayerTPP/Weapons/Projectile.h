// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Projectile.generated.h"

UCLASS()
class MULTIPLAYERTPP_API AProjectile : public AActor
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	AProjectile();

private:

	bool bIsHittingPlayer = false;

	UPROPERTY(EditAnywhere)
		class UBoxComponent* CollisionBox;

	UPROPERTY(VisibleAnywhere)
		class UProjectileMovementComponent* ProjectileMovement;

	UPROPERTY(EditAnywhere)
		class UParticleSystem* Tracer;

	class UParticleSystemComponent* TracerComponent;

	UPROPERTY(EditAnywhere)
		UParticleSystem* ImpactParticles;

	UPROPERTY(EditAnywhere)
		UParticleSystem* OtherPlayerImpactParticles;

	UPROPERTY(EditAnywhere)
		class USoundCue* ImpactSound;



protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	UFUNCTION()
		virtual void OnHit
		(
			UPrimitiveComponent* HitComponent,
			AActor* OtherActor, UPrimitiveComponent* OtherComp,
			FVector NormalImpulse, const FHitResult& Hit
		);

public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;
	virtual void Destroyed() override;

};
