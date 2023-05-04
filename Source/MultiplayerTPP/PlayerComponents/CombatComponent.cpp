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
}

void UCombatComponent::BeginPlay()
{
	Super::BeginPlay();

	if (Player)
	{
		Player->GetCharacterMovement()->JumpZVelocity = BaseJumpVelocity;
		Player->GetCharacterMovement()->MaxWalkSpeed = BaseWalkSpeed;
	}

	if (Player->GetFollowCamera())
	{
		DefaultFOV = Player->GetFollowCamera()->FieldOfView;
		CurrentFOV = DefaultFOV;
	}
}


void UCombatComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	FHitResult HitResult;
	if (Player && Player->IsLocallyControlled())
	{
		TraceUnderCrosshairs(HitResult);
		SetHUD(DeltaTime);
		InterpFOV(DeltaTime);

	}
}

void UCombatComponent::SetHUD(float DeltaTime)
{
	if (Player == nullptr || Player->Controller == nullptr) return;

	PlayerController = PlayerController == nullptr ? Cast<AMPPlayerController>(Player->Controller) : PlayerController;
	if (PlayerController)
	{
		HUD = HUD == nullptr ? Cast<AMPPlayerHUD>(PlayerController->GetHUD()) : HUD;
		if (HUD)
		{
			
			if (EquippedWeapon)
			{
				HUDPackage.CrosshairsCenter = EquippedWeapon->CrosshairsCenter;
				HUDPackage.CrosshairsLeft = EquippedWeapon->CrosshairsLeft;
				HUDPackage.CrosshairsRight = EquippedWeapon->CrosshairsRight;
				HUDPackage.CrosshairsBottom = EquippedWeapon->CrosshairsBottom;
				HUDPackage.CrosshairsTop = EquippedWeapon->CrosshairsTop;
				HUDPackage.Scale = EquippedWeapon->CrosshairsScale;
			}
			else
			{
				HUDPackage.CrosshairsCenter = nullptr;
				HUDPackage.CrosshairsLeft = nullptr;
				HUDPackage.CrosshairsRight = nullptr;
				HUDPackage.CrosshairsBottom = nullptr;
				HUDPackage.CrosshairsTop = nullptr;
				HUDPackage.Scale = 0.0f;
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
			FVector2D WalkSpeedRange = FVector2D(0.0f, Player->GetCharacterMovement()->MaxWalkSpeed);
			FVector2D CrouchWalkSpeedRange = FVector2D(0.0f, Player->GetCharacterMovement()->MaxWalkSpeedCrouched);
			FVector Velocity = Player->GetVelocity();
			Velocity.Z = 0.0f;

			VelocityFactor = Player->bIsCrouched ? FMath::GetMappedRangeValueClamped(CrouchWalkSpeedRange, VelocityFactorRange, Velocity.Size()) : FMath::GetMappedRangeValueClamped(WalkSpeedRange, VelocityFactorRange, Velocity.Size());
			JumpFactor = Player->GetCharacterMovement()->IsFalling() ? FMath::FInterpTo(JumpFactor, 1.0f, DeltaTime, 10.0f) : FMath::FInterpTo(JumpFactor, 0.0f, DeltaTime, 50.0f);
			ShootingFactor = FMath::FInterpTo(ShootingFactor, 0.0f, DeltaTime, 50.0f);

			HUDPackage.CrosshairSpread = 0.5f + VelocityFactor + JumpFactor + AimFactor + ShootingFactor;
			HUD->SetHUD(HUDPackage);
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
		FVector End = Start + CrosshairWorldDirection * 80000.f;

		GetWorld()->LineTraceSingleByChannel(TraceHitResult, Start, End, ECollisionChannel::ECC_Visibility);

		if (TraceHitResult.GetActor() && TraceHitResult.GetActor()->Implements<UInteractWithCrosshairs>())
		{
			HUDPackage.CrosshairsColor = FLinearColor::Red;
		}

		else
		{
			HUDPackage.CrosshairsColor = FLinearColor::Yellow;
		}

		if (!TraceHitResult.bBlockingHit)
		{
			TraceHitResult.ImpactPoint = End;
			UE_LOG(LogTemp, Warning, TEXT("TraceSucces2"));
			HitTarget = End;
		}

		else
		{
			UE_LOG(LogTemp, Warning, TEXT("TraceSucces"));
			HitTarget = TraceHitResult.ImpactPoint;
		}
	}
}

void UCombatComponent::FirePressed(bool bPressed)
{
	bFireButtonPressed = bPressed;

	if (bFireButtonPressed)
	{
		FHitResult HitResult;
		TraceUnderCrosshairs(HitResult);
		ServerFire(HitResult.ImpactPoint);
	}

	if (EquippedWeapon)
	{
		ShootingFactor = 0.5f;
	}
}


void UCombatComponent::ServerFire_Implementation(const FVector_NetQuantize& TraceHitTarget)
{
	MultiCastFire(TraceHitTarget);
}

void UCombatComponent::MultiCastFire_Implementation(const FVector_NetQuantize& TraceHitTarget)
{
	if (!EquippedWeapon) return;
	if (Player)
	{
		Player->PlayFireMontage(bAim);
		EquippedWeapon->Fire(TraceHitTarget);
		UE_LOG(LogTemp, Warning, TEXT("FireSucces"));
	}
}


void UCombatComponent::OnRep_WeaponEquip()
{
	if (EquippedWeapon && Player)
	{
		Player->GetCharacterMovement()->bOrientRotationToMovement = false;
		Player->bUseControllerRotationYaw = true;
	}
}

void UCombatComponent::EquipWeapon(AWeapons* WeaponToEquip)
{
	if (!Player || !WeaponToEquip)
		return;

	EquippedWeapon = WeaponToEquip;
	EquippedWeapon->SetWeaponState(EWeaponState::EWS_Equipped);
	Player->SetBaseAimRotation(FRotator(0.0f, Player->GetBaseAimRotation().Yaw, 0.0f));
	const USkeletalMeshSocket* HandSocket = Player->GetMesh()->GetSocketByName(FName("RightHandSocket"));

	if (HandSocket)
	{
		HandSocket->AttachActor(EquippedWeapon, Player->GetMesh());
	}

	EquippedWeapon->SetOwner(Player);

	Player->GetCharacterMovement()->bOrientRotationToMovement = false;
	Player->bUseControllerRotationYaw = true;
	Player->GetCharacterMovement()->JumpZVelocity = Player->IsWeaponEquipped() ? EquipJumpVelociy : BaseJumpVelocity;
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
	if (Player && Player->GetFollowCamera())
	{
		Player->GetFollowCamera()->SetFieldOfView(CurrentFOV);
	}
}


void UCombatComponent::SetAiming(bool bIsAiming)
{
	bAim = bIsAiming;
	//It is not neccessary to call server rpc on client only as calling it on server will execute on server only. 
	// For More Info https://docs.unrealengine.com/5.1/en-US/rpcs-in-unreal-engine/
	ServerSetAiming(bIsAiming); 

}


void UCombatComponent::ServerSetAiming_Implementation(bool bIsAiming)
{
	bAim = bIsAiming;

	Player->GetCharacterMovement()->MaxWalkSpeed = bIsAiming && Player->IsWeaponEquipped() ? AimWalkSpeed : BaseWalkSpeed;
}

