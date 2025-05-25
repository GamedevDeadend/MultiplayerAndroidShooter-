// Fill out your copyright notice in the Description page of Project Settings.


#include "ProjectileWeapon.h"
#include "Engine/SkeletalMeshSocket.h"
#include "Projectile.h"
#include "Engine/Engine.h"

void AProjectileWeapon:: Fire(const FVector& HitTarget)
{
	Super::Fire(HitTarget);

	//UE_LOG(LogTemp, Warning, TEXT("FireSuccess"));

	const USkeletalMeshSocket* MuzzleFlashSocket = GetWeaponMesh()->GetSocketByName("MuzzleFlash");
	APawn* ProjectileInstigator = Cast<APawn>(GetOwner());
	UWorld* World = GetWorld();

	if (World != nullptr &&MuzzleFlashSocket != nullptr )
	{
		FTransform MFSocketTransform = MuzzleFlashSocket->GetSocketTransform(GetWeaponMesh());
		FVector ToTarget = HitTarget - MFSocketTransform.GetLocation();
		FRotator ProjectileOrientation = ToTarget.Rotation();

		FActorSpawnParameters ProjectileSpawnParams;
		ProjectileSpawnParams.Owner = GetOwner();
		ProjectileSpawnParams.Instigator = ProjectileInstigator;

		AProjectile* SpawnedProjectile = nullptr;

		if (bIsUsingSSR == true)
		{

			if (ProjectileInstigator->HasAuthority())
			{
				if (ProjectileInstigator->IsLocallyControlled())
				{
					//GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Green, TEXT("On Server and Locally Controlled"));
					SpawnedProjectile = World->SpawnActor<AProjectile>(ProjectileClass, MFSocketTransform.GetLocation(), ProjectileOrientation, ProjectileSpawnParams);
					SpawnedProjectile->bUseSSR = false;
					SpawnedProjectile->Damage = Damage;
					SpawnedProjectile->HeadDamage = HeadDamage;
				}
				else // server, not locally controlled - spawn non-replicated projectile, SSR
				{
					//GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Green, TEXT("On Server and  NOT Locally Controlled"));
					SpawnedProjectile = World->SpawnActor<AProjectile>(SSR_ProjectileClass, MFSocketTransform.GetLocation(), ProjectileOrientation, ProjectileSpawnParams);
					SpawnedProjectile->bUseSSR = true;
				}

			}
			else
			{
				if (ProjectileInstigator->IsLocallyControlled()) // client, locally controlled - spawn non-replicated projectile, use SSR
				{
					//GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Green, TEXT("On Client and Locally Controlled"));
					SpawnedProjectile = World->SpawnActor<AProjectile>(SSR_ProjectileClass, MFSocketTransform.GetLocation(), ProjectileOrientation, ProjectileSpawnParams);
					SpawnedProjectile->bUseSSR = true;
					SpawnedProjectile->TraceStart = MFSocketTransform.GetLocation();
					SpawnedProjectile->InitialVelocity = SpawnedProjectile->GetActorForwardVector() * SpawnedProjectile->InitialSpeed;
				}
				else // client, not locally controlled - spawn non-replicated projectile, no SSR
				{
					//GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Green, TEXT("On Client and  NOT Locally Controlled"));
					SpawnedProjectile = World->SpawnActor<AProjectile>(SSR_ProjectileClass, MFSocketTransform.GetLocation(), ProjectileOrientation, ProjectileSpawnParams);
					SpawnedProjectile->bUseSSR = false;
				}
			}
		}
		else
		{
			if (ProjectileInstigator->HasAuthority())
			{
				//GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Green, TEXT("On Server and SSR OFF"));
				SpawnedProjectile = World->SpawnActor<AProjectile>(ProjectileClass, MFSocketTransform.GetLocation(), ProjectileOrientation, ProjectileSpawnParams);
				SpawnedProjectile->bUseSSR = false;
				SpawnedProjectile->Damage = Damage;
				SpawnedProjectile->HeadDamage = HeadDamage;
			}
		}
	}

}