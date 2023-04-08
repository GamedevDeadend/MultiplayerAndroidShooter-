// Fill out your copyright notice in the Description page of Project Settings.


#include "CombatComponent.h"
#include "MultiplayerTPP/Weapons/Weapons.h"
#include "MultiplayerTPP/Character/MPPlayer.h"
#include "Engine/SkeletalMeshSocket.h"
#include "Net/UnrealNetwork.h"
#include "GameFramework/CharacterMovementComponent.h"



UCombatComponent::UCombatComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
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
	
}


void UCombatComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
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
	const USkeletalMeshSocket* HandSocket = Player->GetMesh()->GetSocketByName(FName("RightHandSocket"));

	if (HandSocket)
	{
		HandSocket->AttachActor(EquippedWeapon, Player->GetMesh());
	}

	EquippedWeapon->SetOwner(Player);

	Player->GetCharacterMovement()->bOrientRotationToMovement = false;
	Player->bUseControllerRotationYaw = true;
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
}

