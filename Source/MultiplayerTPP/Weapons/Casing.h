// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Casing.generated.h"

UCLASS()
class MULTIPLAYERTPP_API ACasing : public AActor
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	ACasing();

private:

	UPROPERTY(VisibleAnywhere)
		UStaticMeshComponent* CasingMesh;

	UPROPERTY(VisibleAnywhere)
		float ImpulseScale;

	UPROPERTY(EditAnywhere)
		class USoundCue* ShellHitSound;

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

};
