// Fill out your copyright notice in the Description page of Project Settings.


#include "ProjectileBullet.h"
#include "Kismet/GameplayStatics.h"
#include "GameFrameWork/Character.h"

void AProjectileBullet::OnHit
(
	UPrimitiveComponent* HitComponent,
	AActor* OtherActor, UPrimitiveComponent* OtherComp,
	FVector NormalImpulse, const FHitResult& Hit
)
{
	ACharacter* OwnerCharacter = Cast<ACharacter>(GetOwner());
	if (OwnerCharacter)
	{
		AController* DamageInstigatorController = OwnerCharacter->GetController();
		if (DamageInstigatorController)
		{
			UGameplayStatics::ApplyDamage(OtherActor, Damage, DamageInstigatorController, this, UDamageType::StaticClass());
		}
	}

	Super::OnHit(HitComponent, OtherActor, OtherComp, NormalImpulse, Hit);
}

