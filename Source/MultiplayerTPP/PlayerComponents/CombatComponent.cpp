// Fill out your copyright notice in the Description page of Project Settings.


#include "CombatComponent.h"
#include "MultiplayerTPP/Weapons/Weapons.h"
#include "MultiplayerTPP/Character/MPPlayer.h"
#include "Engine/SkeletalMeshSocket.h"
#include "Net/UnrealNetwork.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "DrawDebugHelpers.h"
#include "MultiplayerTPP/Controllers/MPPlayerController.h"
#include "Camera/CameraComponent.h"
#include "MultiplayerTPP/Interfaces/InteractWithCrosshairs.h"
#include "Components/SpotLightComponent.h"
#include "Sound/SoundCue.h"
#include "MultiplayerTPP/Types/WeaponType.h"




UCombatComponent::UCombatComponent()
{
	PrimaryComponentTick.bCanEverTick = true;

	BaseWalkSpeed = 600.0f;
	BaseJumpVelocity = 420.0f;
	EquipJumpVelociy = 750.0f;
	AimWalkSpeed = 300.0f;
}

void UCombatComponent :: GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(UCombatComponent, EquippedWeapon);
	DOREPLIFETIME(UCombatComponent, bAim);
	DOREPLIFETIME_CONDITION(UCombatComponent, CarriedAmmo, COND_OwnerOnly);
	DOREPLIFETIME(UCombatComponent, CombatState);
}

void UCombatComponent::BeginPlay()
{
	Super::BeginPlay();

	if (MPPlayer)
	{
		MPPlayer->GetCharacterMovement()->JumpZVelocity = BaseJumpVelocity;
		MPPlayer->GetCharacterMovement()->MaxWalkSpeed = BaseWalkSpeed;
	}

	if (MPPlayer->GetFollowCamera())
	{
		DefaultFOV = MPPlayer->GetFollowCamera()->FieldOfView;
		CurrentFOV = DefaultFOV;
	}

	if (MPPlayer->HasAuthority())
	{
		InitPlayeCarriedAmmoMap();
	}
}

void UCombatComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (MPPlayer && MPPlayer->IsLocallyControlled())
	{
		FHitResult HitResult;
		TraceUnderCrosshairs(HitResult);
		HitTarget = HitResult.ImpactPoint;

		SetHUD(DeltaTime);
		InterpFOV(DeltaTime);

	}
}

void UCombatComponent::InitPlayeCarriedAmmoMap()
{
	CarriedAmmoMap.Emplace(EWeaponType::EWT_AR_Auto, DeafaultAvailableAmmo);
	CarriedAmmoMap.Emplace(EWeaponType::EWT_AR_Burst, DeafaultAvailableAmmo);
	CarriedAmmoMap.Emplace(EWeaponType::EWT_AR_Single, DeafaultAvailableAmmo);
}

/// <summary>
/// Rep Notifier for Combat State
/// </summary>
void UCombatComponent::OnRep_CombatState()
{
	switch (CombatState)
	{
		case ECombatState::ECS_Reloading :
			if (MPPlayer && MPPlayer->IsLocallyControlled() == false)
			{
				HandleReload();
			}
		break;
	}
}
void UCombatComponent::OnRep_Aiming()
{
	if (MPPlayer != nullptr && MPPlayer->IsLocallyControlled())
	{
		 bAim = bIsAimPressed;
	}
}
/// <summary>
/// Function Called when Reload button is pressed
/// It will calll server RPC to handle reload on server 
/// </summary>
void UCombatComponent::Reload()
{
	if (CarriedAmmo > 0 && CombatState != ECombatState::ECS_Reloading)
	{
		ServerReload();
		HandleReload();
	}
}
/// <summary>
/// Servr RPC for Reload
/// </summary>
void UCombatComponent::ServerReload_Implementation()
{
	CombatState = ECombatState::ECS_Reloading;

	if (MPPlayer != nullptr && MPPlayer->IsLocallyControlled() == false)
	{
		HandleReload();
	}
}

void UCombatComponent::FinishReloading()
{
	if (MPPlayer != nullptr)
	{
		if (MPPlayer->HasAuthority())
		{
			CombatState = ECombatState::ECS_Unoccupied;

			if (EquippedWeapon != nullptr)
			{
				EquippedWeapon->Reload(CarriedAmmo);
				UpdateCarriedAmmo();
			}
		}
	}
}

void UCombatComponent::HandleReload()
{
	if (MPPlayer == nullptr) return;
	MPPlayer->PlayReloadMontage();
}

void UCombatComponent::SetHUD(float DeltaTime)
{
	if (MPPlayer == nullptr || MPPlayer->Controller == nullptr) return;

	MPPlayerController = MPPlayerController == nullptr ? Cast<AMPPlayerController>(MPPlayer->Controller) : MPPlayerController;
	if (MPPlayerController)
	{
		HUD = HUD == nullptr ? Cast<AMPPlayerHUD>(MPPlayerController->GetHUD()) : HUD;
		if (HUD)
		{
			
			if (EquippedWeapon)
			{
				MPPlayerHUDPackage.CrosshairsCenter = EquippedWeapon->CrosshairsCenter;
				MPPlayerHUDPackage.CrosshairsLeft = EquippedWeapon->CrosshairsLeft;
				MPPlayerHUDPackage.CrosshairsRight = EquippedWeapon->CrosshairsRight;
				MPPlayerHUDPackage.CrosshairsBottom = EquippedWeapon->CrosshairsBottom;
				MPPlayerHUDPackage.CrosshairsTop = EquippedWeapon->CrosshairsTop;
				MPPlayerHUDPackage.Scale = EquippedWeapon->CrosshairsScale;
			}
			else
			{
				MPPlayerHUDPackage.CrosshairsCenter = nullptr;
				MPPlayerHUDPackage.CrosshairsLeft = nullptr;
				MPPlayerHUDPackage.CrosshairsRight = nullptr;
				MPPlayerHUDPackage.CrosshairsBottom = nullptr;
				MPPlayerHUDPackage.CrosshairsTop = nullptr;
				MPPlayerHUDPackage.Scale = 0.0f;
			}

			if (bAim)
			{
				AimFactor = FMath::FInterpTo(AimFactor, -0.5f, DeltaTime, 10.0f);
			}
			else
			{
				AimFactor = FMath::FInterpTo(AimFactor, 0.0f, DeltaTime, 30.0f);
			}

			FVector2D VelocityFactorRange(0.0f, 0.5f);
			FVector2D WalkSpeedRange = FVector2D(0.0f, MPPlayer->GetCharacterMovement()->MaxWalkSpeed);
			FVector2D CrouchWalkSpeedRange = FVector2D(0.0f, MPPlayer->GetCharacterMovement()->MaxWalkSpeedCrouched);
			FVector Velocity = MPPlayer->GetVelocity();
			Velocity.Z = 0.0f;

			VelocityFactor = MPPlayer->bIsCrouched ? FMath::GetMappedRangeValueClamped(CrouchWalkSpeedRange, VelocityFactorRange, Velocity.Size()) : FMath::GetMappedRangeValueClamped(WalkSpeedRange, VelocityFactorRange, Velocity.Size());
			JumpFactor = MPPlayer->GetCharacterMovement()->IsFalling() ? FMath::FInterpTo(JumpFactor, 1.0f, DeltaTime, 10.0f) : FMath::FInterpTo(JumpFactor, 0.0f, DeltaTime, 50.0f);
			ShootingFactor = FMath::FInterpTo(ShootingFactor, 0.0f, DeltaTime, 50.0f);

			MPPlayerHUDPackage.CrosshairSpread = 0.5f + VelocityFactor + JumpFactor + AimFactor + ShootingFactor;
			HUD->SetHUD(MPPlayerHUDPackage);
		}
	}

}


void UCombatComponent::TraceUnderCrosshairs(FHitResult& TraceHitResult)
{
	FVector2D ViewPortSize;
	if (GEngine && GEngine->GameViewport)
	{
		GEngine->GameViewport->GetViewportSize(ViewPortSize);
	}

	FVector2D CrosshairLocation = FVector2D(ViewPortSize.X / 2, ViewPortSize.Y / 2);
	FVector CrosshairWorldPosition;
	FVector CrosshairWorldDirection;

	//Calculation of Crossair
	bool bLineTrace = UGameplayStatics::DeprojectScreenToWorld
	(
		UGameplayStatics::GetPlayerController(this, 0),
		CrosshairLocation, CrosshairWorldPosition,
		CrosshairWorldDirection
	);

	if (bLineTrace)
	{
		FVector Start = CrosshairWorldPosition;
		FVector End = Start + CrosshairWorldDirection * TRACE_LENGTH;

		if (MPPlayer)
		{
			float CameraToPlayerDistance = (MPPlayer->GetActorLocation() - Start).Size();
			Start += CrosshairWorldDirection * (CameraToPlayerDistance + 30.0f);
			//DrawDebugSphere(GetWorld(), Start, 20.0f, 14, FColor::Blue, false);
		}


		GetWorld()->LineTraceSingleByChannel(TraceHitResult, Start, End, ECollisionChannel::ECC_Visibility);

		if (TraceHitResult.GetActor() && TraceHitResult.GetActor()->Implements<UInteractWithCrosshairs>())
		{
			MPPlayerHUDPackage.CrosshairsColor = EnemyAimColor;
		}

		else
		{
			MPPlayerHUDPackage.CrosshairsColor = NormalAimColor;
		}

		if (!TraceHitResult.bBlockingHit)
		{
			TraceHitResult.ImpactPoint = End;
			//UE_LOG(LogTemp, Warning, TEXT("TraceSucces2"));
			HitTarget = End;
		}

		else
		{
			//UE_LOG(LogTemp, Warning, TEXT("TraceSucces"));
			HitTarget = TraceHitResult.ImpactPoint;
		}
	}
}

void UCombatComponent::FirePressed(bool bPressed)
{
	bFireButtonPressed = bPressed;

	if (EquippedWeapon == nullptr) return;

	if (bPressed)
	{
		switch (EquippedWeapon->GetWeaponType())
		{
			case EWeaponType::EWT_AR_Single:
				Fire();
				break;

			case EWeaponType::EWT_AR_Auto:
				MPPlayer->GetWorldTimerManager().SetTimer(AutoFireTimerHandle, this, &UCombatComponent::Fire, 0.1f, true);
				break;

			case EWeaponType::EWT_AR_Burst:
				MPPlayer->GetWorldTimerManager().SetTimer(BurstFireTimerHandle, this, &UCombatComponent::BurstFire, 0.1f, true);
				break;
		}
	}
	else
	{
		MPPlayer->GetWorldTimerManager().ClearTimer(AutoFireTimerHandle);
	}

	if (EquippedWeapon)
	{
		ShootingFactor = 0.5f;
	}
}

void UCombatComponent::Fire()
{
	if (EquippedWeapon != nullptr)
	{
		bIsWeaponEmpty = EquippedWeapon->GetIsEmpty();

		if (bIsWeaponEmpty)
		{
			Reload();
		}
	}

	if ( !bIsWeaponEmpty && CombatState == ECombatState::ECS_Unoccupied)
	{
		ServerFire(HitTarget);

		if (MPPlayer->HasAuthority() == false && MPPlayer->IsLocallyControlled() == true)
		{
			LocalFire(HitTarget);
		}
	}
}

void UCombatComponent::ServerFire_Implementation(const FVector_NetQuantize& TraceHitTarget)
{
	MultiCastFire(TraceHitTarget);
}

void UCombatComponent::MultiCastFire_Implementation(const FVector_NetQuantize& TraceHitTarget)
{
	if (MPPlayer != nullptr && MPPlayer->IsLocallyControlled() && !MPPlayer->HasAuthority()){ return; }
	LocalFire(TraceHitTarget);
}

void UCombatComponent::LocalFire(const FVector_NetQuantize& TraceHitTarget)
{
	if (!EquippedWeapon) return;
	if (MPPlayer)
	{
		MPPlayer->PlayFireMontage(bAim);
		EquippedWeapon->Fire(TraceHitTarget);
	}
}

void UCombatComponent::BurstFire()
{
	BurstFireCount++;
	if (BurstFireCount == 3)
	{
		MPPlayer->GetWorldTimerManager().ClearTimer(BurstFireTimerHandle);
		BurstFireCount = 0;
	}

	Fire();
}

void UCombatComponent::OnRep_EquippedWeapon()
{
	if (EquippedWeapon && MPPlayer)
	{
		EquippedWeapon->SetWeaponState(EWeaponState::EWS_Equipped);

		if (EquippedWeapon->EquipSound != nullptr)
		{
			UGameplayStatics::PlaySoundAtLocation(this, EquippedWeapon->EquipSound, MPPlayer->GetActorLocation());
		}

		MPPlayer->SetBaseAimRotation(FRotator(0.0f, MPPlayer->GetBaseAimRotation().Yaw, 0.0f));

		const USkeletalMeshSocket* HandSocket = MPPlayer->GetMesh()->GetSocketByName(FName("RightHandSocket"));

		if (HandSocket)
		{
			HandSocket->AttachActor(EquippedWeapon, MPPlayer->GetMesh());
		}
		MPPlayer->GetCharacterMovement()->bOrientRotationToMovement = false;
		MPPlayer->bUseControllerRotationYaw = true;
		MPPlayer->GetCharacterMovement()->JumpZVelocity = MPPlayer->IsWeaponEquipped() ? EquipJumpVelociy : BaseJumpVelocity;

		if (MPPlayerController != nullptr && MPPlayerController->IsLocalController())
		{
			MPPlayerController->SetHUDWeaponInfo(EquippedWeapon->GetWeaponDataAsset());
		}
	}
}

void UCombatComponent::EquipWeapon(AWeapons* WeaponToEquip)
{
	if (MPPlayer == nullptr || WeaponToEquip == nullptr)
	{
		return;
	}

	if (EquippedWeapon != nullptr)
	{
		EquippedWeapon->Dropped();
	}

	EquippedWeapon = WeaponToEquip;
	EquippedWeapon->SetOwner(MPPlayer);
	EquippedWeapon->SetWeaponState(EWeaponState::EWS_Equipped);
	MPPlayer->SetBaseAimRotation(FRotator(0.0f, MPPlayer->GetBaseAimRotation().Yaw, 0.0f));
	const USkeletalMeshSocket* HandSocket = MPPlayer->GetMesh()->GetSocketByName(FName("RightHandSocket"));

	if (HandSocket)
	{
		HandSocket->AttachActor(EquippedWeapon, MPPlayer->GetMesh());
	}

	EquippedWeapon->SetHUDAmmo();

	if (CarriedAmmoMap.Contains(EquippedWeapon->GetWeaponType()) )
	{
		CarriedAmmo =  CarriedAmmoMap[EquippedWeapon->GetWeaponType()];
	}
	UpdateCarriedAmmo();

	if (MPPlayerController != nullptr &&MPPlayerController->IsLocalController())
	{
			MPPlayerController->SetHUDWeaponInfo(EquippedWeapon->GetWeaponDataAsset());
	}

	if (EquippedWeapon->EquipSound != nullptr)
	{
		UGameplayStatics::PlaySoundAtLocation(this, EquippedWeapon->EquipSound, MPPlayer->GetActorLocation());
	}

	if (EquippedWeapon->GetIsEmpty())
	{
		Reload();
	}

	MPPlayer->GetCharacterMovement()->bOrientRotationToMovement = false;
	MPPlayer->bUseControllerRotationYaw = true;
	MPPlayer->GetCharacterMovement()->JumpZVelocity = MPPlayer->IsWeaponEquipped() ? EquipJumpVelociy : BaseJumpVelocity;
}

void UCombatComponent::UpdateCarriedAmmo()
{
	if (EquippedWeapon == nullptr) { return; }

	if (CarriedAmmoMap.Contains(EquippedWeapon->GetWeaponType()))
	{
		CarriedAmmoMap[EquippedWeapon->GetWeaponType()] = CarriedAmmo;
	}

	if (MPPlayerController != nullptr)
	{
		MPPlayerController->SetHUDCarriedAmmo(CarriedAmmo);
	}
}

void UCombatComponent::InterpFOV(float DeltaTime)
{
	if (EquippedWeapon == nullptr) return;

	ZoomFOV = EquippedWeapon->GetZoomedFOV();

	if (bAim)
	{
		CurrentFOV = FMath::FInterpTo(CurrentFOV, ZoomFOV, DeltaTime, EquippedWeapon->GetZoomInterpSpeed());
	}
	else
	{
		CurrentFOV = FMath::FInterpTo(CurrentFOV, DefaultFOV, DeltaTime, ZoomOutInterp);
	}
	if (MPPlayer && MPPlayer->GetFollowCamera())
	{
		MPPlayer->GetFollowCamera()->SetFieldOfView(CurrentFOV);
	}
}


void UCombatComponent::OnRep_CarriedAmmo()
{
	UpdateCarriedAmmo();
} 

void UCombatComponent::SetAiming(bool bIsAiming)
{
	bAim = bIsAiming;
	//It is not neccessary to call server rpc on client only as calling it on server will execute on server only. 
	// For More Info https://docs.unrealengine.com/5.1/en-US/rpcs-in-unreal-engine/
	ServerSetAiming(bIsAiming);

	if (MPPlayer != nullptr && MPPlayer->IsLocallyControlled())
	{
		bIsAimPressed = bAim;
	}

}


void UCombatComponent::ServerSetAiming_Implementation(bool bIsAiming)
{
	bAim = bIsAiming;

	MPPlayer->GetCharacterMovement()->MaxWalkSpeed = bIsAiming && MPPlayer->IsWeaponEquipped() ? AimWalkSpeed : BaseWalkSpeed;
}

