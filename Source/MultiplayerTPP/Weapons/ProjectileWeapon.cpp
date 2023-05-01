// Fill out your copyright notice in the Description page of Project Settings.


#include "ProjectileWeapon.h"
#include "Engine/SkeletalMeshSocket.h"
#include "Projectile.h"

void AProjectileWeapon:: Fire(const FVector& HitTarget)
{
	Super::Fire(HitTarget);

	UE_LOG(LogTemp, Warning, TEXT("FireSuccess"));

	if (!HasAuthority()) return;

	const USkeletalMeshSocket* MuzzleFlashSocket = GetWeaponMesh()->GetSocketByName("MuzzleFlash");
	APawn* ProjectileInstigator = Cast<APawn>(GetOwner());

	if (MuzzleFlashSocket)
	{
		FTransform MFSocketTransform = MuzzleFlashSocket->GetSocketTransform(GetWeaponMesh());
		FVector ToTarget = HitTarget - MFSocketTransform.GetLocation();
		FRotator ProjectileOrientation = ToTarget.Rotation();

		FActorSpawnParameters ProjectileSpawnParams;
		ProjectileSpawnParams.Owner = GetOwner();
		ProjectileSpawnParams.Instigator = ProjectileInstigator;

		if (ProjectileClass && ProjectileInstigator)
		{
			UWorld* World = GetWorld();

			if (World)
			{
				World->SpawnActor<AProjectile>
					(
						ProjectileClass,
						MFSocketTransform.GetLocation(),
						ProjectileOrientation,
						ProjectileSpawnParams
					);
			}
		}

	}

}