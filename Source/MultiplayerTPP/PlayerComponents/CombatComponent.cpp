// Fill out your copyright notice in the Description page of Project Settings.


#include "CombatComponent.h"
#include "MultiplayerTPP/Weapons/Weapons.h"
#include "MultiplayerTPP/Character/MPPlayer.h"
#include "Engine/SkeletalMeshSocket.h"
#include "Net/UnrealNetwork.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "DrawDebugHelpers.h"



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
}


void UCombatComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	FHitResult HitResult;
	TraceUnderCrosshairs(HitResult);
}


void UCombatComponent::FirePressed(bool bPressed)
{
	bFireButtonPressed = bPressed;

	if (bFireButtonPressed)
	{
		ServerFire();
	}
}


void UCombatComponent::ServerFire_Implementation()
{
	MultiCastFire();
}

void UCombatComponent::MultiCastFire_Implementation()
{
	if (!EquippedWeapon) return;
	if (Player)
	{
		Player->PlayFireMontage(bAim);
		EquippedWeapon->Fire(HitTarget);
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
		UE_LOG(LogTemp, Warning, TEXT("TraceSucces"));
		FVector Start = CrosshairWorldPosition;
		FVector End = Start + CrosshairWorldDirection * 80000.f;

		GetWorld()->LineTraceSingleByChannel(TraceHitResult, Start, End, ECollisionChannel::ECC_Visibility);

		if (!TraceHitResult.bBlockingHit)
		{
			TraceHitResult.ImpactPoint = End;
			UE_LOG(LogTemp, Warning, TEXT("TraceSucces2"));
			HitTarget = End;
		}

		else
		{
			DrawDebugSphere(GetWorld(), TraceHitResult.ImpactPoint, 52.0f, 12, FColor::Red);
			UE_LOG(LogTemp, Warning, TEXT("TraceSucces"));
			HitTarget = TraceHitResult.ImpactPoint;
		}
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

	Player->GetCharacterMovement()->MaxWalkSpeed = bIsAiming&& Player->IsWeaponEquipped() ? AimWalkSpeed : BaseWalkSpeed;
}

